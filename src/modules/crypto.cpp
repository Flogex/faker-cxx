#include "faker-cxx/crypto.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include "faker-cxx/word.h"

namespace faker::crypto
{
namespace
{
template <size_t N>
inline std::string toHex(const std::array<uint8_t, N>& data)
{
    static std::string_view hexDigits{"0123456789abcdef"};

    std::string result;
    result.reserve(N * 2);

    for (uint8_t byte : data)
    {
        result.push_back(hexDigits[byte >> 4]);
        result.push_back(hexDigits[byte & 0x0f]);
    }

    return result;
}

class SHA256
{
public:
    SHA256();
    void update(const uint8_t* data, size_t length);
    void update(const std::string& data);
    std::array<uint8_t, 32> digest();

    static std::string toString(const std::array<uint8_t, 32>& digest)
    {
        return toHex(digest);
    }

private:
    uint8_t m_data[64]{};
    uint32_t m_blocklen;
    uint64_t m_bitlen;
    uint32_t m_state[8]{}; // A, B, C, D, E, F, G, H

    static constexpr std::array<uint32_t, 64> K = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    static uint32_t rotr(uint32_t x, uint32_t n);
    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g);
    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c);
    static uint32_t sig0(uint32_t x);
    static uint32_t sig1(uint32_t x);
    void transform();
    void pad();
    void revert(std::array<uint8_t, 32>& hash);
};

class Md5Hash
{
public:
    /**
     * \param message message to processing
     * \return get md5-hash from message
     */
    [[nodiscard]] static std::array<uint8_t, 16> compute(const std::string& message);

private:
    Md5Hash() = default;

    static uint32_t rotate_left(uint32_t x, int32_t n);
    static std::array<uint8_t, 4> uint32_to_4_bytes(uint32_t value);
    static uint32_t uint32_from_4_bytes(std::array<uint8_t, 4> bytes);

    constexpr static uint32_t k[64] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

    constexpr static uint32_t r[64] = {7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
                                       5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
                                       4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
                                       6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
};

class SHA1
{
public:
    SHA1()
    {
        reset();
    }

    // Process input string in chunks
    void update(const std::string& data)
    {
        for (size_t i = 0; i < data.size(); ++i)
        {
            processByte(static_cast<unsigned char>(data[i]));
        }
    }

    // Finalize and retrieve the resulting hash
    std::array<unsigned char, 20> digest()
    {
        pad();
        std::array<unsigned char, 20> result;
        for (size_t i = 0; i < 5; ++i)
        {
            result[i * 4] = static_cast<unsigned char>((state[i] >> 24) & 0xFF);
            result[i * 4 + 1] = static_cast<unsigned char>((state[i] >> 16) & 0xFF);
            result[i * 4 + 2] = static_cast<unsigned char>((state[i] >> 8) & 0xFF);
            result[i * 4 + 3] = static_cast<unsigned char>(state[i] & 0xFF);
        }
        return result;
    }

    // Convert the digest into a hexadecimal string representation
    static std::string toString(const std::array<unsigned char, 20>& hash)
    {
        std::ostringstream ss;
        ss << std::hex << std::setfill('0');
        for (auto byte : hash)
        {
            ss << std::setw(2) << static_cast<int>(byte);
        }
        return ss.str();
    }

private:
    void reset()
    {
        state = {0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0};
        length = 0;
        buffer.clear();
    }

    void processByte(unsigned char byte)
    {
        buffer.push_back(byte);
        if (buffer.size() == 64)
        {
            processBlock();
            buffer.clear();
        }
        length += 8;
    }

