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
#include <pdf/pdfwriter_impl.hxx>
#include <comphelper/hash.hxx>

namespace vcl::pdf
{
/*************************************************************
begin i12626 methods

Implements Algorithm 3.2, step 1 only
*/
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
        o_pPaddedPW[i] = PDFEncryptor::s_nPadString[y];
}

/**********************************
Algorithm 3.2  Compute the encryption key used

step 1 should already be done before calling, the paThePaddedPassword parameter should contain
the padded password and must be 32 byte long, the encryption key is returned into the paEncryptionKey parameter,
it will be 16 byte long for 128 bit security; for 40 bit security only the first 5 bytes are used

TODO: in pdf ver 1.5 and 1.6 the step 6 is different, should be implemented. See spec.

*/
bool computeEncryptionKey(EncryptionHashTransporter* i_pTransporter,
                          vcl::PDFWriter::PDFEncryptionProperties& io_rProperties,
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

/**********************************
Algorithm 3.3  Compute the encryption dictionary /O value, save into the class data member
the step numbers down here correspond to the ones in PDF v.1.4 specification
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
                        io_rOValue.data(),
                        sal_Int32(
                            io_rOValue
                                .size())); // encrypted data, can be the same as the input, encrypt "in place"
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

/**********************************
Algorithms 3.4 and 3.5  Compute the encryption dictionary /U value, save into the class data member, revision 2 (40 bit) or 3 (128 bit)
*/
bool computeUDictionaryValue(EncryptionHashTransporter* i_pTransporter,
                             vcl::PDFWriter::PDFEncryptionProperties& io_rProperties,
                             sal_Int32 i_nKeyLength, sal_Int32 i_nAccessPermissions)
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
            aDigest.update(PDFEncryptor::s_nPadString, sizeof(PDFEncryptor::s_nPadString));
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

void computeDocumentIdentifier(std::vector<sal_uInt8>& o_rIdentifier,
                               const vcl::PDFWriter::PDFDocInfo& i_rDocInfo,
                               const OString& i_rCString1,
                               const css::util::DateTime& rCreationMetaDate, OString& o_rCString2)
{
    o_rIdentifier.clear();

    //build the document id
    OString aInfoValuesOut;
    OStringBuffer aID(1024);
    if (!i_rDocInfo.Title.isEmpty())
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Title, aID);
    if (!i_rDocInfo.Author.isEmpty())
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Author, aID);
    if (!i_rDocInfo.Subject.isEmpty())
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Subject, aID);
    if (!i_rDocInfo.Keywords.isEmpty())
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Keywords, aID);
    if (!i_rDocInfo.Creator.isEmpty())
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Creator, aID);
    if (!i_rDocInfo.Producer.isEmpty())
        PDFWriter::AppendUnicodeTextString(i_rDocInfo.Producer, aID);

    TimeValue aTVal, aGMT;
    oslDateTime aDT;
    aDT.NanoSeconds = rCreationMetaDate.NanoSeconds;
    aDT.Seconds = rCreationMetaDate.Seconds;
    aDT.Minutes = rCreationMetaDate.Minutes;
    aDT.Hours = rCreationMetaDate.Hours;
    aDT.Day = rCreationMetaDate.Day;
    aDT.Month = rCreationMetaDate.Month;
    aDT.Year = rCreationMetaDate.Year;

    osl_getSystemTime(&aGMT);
    osl_getLocalTimeFromSystemTime(&aGMT, &aTVal);
    OStringBuffer aCreationMetaDateString(64);

    // i59651: we fill the Metadata date string as well, if PDF/A is requested
    // according to ISO 19005-1:2005 6.7.3 the date is corrected for
    // local time zone offset UTC only, whereas Acrobat 8 seems
    // to use the localtime notation only
    // according to a recommendation in XMP Specification (Jan 2004, page 75)
    // the Acrobat way seems the right approach
    aCreationMetaDateString.append(OStringChar(static_cast<char>('0' + ((aDT.Year / 1000) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Year / 100) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Year / 10) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Year) % 10))) + "-"
                                   + OStringChar(static_cast<char>('0' + ((aDT.Month / 10) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Month) % 10))) + "-"
                                   + OStringChar(static_cast<char>('0' + ((aDT.Day / 10) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Day) % 10))) + "T"
                                   + OStringChar(static_cast<char>('0' + ((aDT.Hours / 10) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Hours) % 10))) + ":"
                                   + OStringChar(static_cast<char>('0' + ((aDT.Minutes / 10) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Minutes) % 10)))
                                   + ":"
                                   + OStringChar(static_cast<char>('0' + ((aDT.Seconds / 10) % 10)))
                                   + OStringChar(static_cast<char>('0' + ((aDT.Seconds) % 10))));

    sal_uInt32 nDelta = 0;
    if (aGMT.Seconds > aTVal.Seconds)
    {
        nDelta = aGMT.Seconds - aTVal.Seconds;
        aCreationMetaDateString.append("-");
    }
    else if (aGMT.Seconds < aTVal.Seconds)
    {
        nDelta = aTVal.Seconds - aGMT.Seconds;
        aCreationMetaDateString.append("+");
    }
    else
    {
        aCreationMetaDateString.append("Z");
    }
    if (nDelta)
    {
        aCreationMetaDateString.append(
            OStringChar(static_cast<char>('0' + ((nDelta / 36000) % 10)))
            + OStringChar(static_cast<char>('0' + ((nDelta / 3600) % 10))) + ":"
            + OStringChar(static_cast<char>('0' + ((nDelta / 600) % 6)))
            + OStringChar(static_cast<char>('0' + ((nDelta / 60) % 10))));
    }
    aID.append(i_rCString1.getStr(), i_rCString1.getLength());

    aInfoValuesOut = aID.makeStringAndClear();
    o_rCString2 = aCreationMetaDateString.makeStringAndClear();

    ::comphelper::Hash aDigest(::comphelper::HashType::MD5);
    aDigest.update(reinterpret_cast<unsigned char const*>(&aGMT), sizeof(aGMT));
    aDigest.update(reinterpret_cast<unsigned char const*>(aInfoValuesOut.getStr()),
                   aInfoValuesOut.getLength());
    //the binary form of the doc id is needed for encryption stuff
    o_rIdentifier = aDigest.finalize();
}

sal_Int32 computeAccessPermissions(const vcl::PDFWriter::PDFEncryptionProperties& i_rProperties,
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

} // end vcl::pdf

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
