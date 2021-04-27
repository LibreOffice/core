/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svl/msodocumentlockfile.hxx>
#include <algorithm>
#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

namespace svt
{
namespace
{
bool isWordFormat(const OUString& sExt)
{
    return sExt.equalsIgnoreAsciiCase("DOC") || sExt.equalsIgnoreAsciiCase("DOCX")
           || sExt.equalsIgnoreAsciiCase("RTF") || sExt.equalsIgnoreAsciiCase("ODT");
}

bool isExcelFormat(const OUString& sExt)
{
    return //sExt.equalsIgnoreAsciiCase("XLS") || // MSO does not create lockfile for XLS
        sExt.equalsIgnoreAsciiCase("XLSX") || sExt.equalsIgnoreAsciiCase("ODS");
}

bool isPowerPointFormat(const OUString& sExt)
{
    return sExt.equalsIgnoreAsciiCase("PPTX") || sExt.equalsIgnoreAsciiCase("PPT")
           || sExt.equalsIgnoreAsciiCase("ODP");
}

// Need to generate different lock file name for MSO.
OUString GenerateMSOLockFileURL(const OUString& aOrigURL)
{
    INetURLObject aURL = LockFileCommon::ResolveLinks(INetURLObject(aOrigURL));

    // For text documents MSO Word cuts some of the first characters of the file name
    OUString sFileName = aURL.GetLastName();
    const OUString sExt = aURL.GetFileExtension();

    if (isWordFormat(sExt))
    {
        const sal_Int32 nFileNameLength = sFileName.getLength() - sExt.getLength() - 1;
        if (nFileNameLength >= 8)
            sFileName = sFileName.copy(2);
        else if (nFileNameLength == 7)
            sFileName = sFileName.copy(1);
    }
    aURL.setName(OUString("~$" + sFileName));
    return aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);
}
}

// static
MSODocumentLockFile::AppType MSODocumentLockFile::getAppType(const OUString& sOrigURL)
{
    AppType eResult = AppType::PowerPoint;
    INetURLObject aDocURL = LockFileCommon::ResolveLinks(INetURLObject(sOrigURL));
    const OUString sExt = aDocURL.GetFileExtension();
    if (isWordFormat(sExt))
        eResult = AppType::Word;
    else if (isExcelFormat(sExt))
        eResult = AppType::Excel;

    return eResult;
}

MSODocumentLockFile::MSODocumentLockFile(const OUString& aOrigURL)
    : GenDocumentLockFile(GenerateMSOLockFileURL(aOrigURL))
    , m_eAppType(getAppType(aOrigURL))
{
}

MSODocumentLockFile::~MSODocumentLockFile() {}

void MSODocumentLockFile::WriteEntryToStream(
    const LockFileEntry& aEntry, const css::uno::Reference<css::io::XOutputStream>& xOutput)
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // Reallocate the date with the right size, different lock file size for different components
    int nLockFileSize = m_eAppType == AppType::Word ? MSO_WORD_LOCKFILE_SIZE
                                                    : MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE;
    css::uno::Sequence<sal_Int8> aData(nLockFileSize);

    // Write out the user name's length as a single byte integer
    // The maximum length is 52 in MSO, so we'll need to truncate the user name if it's longer
    OUString aUserName = aEntry[LockFileComponent::OOOUSERNAME];
    int nIndex = 0;
    aData[nIndex] = static_cast<sal_Int8>(
        std::min(aUserName.getLength(), sal_Int32(MSO_USERNAME_MAX_LENGTH)));

    if (aUserName.getLength() > MSO_USERNAME_MAX_LENGTH)
        aUserName = aUserName.copy(0, MSO_USERNAME_MAX_LENGTH);

    // From the second position write out the user name using one byte characters.
    nIndex = 1;
    for (int nChar = 0; nChar < aUserName.getLength(); ++nChar)
    {
        aData[nIndex] = static_cast<sal_Int8>(aUserName[nChar]);
        ++nIndex;
    }

    // Fill up the remaining bytes with dummy data
    switch (m_eAppType)
    {
        case AppType::Word:
            while (nIndex < MSO_USERNAME_MAX_LENGTH + 2)
            {
                aData[nIndex] = static_cast<sal_Int8>(0);
                ++nIndex;
            }
            break;
        case AppType::PowerPoint:
            aData[nIndex] = static_cast<sal_Int8>(0);
            ++nIndex;
            [[fallthrough]];
        case AppType::Excel:
            while (nIndex < MSO_USERNAME_MAX_LENGTH + 3)
            {
                aData[nIndex] = static_cast<sal_Int8>(0x20);
                ++nIndex;
            }
            break;
    }

    // At the next position we have the user name's length again, but now as a 2 byte integer
    aData[nIndex] = static_cast<sal_Int8>(
        std::min(aUserName.getLength(), sal_Int32(MSO_USERNAME_MAX_LENGTH)));
    ++nIndex;
    aData[nIndex] = 0;
    ++nIndex;

    // And the user name again with unicode characters
    for (int nChar = 0; nChar < aUserName.getLength(); ++nChar)
    {
        aData[nIndex] = static_cast<sal_Int8>(aUserName[nChar] & 0xff);
        ++nIndex;
        aData[nIndex] = static_cast<sal_Int8>(aUserName[nChar] >> 8);
        ++nIndex;
    }

    // Fill the remaining part with dummy bits
    switch (m_eAppType)
    {
        case AppType::Word:
            while (nIndex < nLockFileSize)
            {
                aData[nIndex] = static_cast<sal_Int8>(0);
                ++nIndex;
            }
            break;
        case AppType::Excel:
        case AppType::PowerPoint:
            while (nIndex < nLockFileSize)
            {
                aData[nIndex] = static_cast<sal_Int8>(0x20);
                ++nIndex;
                if (nIndex < nLockFileSize)
                {
                    aData[nIndex] = static_cast<sal_Int8>(0);
                    ++nIndex;
                }
            }
            break;
    }

    xOutput->writeBytes(aData);
}

