/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <pdf/PDFEncryptor.hxx>
#include <pdf/EncryptionHashTransporter.hxx>
#include <vcl/pdfwriter.hxx>
#include <comphelper/crypto/Crypto.hxx>
#include <comphelper/hash.hxx>
#include <comphelper/random.hxx>
#include <comphelper/diagnose_ex.hxx>
#include <array>

using namespace css;

namespace vcl::pdf
{
namespace
{
// the maximum password length
constexpr sal_Int32 MD5_DIGEST_SIZE = 16;

// security 128 bit
constexpr sal_Int32 SECUR_128BIT_KEY = 16;

// maximum length of MD5 digest input, in step 2 of algorithm 3.1
// PDF spec ver. 1.4: see there for details
constexpr sal_Int32 MAXIMUM_RC4_KEY_LENGTH = SECUR_128BIT_KEY + 3 + 2;

constexpr sal_Int32 ENCRYPTED_PWD_SIZE = 32;

/* pad string used for password in Standard security handler */
constexpr const std::array<sal_uInt8, 32> s_nPadString
    = { 0x28, 0xBF, 0x4E, 0x5E, 0x4E, 0x75, 0x8A, 0x41, 0x64, 0x00, 0x4E,
        0x56, 0xFF, 0xFA, 0x01, 0x08, 0x2E, 0x2E, 0x00, 0xB6, 0xD0, 0x68,
        0x3E, 0x80, 0x2F, 0x0C, 0xA9, 0xFE, 0x64, 0x53, 0x69, 0x7A };

/** Implements Algorithm 3.2, step 1 only */
void padPassword(std::u16string_view i_rPassword, sal_uInt8* o_pPaddedPW)
{
    // get ansi-1252 version of the password string CHECKIT ! i12626
    OString aString(OUStringToOString(i_rPassword, RTL_TEXTENCODING_MS_1252));

    //copy the string to the target
    sal_Int32 nToCopy
        = (aString.getLength() < ENCRYPTED_PWD_SIZE) ? aString.getLength() : ENCRYPTED_PWD_SIZE;
    sal_Int32 nCurrentChar;

    for (nCurrentChar = 0; nCurrentChar < nToCopy; nCurrentChar++)
        o_pPaddedPW[nCurrentChar] = static_cast<sal_uInt8>(aString[nCurrentChar]);

    //pad it with standard byte string
    sal_Int32 i, y;
    for (i = nCurrentChar, y = 0; i < ENCRYPTED_PWD_SIZE; i++, y++)
        o_pPaddedPW[i] = s_nPadString[y];
}

/** Algorithm 3.2  Compute the encryption key used
 * Step 1 should already be done before calling, the paThePaddedPassword parameter should contain
 * the padded password and must be 32 byte long, the encryption key is returned into the
 * paEncryptionKey parameter, it will be 16 byte long for 128 bit security; for 40 bit security
 * only the first 5 bytes are used
 *
 * TODO: in pdf ver 1.5 and 1.6 the step 6 is different, should be implemented. See spec.
 */
bool computeEncryptionKey(EncryptionHashTransporter* i_pTransporter,
                          vcl::PDFEncryptionProperties& io_rProperties,
                          sal_Int32 i_nAccessPermissions)
{
    bool bSuccess = true;
    ::std::vector<unsigned char> nMD5Sum;

    // transporter contains an MD5 digest with the padded user password already
    ::comphelper::Hash* const pDigest = i_pTransporter->getUDigest();
    if (pDigest)
    {
        //step 3
        if (!io_rProperties.OValue.empty())
            pDigest->update(io_rProperties.OValue.data(), io_rProperties.OValue.size());
        else
            bSuccess = false;
        //Step 4
        sal_uInt8 nPerm[4];

        nPerm[0] = static_cast<sal_uInt8>(i_nAccessPermissions);
        nPerm[1] = static_cast<sal_uInt8>(i_nAccessPermissions >> 8);
        nPerm[2] = static_cast<sal_uInt8>(i_nAccessPermissions >> 16);
        nPerm[3] = static_cast<sal_uInt8>(i_nAccessPermissions >> 24);

        pDigest->update(nPerm, sizeof(nPerm));

        //step 5, get the document ID, binary form
        pDigest->update(io_rProperties.DocumentIdentifier.data(),
                        io_rProperties.DocumentIdentifier.size());
        //get the digest
        nMD5Sum = pDigest->finalize();

        //step 6, only if 128 bit
        for (sal_Int32 i = 0; i < 50; i++)
        {
            nMD5Sum = ::comphelper::Hash::calculateHash(nMD5Sum.data(), nMD5Sum.size(),
                                                        ::comphelper::HashType::MD5);
        }
    }
    else
        bSuccess = false;

    i_pTransporter->invalidate();

    //Step 7
    if (bSuccess)
    {
        io_rProperties.EncryptionKey.resize(MAXIMUM_RC4_KEY_LENGTH);
        for (sal_Int32 i = 0; i < MD5_DIGEST_SIZE; i++)
            io_rProperties.EncryptionKey[i] = nMD5Sum[i];
    }
    else
        io_rProperties.EncryptionKey.clear();

    return bSuccess;
}

/** Algorithm 3.3
 * Compute the encryption dictionary /O value, save into the class data member the step
 * numbers down here correspond to the ones in PDF v.1.4 specification
 */
bool computeODictionaryValue(const sal_uInt8* i_pPaddedOwnerPassword,
                             const sal_uInt8* i_pPaddedUserPassword,
                             std::vector<sal_uInt8>& io_rOValue, sal_Int32 i_nKeyLength)
{
    bool bSuccess = true;

    io_rOValue.resize(ENCRYPTED_PWD_SIZE);

    rtlCipher aCipher = rtl_cipher_createARCFOUR(rtl_Cipher_ModeStream);
    if (aCipher)
    {
        //step 1 already done, data is in i_pPaddedOwnerPassword
        //step 2

        ::std::vector<unsigned char> nMD5Sum(::comphelper::Hash::calculateHash(
            i_pPaddedOwnerPassword, ENCRYPTED_PWD_SIZE, ::comphelper::HashType::MD5));
        //step 3, only if 128 bit
        if (i_nKeyLength == SECUR_128BIT_KEY)
        {
            sal_Int32 i;
            for (i = 0; i < 50; i++)
            {
                nMD5Sum = ::comphelper::Hash::calculateHash(nMD5Sum.data(), nMD5Sum.size(),
                                                            ::comphelper::HashType::MD5);
            }
        }
        //Step 4, the key is in nMD5Sum
        //step 5 already done, data is in i_pPaddedUserPassword
        //step 6
        if (rtl_cipher_initARCFOUR(aCipher, rtl_Cipher_DirectionEncode, nMD5Sum.data(),
                                   i_nKeyLength, nullptr, 0)
            == rtl_Cipher_E_None)
        {
            // encrypt the user password using the key set above
            rtl_cipher_encodeARCFOUR(
                aCipher, i_pPaddedUserPassword, ENCRYPTED_PWD_SIZE, // the data to be encrypted
                io_rOValue.data(), sal_Int32(io_rOValue.size())); //encrypted data
            //Step 7, only if 128 bit
            if (i_nKeyLength == SECUR_128BIT_KEY)
            {
                sal_uInt32 i;
                size_t y;
                sal_uInt8 nLocalKey[SECUR_128BIT_KEY]; // 16 = 128 bit key

                for (i = 1; i <= 19; i++) // do it 19 times, start with 1
                {
                    for (y = 0; y < sizeof(nLocalKey); y++)
                        nLocalKey[y] = static_cast<sal_uInt8>(nMD5Sum[y] ^ i);

                    if (rtl_cipher_initARCFOUR(aCipher, rtl_Cipher_DirectionEncode, nLocalKey,
                                               SECUR_128BIT_KEY, nullptr,
                                               0) //destination data area, on init can be NULL
                        != rtl_Cipher_E_None)
                    {
                        bSuccess = false;
                        break;
                    }
                    rtl_cipher_encodeARCFOUR(
                        aCipher, io_rOValue.data(),
                        sal_Int32(io_rOValue.size()), // the data to be encrypted
                        io_rOValue.data(), sal_Int32(io_rOValue.size()));
                    // encrypted data, can be the same as the input, encrypt "in place"
                    //step 8, store in class data member
                }
            }
        }
        else
            bSuccess = false;
    }
    else
        bSuccess = false;

    if (aCipher)
        rtl_cipher_destroyARCFOUR(aCipher);

    if (!bSuccess)
        io_rOValue.clear();
    return bSuccess;
}

/** Algorithms 3.4 and 3.5
 *
 * Compute the encryption dictionary /U value, save into the class data member,
 * revision 2 (40 bit) or 3 (128 bit)
 */
bool computeUDictionaryValue(EncryptionHashTransporter* i_pTransporter,
                             vcl::PDFEncryptionProperties& io_rProperties, sal_Int32 i_nKeyLength,
                             sal_Int32 i_nAccessPermissions)
{
    bool bSuccess = true;

    io_rProperties.UValue.resize(ENCRYPTED_PWD_SIZE);

    ::comphelper::Hash aDigest(::comphelper::HashType::MD5);
    rtlCipher aCipher = rtl_cipher_createARCFOUR(rtl_Cipher_ModeStream);
    if (aCipher)
    {
        //step 1, common to both 3.4 and 3.5
        if (computeEncryptionKey(i_pTransporter, io_rProperties, i_nAccessPermissions))
        {
            // prepare encryption key for object
            for (sal_Int32 i = i_nKeyLength, y = 0; y < 5; y++)
                io_rProperties.EncryptionKey[i++] = 0;

            //or 3.5, for 128 bit security
            //step6, initialize the last 16 bytes of the encrypted user password to 0
            for (sal_uInt32 i = MD5_DIGEST_SIZE; i < sal_uInt32(io_rProperties.UValue.size()); i++)
                io_rProperties.UValue[i] = 0;
            //steps 2 and 3
            aDigest.update(s_nPadString.data(), sizeof(s_nPadString));
            aDigest.update(io_rProperties.DocumentIdentifier.data(),
                           io_rProperties.DocumentIdentifier.size());

            ::std::vector<unsigned char> const nMD5Sum(aDigest.finalize());
            //Step 4
            rtl_cipher_initARCFOUR(aCipher, rtl_Cipher_DirectionEncode,
                                   io_rProperties.EncryptionKey.data(), SECUR_128BIT_KEY, nullptr,
                                   0); //destination data area
            rtl_cipher_encodeARCFOUR(
                aCipher, nMD5Sum.data(), nMD5Sum.size(), // the data to be encrypted
                io_rProperties.UValue.data(),
                SECUR_128BIT_KEY); //encrypted data, stored in class data member
            //step 5
            sal_uInt32 i;
            size_t y;
            sal_uInt8 nLocalKey[SECUR_128BIT_KEY];

            for (i = 1; i <= 19; i++) // do it 19 times, start with 1
            {
                for (y = 0; y < sizeof(nLocalKey); y++)
                    nLocalKey[y] = static_cast<sal_uInt8>(io_rProperties.EncryptionKey[y] ^ i);

                rtl_cipher_initARCFOUR(aCipher, rtl_Cipher_DirectionEncode, nLocalKey,
                                       SECUR_128BIT_KEY, // key and key length
                                       nullptr, 0); //destination data area, on init can be NULL
                rtl_cipher_encodeARCFOUR(
                    aCipher, io_rProperties.UValue.data(),
                    SECUR_128BIT_KEY, // the data to be encrypted
                    io_rProperties.UValue.data(),
                    SECUR_128BIT_KEY); // encrypted data, can be the same as the input, encrypt "in place"
            }
        }
        else
            bSuccess = false;
    }
    else
        bSuccess = false;

    if (aCipher)
        rtl_cipher_destroyARCFOUR(aCipher);

    if (!bSuccess)
        io_rProperties.UValue.clear();
    return bSuccess;
}

sal_Int32 computeAccessPermissions(const vcl::PDFEncryptionProperties& i_rProperties,
                                   sal_Int32& o_rKeyLength, sal_Int32& o_rRC4KeyLength)
{
    /*
    2) compute the access permissions, in numerical form

    the default value depends on the revision 2 (40 bit) or 3 (128 bit security):
    - for 40 bit security the unused bit must be set to 1, since they are not used
    - for 128 bit security the same bit must be preset to 0 and set later if needed
    according to the table 3.15, pdf v 1.4 */
    sal_Int32 nAccessPermissions = 0xfffff0c0;

    o_rKeyLength = SECUR_128BIT_KEY;
    o_rRC4KeyLength = 16; // for this value see PDF spec v 1.4, algorithm 3.1 step 4, where n is 16,
        // thus maximum permitted value is 16

    nAccessPermissions |= (i_rProperties.CanPrintTheDocument) ? 1 << 2 : 0;
    nAccessPermissions |= (i_rProperties.CanModifyTheContent) ? 1 << 3 : 0;
    nAccessPermissions |= (i_rProperties.CanCopyOrExtract) ? 1 << 4 : 0;
    nAccessPermissions |= (i_rProperties.CanAddOrModify) ? 1 << 5 : 0;
    nAccessPermissions |= (i_rProperties.CanFillInteractive) ? 1 << 8 : 0;
    nAccessPermissions |= (i_rProperties.CanExtractForAccessibility) ? 1 << 9 : 0;
    nAccessPermissions |= (i_rProperties.CanAssemble) ? 1 << 10 : 0;
    nAccessPermissions |= (i_rProperties.CanPrintFull) ? 1 << 11 : 0;
    return nAccessPermissions;
}

} // end anonymous namespace

PDFEncryptor::PDFEncryptor()
{
    /* prepare the cypher engine */
    m_aCipher = rtl_cipher_createARCFOUR(rtl_Cipher_ModeStream);
}

PDFEncryptor::~PDFEncryptor() { rtl_cipher_destroyARCFOUR(m_aCipher); }

/* init the encryption engine
1. init the document id, used both for building the document id and for building the encryption key(s)
2. build the encryption key following algorithms described in the PDF specification
 */
void PDFEncryptor::initEncryption(EncryptionHashTransporter& rEncryptionHashTransporter,
                                  const OUString& i_rOwnerPassword, const OUString& i_rUserPassword)
{
    if (!i_rOwnerPassword.isEmpty() || !i_rUserPassword.isEmpty())
    {
        // get padded passwords
        sal_uInt8 aPadUPW[ENCRYPTED_PWD_SIZE], aPadOPW[ENCRYPTED_PWD_SIZE];
        padPassword(i_rOwnerPassword.isEmpty() ? i_rUserPassword : i_rOwnerPassword, aPadOPW);
        padPassword(i_rUserPassword, aPadUPW);

        if (computeODictionaryValue(aPadOPW, aPadUPW, rEncryptionHashTransporter.getOValue(),
                                    SECUR_128BIT_KEY))
        {
            rEncryptionHashTransporter.getUDigest()->update(aPadUPW, ENCRYPTED_PWD_SIZE);
        }

        // trash temporary padded cleartext PWDs
        rtl_secureZeroMemory(aPadOPW, sizeof(aPadOPW));
        rtl_secureZeroMemory(aPadUPW, sizeof(aPadUPW));
    }
}

bool PDFEncryptor::prepareEncryption(
    const uno::Reference<beans::XMaterialHolder>& xEncryptionMaterialHolder,
    vcl::PDFEncryptionProperties& rProperties)
{
    bool bSuccess = false;
    EncryptionHashTransporter* pTransporter
        = EncryptionHashTransporter::getEncHashTransporter(xEncryptionMaterialHolder);
    if (pTransporter)
    {
        sal_Int32 nKeyLength = 0;
        sal_Int32 nRC4KeyLength = 0;
        sal_Int32 nAccessPermissions
            = computeAccessPermissions(rProperties, nKeyLength, nRC4KeyLength);
        rProperties.OValue = pTransporter->getOValue();
        bSuccess
            = computeUDictionaryValue(pTransporter, rProperties, nKeyLength, nAccessPermissions);
    }

    if (!bSuccess)
    {
        rProperties.OValue.clear();
        rProperties.UValue.clear();
        rProperties.EncryptionKey.clear();
    }
    return bSuccess;
}

/* this function implements part of the PDF spec algorithm 3.1 in encryption */
void PDFEncryptor::setupKeysAndCheck(vcl::PDFEncryptionProperties& rProperties)
{
    // sanity check
    if (rProperties.OValue.size() != ENCRYPTED_PWD_SIZE
        || rProperties.UValue.size() != ENCRYPTED_PWD_SIZE
        || rProperties.EncryptionKey.size() != MAXIMUM_RC4_KEY_LENGTH)
    {
        // the field lengths are invalid ? This was not setup by initEncryption.
        // do not encrypt after all
        rProperties.OValue.clear();
        rProperties.UValue.clear();
        OSL_ENSURE(false, "encryption data failed sanity check, encryption disabled");
    }
    else // setup key lengths
    {
        m_nAccessPermissions = computeAccessPermissions(rProperties, m_nKeyLength, m_nRC4KeyLength);
    }
}

void PDFEncryptor::enableStreamEncryption() { m_bEncryptThisStream = true; }

void PDFEncryptor::disableStreamEncryption() { m_bEncryptThisStream = false; }

void PDFEncryptor::setupEncryption(std::vector<sal_uInt8> const& rEncryptionKey, sal_Int32 nObject)
{
    std::vector<sal_uInt8> aKey(rEncryptionKey.begin(), rEncryptionKey.begin() + m_nKeyLength);
    std::vector<sal_uInt8> aObjectArray{
        sal_uInt8(nObject), sal_uInt8(nObject >> 8), sal_uInt8(nObject >> 16),
        0, // generation number, always zero
        0 // generation number, always zero
    };
    aKey.insert(aKey.end(), aObjectArray.begin(), aObjectArray.end());

    // do the MD5 hash
    auto const nMD5Sum
        = comphelper::Hash::calculateHash(aKey.data(), aKey.size(), ::comphelper::HashType::MD5);

    // initialize the RC4 with the key
    // key length: see algorithm 3.1, step 4: (N+5) max 16
    rtl_cipher_initARCFOUR(m_aCipher, rtl_Cipher_DirectionEncode, nMD5Sum.data(), getRC4KeyLength(),
                           nullptr, 0);
}

/* implement the encryption part of the PDF spec encryption algorithm 3.1 */
void PDFEncryptor::encrypt(const void* pInput, sal_uInt64 nInputSize, sal_uInt8* pOutput,
                           sal_uInt64 nOutputsSize)
{
    rtl_cipher_encodeARCFOUR(m_aCipher, pInput, sal_Size(nInputSize), pOutput,
                             sal_Size(nOutputsSize));
}

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