    void processBlock()
    {
        std::array<uint32_t, 80> w = {0};
        for (size_t i = 0; i < 16; ++i)
        {
            w[i] = (static_cast<uint32_t>(buffer[i * 4]) << 24) | (static_cast<uint32_t>(buffer[i * 4 + 1]) << 16) |
                   (static_cast<uint32_t>(buffer[i * 4 + 2]) << 8) | (static_cast<uint32_t>(buffer[i * 4 + 3]));
        }

        for (size_t i = 16; i < 80; ++i)
        {
            w[i] = rotateLeft(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
        }

        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];
        uint32_t e = state[4];

        for (size_t i = 0; i < 80; ++i)
        {
            uint32_t temp = rotateLeft(a, 5) + e + w[i] +
                            (i < 20 ? ((b & c) | (~b & d)) + 0x5A827999 :
                             i < 40 ? (b ^ c ^ d) + 0x6ED9EBA1 :
                             i < 60 ? ((b & c) | (b & d) | (c & d)) + 0x8F1BBCDC :
                                      (b ^ c ^ d) + 0xCA62C1D6);

            e = d;
            d = c;
            c = rotateLeft(b, 30);
            b = a;
            a = temp;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
    }

    void pad()
    {
        buffer.push_back(0x80);
        while (buffer.size() < 56)
        {
            buffer.push_back(0);
        }

        uint64_t len = length;
        for (int i = 0; i < 8; ++i)
        {
            buffer.push_back(static_cast<unsigned char>((len >> (56 - i * 8)) & 0xFF));
        }

        processBlock();
    }

    uint32_t rotateLeft(uint32_t value, size_t bits)
    {
        return (value << bits) | (value >> (32 - bits));
    }

private:
    std::array<uint32_t, 5> state;
    uint64_t length;
    std::vector<unsigned char> buffer;
};
}

std::string sha256(std::optional<std::string> data)
{
    std::string orgData;
    if (!data.has_value() || data->empty())
    {
        orgData = word::sample();
    }
    else
    {
        orgData = data.value();
    }
    std::string result;
    SHA256 sha;
    sha.update(orgData);
    std::array<uint8_t, 32> digest = sha.digest();
    result = SHA256::toString(digest);

    return result;
}

// SHA-1 hashing function wrapper
std::string sha1(std::optional<std::string> data)
{
    std::string orgData;
    if (!data.has_value() || data->empty())
    {
        orgData = word::sample(); // Fallback to default sample data
    }
    else
    {
        orgData = data.value(); // Use provided data
    }

    SHA1 sha;
    sha.update(orgData);                                 // Update the SHA-1 instance with the data
    std::array<unsigned char, 20> result = sha.digest(); // Get the final SHA-1 hash

    return SHA1::toString(result); // Convert the hash to a hex string
}

std::string md5(std::optional<std::string> data)
{
    std::string orgData;
    if (!data.has_value() || data->empty())
    {
        orgData = word::sample();
    }
    else
    {
        orgData = data.value();
    }

    return toHex(Md5Hash::compute(orgData));
}

namespace
{
uint32_t Md5Hash::uint32_from_4_bytes(std::array<uint8_t, 4> bytes)
{
    uint32_t value = 0;

    value = (value << 8) + bytes[3];
    value = (value << 8) + bytes[2];
    value = (value << 8) + bytes[1];
    value = (value << 8) + bytes[0];

    return value;
}

std::array<uint8_t, 4> Md5Hash::uint32_to_4_bytes(uint32_t value)
{
    std::array<uint8_t, 4> bytes{};

    bytes[0] = static_cast<uint8_t>(value);
    bytes[1] = static_cast<uint8_t>(value >> 8);
    bytes[2] = static_cast<uint8_t>(value >> 16);
    bytes[3] = static_cast<uint8_t>(value >> 24);

    return bytes;
}

uint32_t Md5Hash::rotate_left(const uint32_t x, const int32_t n)
{
    return (x << n) | (x >> (32 - n));
}

std::array<uint8_t, 16> Md5Hash::compute(const std::string& message)
{
    uint32_t a0 = 0x67452301, b0 = 0xefcdab89, c0 = 0x98badcfe, d0 = 0x10325476;
    size_t new_len = message.size() + 1;

    while (new_len % (512 / 8) != 448 / 8)
        new_len++;

    std::string msg_copy = message;
    msg_copy.resize(new_len + 8);

    msg_copy[message.size()] = static_cast<char>(0x80);

    std::array<uint8_t, 4> bytes = Md5Hash::uint32_to_4_bytes(static_cast<uint32_t>(message.size() * 8));
    for (size_t i = new_len; i < new_len + 4; i++)
        msg_copy[i] = static_cast<char>(bytes[i - new_len]);

    bytes = Md5Hash::uint32_to_4_bytes(static_cast<uint32_t>(message.size() >> 29));
    for (size_t i = new_len + 4; i < new_len + 8; i++)
        msg_copy[i] = static_cast<char>(bytes[i - new_len - 4]);

    for (size_t i = 0; i < new_len; i += (512 / 8))
    {
        uint32_t w[16];

        for (size_t j = 0; j < 16; j++)
        {
            const std::array<uint8_t, 4> array{
                static_cast<uint8_t>(msg_copy[i + j * 4]),
                static_cast<uint8_t>(msg_copy[i + j * 4 + 1]),
                static_cast<uint8_t>(msg_copy[i + j * 4 + 2]),
                static_cast<uint8_t>(msg_copy[i + j * 4 + 3]),
            };

            w[j] = Md5Hash::uint32_from_4_bytes(array);
        }

        uint32_t a = a0, b = b0, c = c0, d = d0;

        for (size_t j = 0; j < 64; j++)
        {
            uint32_t f, g;

            if (j < 16)
            {
                f = (b & c) | ((~b) & d);
                g = static_cast<uint32_t>(j);
            }
            else if (j < 32)
            {
                f = (d & b) | (c & (~d));
                g = (5 * j + 1) % 16;
            }
            else if (j < 48)
            {
                f = b ^ c ^ d;
                g = (3 * j + 5) % 16;
            }
            else
            {
                f = c ^ (b | (~d));
                g = (7 * j) % 16;
            }

            const uint32_t temp = d;
            d = c;
            c = b;
            b = b + Md5Hash::rotate_left((a + f + Md5Hash::k[j] + w[g]), static_cast<int32_t>(Md5Hash::r[j]));
            a = temp;
        }

        a0 += a;
        b0 += b;
        c0 += c;
        d0 += d;
    }

    const std::array<uint8_t, 4> a0_arr = Md5Hash::uint32_to_4_bytes(a0);
    const std::array<uint8_t, 4> b0_arr = Md5Hash::uint32_to_4_bytes(b0);
    const std::array<uint8_t, 4> c0_arr = Md5Hash::uint32_to_4_bytes(c0);
    const std::array<uint8_t, 4> d0_arr = Md5Hash::uint32_to_4_bytes(d0);

    const std::array<uint8_t, 16> result = {
        a0_arr[0], a0_arr[1], a0_arr[2], a0_arr[3],

        b0_arr[0], b0_arr[1], b0_arr[2], b0_arr[3],

        c0_arr[0], c0_arr[1], c0_arr[2], c0_arr[3],

        d0_arr[0], d0_arr[1], d0_arr[2], d0_arr[3],
    };

    return result;
}

SHA256::SHA256() : m_blocklen(0), m_bitlen(0)
{
    m_state[0] = 0x6a09e667;
    m_state[1] = 0xbb67ae85;
    m_state[2] = 0x3c6ef372;
    m_state[3] = 0xa54ff53a;
    m_state[4] = 0x510e527f;
    m_state[5] = 0x9b05688c;
    m_state[6] = 0x1f83d9ab;
    m_state[7] = 0x5be0cd19;
}

void SHA256::update(const uint8_t* data, size_t length)
{
    for (size_t i = 0; i < length; i++)
    {
        m_data[m_blocklen++] = data[i];
        if (m_blocklen == 64)
        {
            transform();

            m_bitlen += 512;
            m_blocklen = 0;
        }
    }
}

void SHA256::update(const std::string& data)
{
    update(reinterpret_cast<const uint8_t*>(data.c_str()), data.size());
}

std::array<uint8_t, 32> SHA256::digest()
{
    std::array<uint8_t, 32> hash{};

    pad();
    revert(hash);

    return hash;
}

uint32_t SHA256::rotr(uint32_t x, uint32_t n)
{
    return (x >> n) | (x << (32 - n));
}

uint32_t SHA256::choose(uint32_t e, uint32_t f, uint32_t g)
{
    return (e & f) ^ (~e & g);
}

uint32_t SHA256::majority(uint32_t a, uint32_t b, uint32_t c)
{
    return (a & (b | c)) | (b & c);
}

uint32_t SHA256::sig0(uint32_t x)
{
    return SHA256::rotr(x, 7) ^ SHA256::rotr(x, 18) ^ (x >> 3);
}

uint32_t SHA256::sig1(uint32_t x)
{
    return SHA256::rotr(x, 17) ^ SHA256::rotr(x, 19) ^ (x >> 10);
}

void SHA256::transform()
{
    uint32_t maj, xorA, ch, xorE, sum, newA, newE, m[64];
    uint32_t state[8];

    for (uint8_t i = 0, j = 0; i < 16; i++, j += 4)
    {
        m[i] = (static_cast<uint32_t>(m_data[j]) << 24) | (static_cast<uint32_t>(m_data[j + 1]) << 16) |
               (static_cast<uint32_t>(m_data[j + 2]) << 8) | static_cast<uint32_t>(m_data[j + 3]);
    }

    for (uint8_t k = 16; k < 64; k++)
    {
        m[k] = SHA256::sig1(m[k - 2]) + m[k - 7] + SHA256::sig0(m[k - 15]) + m[k - 16];
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        state[i] = m_state[i];
    }

    for (uint8_t i = 0; i < 64; i++)
    {
        maj = SHA256::majority(state[0], state[1], state[2]);
        xorA = SHA256::rotr(state[0], 2) ^ SHA256::rotr(state[0], 13) ^ SHA256::rotr(state[0], 22);

        ch = choose(state[4], state[5], state[6]);

        xorE = SHA256::rotr(state[4], 6) ^ SHA256::rotr(state[4], 11) ^ SHA256::rotr(state[4], 25);

        sum = m[i] + K[i] + state[7] + ch + xorE;
        newA = xorA + maj + sum;
        newE = state[3] + sum;

        state[7] = state[6];
        state[6] = state[5];
        state[5] = state[4];
        state[4] = newE;
        state[3] = state[2];
        state[2] = state[1];
        state[1] = state[0];
        state[0] = newA;
    }

    for (uint8_t i = 0; i < 8; i++)
    {
        m_state[i] += state[i];
    }
}

void SHA256::pad()
{

    uint64_t i = m_blocklen;
    uint8_t end = m_blocklen < 56 ? 56 : 64;

    m_data[i++] = 0x80;
    while (i < end)
    {
        m_data[i++] = 0x00;
    }

    if (m_blocklen >= 56)
    {
        transform();
        memset(m_data, 0, 56);
    }

    m_bitlen += m_blocklen * 8;
    m_data[63] = static_cast<uint8_t>(m_bitlen);
    m_data[62] = static_cast<uint8_t>(m_bitlen >> 8);
    m_data[61] = static_cast<uint8_t>(m_bitlen >> 16);
    m_data[60] = static_cast<uint8_t>(m_bitlen >> 24);
    m_data[59] = static_cast<uint8_t>(m_bitlen >> 32);
    m_data[58] = static_cast<uint8_t>(m_bitlen >> 40);
    m_data[57] = static_cast<uint8_t>(m_bitlen >> 48);

    transform();
}

void SHA256::revert(std::array<uint8_t, 32>& hash)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 8; j++)
        {
            hash[static_cast<uint8_t>(i + (j * 4))] = (m_state[j] >> (24 - i * 8)) & 0x000000ff;
        }
    }
}

}
}