css::uno::Reference<css::io::XInputStream> MSODocumentLockFile::OpenStream()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    css::uno::Reference<css::ucb::XCommandEnvironment> xEnv;
    ::ucbhelper::Content aSourceContent(GetURL(), xEnv, comphelper::getProcessComponentContext());

    // the file can be opened readonly, no locking will be done
    return aSourceContent.openStreamNoLock();
}

LockFileEntry MSODocumentLockFile::GetLockData()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    LockFileEntry aResult;
    css::uno::Reference<css::io::XInputStream> xInput = OpenStream();
    if (!xInput.is())
        throw css::uno::RuntimeException();

    const sal_Int32 nBufLen = 256;
    css::uno::Sequence<sal_Int8> aBuf(nBufLen);
    const sal_Int32 nRead = xInput->readBytes(aBuf, nBufLen);
    xInput->closeInput();
    if (nRead >= 162)
    {
        // Reverse engineering of MS Office Owner Files format (MS Office 2016 tested).
        // It starts with a single byte with name length, after which characters of username go
        // in current Windows 8-bit codepage.
        // For Word lockfiles, the name is followed by zero bytes up to position 54.
        // For PowerPoint lockfiles, the name is followed by a single zero byte, and then 0x20
        // bytes up to position 55.
        // For Excel lockfiles, the name is followed by 0x20 bytes up to position 55.
        // At those positions in each type of lockfile, a name length 2-byte word goes, followed
        // by UTF-16-LE-encoded copy of username. Spaces or some garbage follow up to the end of
        // the lockfile (total 162 bytes for Word, 165 bytes for Excel/PowerPoint).
        // Apparently MS Office does not allow username to be longer than 52 characters (trying
        // to enter more in its options dialog results in error messages stating this limit).
        const int nACPLen = aBuf[0];
        if (nACPLen > 0 && nACPLen <= 52) // skip wrong format
        {
            const sal_Int8* pBuf = aBuf.getConstArray() + 54;
            int nUTF16Len = *pBuf; // try Word position
            // If UTF-16 length is 0x20, then ACP length is also less than maximal, which means
            // that in Word lockfile case, at least two preceding bytes would be zero. Both
            // Excel and PowerPoint lockfiles would have at least one of those bytes non-zero.
            if (nUTF16Len == 0x20 && (*(pBuf - 1) != 0 || *(pBuf - 2) != 0))
                nUTF16Len = *++pBuf; // use Excel/PowerPoint position

            if (nUTF16Len > 0 && nUTF16Len <= 52) // skip wrong format
            {
                OUStringBuffer str(nUTF16Len);
                sal_uInt8 const* p = reinterpret_cast<sal_uInt8 const*>(pBuf + 2);
                for (int i = 0; i != nUTF16Len; ++i)
                {
                    str.append(sal_Unicode(p[0] | (sal_uInt32(p[1]) << 8)));
                    p += 2;
                }
                aResult[LockFileComponent::OOOUSERNAME] = str.makeStringAndClear();
            }
        }
    }
    return aResult;
}

void MSODocumentLockFile::RemoveFile()
{
    ::osl::MutexGuard aGuard(m_aMutex);

    // TODO/LATER: the removing is not atomic, is it possible in general to make it atomic?
    LockFileEntry aNewEntry = GenerateOwnEntry();
    LockFileEntry aFileData = GetLockData();

    if (aFileData[LockFileComponent::OOOUSERNAME] != aNewEntry[LockFileComponent::OOOUSERNAME])
        throw css::io::IOException(); // not the owner, access denied

    RemoveFileDirectly();
}

bool MSODocumentLockFile::IsMSOSupportedFileFormat(const OUString& aURL)
{
    INetURLObject aDocURL = LockFileCommon::ResolveLinks(INetURLObject(aURL));
    const OUString sExt = aDocURL.GetFileExtension();

    return isWordFormat(sExt) || isExcelFormat(sExt) || isPowerPointFormat(sExt);
}

} // namespace svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
