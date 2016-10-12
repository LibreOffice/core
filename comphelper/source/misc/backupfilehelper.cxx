/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/deployment/XPackage.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/deployment/XExtensionManager.hpp>
#include <com/sun/star/task/XAbortChannel.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/deployment/ExtensionManager.hpp>
#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <comphelper/backupfilehelper.hxx>
#include <rtl/crc.h>
#include <algorithm>
#include <deque>
#include <vector>
#include <zlib.h>

using namespace css;
typedef std::shared_ptr< osl::File > FileSharedPtr;
static const sal_uInt32 BACKUP_FILE_HELPER_BLOCK_SIZE = 16384;

namespace
{
    OUString splitAtLastToken(const OUString& rSrc, sal_Unicode aToken, OUString& rRight)
    {
        const sal_Int32 nIndex(rSrc.lastIndexOf(aToken));
        OUString aRetval;

        if (-1 == nIndex)
        {
            aRetval = rSrc;
        }
        else if (nIndex > 0)
        {
            aRetval = rSrc.copy(0, nIndex);
        }

        if (rSrc.getLength() > nIndex + 1)
        {
            rRight = rSrc.copy(nIndex + 1);
        }

        return aRetval;
    }

    sal_uInt32 createCrc32(FileSharedPtr& rCandidate, sal_uInt32 nOffset)
    {
        sal_uInt32 nCrc32(0);

        if (rCandidate && osl::File::E_None == rCandidate->open(osl_File_OpenFlag_Read))
        {
            sal_uInt8 aArray[BACKUP_FILE_HELPER_BLOCK_SIZE];
            sal_uInt64 nBytesTransfer(0);
            sal_uInt64 nSize(0);

            rCandidate->getSize(nSize);

            // set offset in source file - should be zero due to crc32 should
            // only be needed to be created for new entries, gets loaded with old
            // ones
            if (osl::File::E_None == rCandidate->setPos(osl_Pos_Absolut, sal_Int64(nOffset)))
            {
                while (nSize != 0)
                {
                    const sal_uInt64 nToTransfer(std::min(nSize, (sal_uInt64)BACKUP_FILE_HELPER_BLOCK_SIZE));

                    if (osl::File::E_None == rCandidate->read(static_cast<void*>(aArray), nToTransfer, nBytesTransfer) && nBytesTransfer == nToTransfer)
                    {
                        // add to crc and reduce size
                        nCrc32 = rtl_crc32(nCrc32, static_cast<void*>(aArray), static_cast<sal_uInt32>(nBytesTransfer));
                        nSize -= nToTransfer;
                    }
                    else
                    {
                        // error - reset to zero again
                        nSize = nCrc32 = 0;
                    }
                }
            }

            rCandidate->close();
        }

        return nCrc32;
    }

    bool read_sal_uInt32(FileSharedPtr& rFile, sal_uInt32& rTarget)
    {
        sal_uInt8 aArray[4];
        sal_uInt64 nBaseRead(0);

        // read rTarget
        if (osl::File::E_None == rFile->read(static_cast<void*>(aArray), 4, nBaseRead) && 4 == nBaseRead)
        {
            rTarget = (sal_uInt32(aArray[0]) << 24) + (sal_uInt32(aArray[1]) << 16) + (sal_uInt32(aArray[2]) << 8) + sal_uInt32(aArray[3]);
            return true;
        }

        return false;
    }

    bool write_sal_uInt32(oslFileHandle& rHandle, sal_uInt32 nSource)
    {
        sal_uInt8 aArray[4];
        sal_uInt64 nBaseWritten(0);

        // write nSource
        aArray[0] = sal_uInt8((nSource & 0xff000000) >> 24);
        aArray[1] = sal_uInt8((nSource & 0x00ff0000) >> 16);
        aArray[2] = sal_uInt8((nSource & 0x0000ff00) >> 8);
        aArray[3] = sal_uInt8(nSource & 0x000000ff);

        if (osl_File_E_None == osl_writeFile(rHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) && 4 == nBaseWritten)
        {
            return true;
        }

        return false;
    }

    bool read_OString(FileSharedPtr& rFile, OString& rTarget)
    {
        sal_uInt32 nLength(0);

        if (!read_sal_uInt32(rFile, nLength))
        {
            return false;
        }

        std::vector< sal_Char > aTarget(nLength);
        sal_uInt64 nBaseRead(0);

        // read rTarget
        if (osl::File::E_None == rFile->read(static_cast<void*>(&aTarget[0]), nLength, nBaseRead) && nLength == nBaseRead)
        {
            rTarget = OString(&aTarget[0], static_cast< sal_Int32 >(nLength));
            return true;
        }

        return false;
    }

    bool write_OString(oslFileHandle& rHandle, const OString& rSource)
    {
        const sal_uInt32 nLength(rSource.getLength());

        if (!write_sal_uInt32(rHandle, nLength))
        {
            return false;
        }

        sal_uInt64 nBaseWritten(0);

        if (osl_File_E_None == osl_writeFile(rHandle, static_cast<const void*>(rSource.getStr()), nLength, &nBaseWritten) && nLength == nBaseWritten)
        {
            return true;
        }

        return false;
    }

    bool fileExists(const OUString& rBaseURL)
    {
        if (!rBaseURL.isEmpty())
        {
            FileSharedPtr aBaseFile(new osl::File(rBaseURL));

            return (osl::File::E_None == aBaseFile->open(osl_File_OpenFlag_Read));
        }

        return false;
    }
}

namespace
{
    enum PackageState { REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE };

    class ExtensionInfoEntry
    {
    private:
        PackageState    meState;            // REGISTERED, NOT_REGISTERED, AMBIGUOUS, NOT_AVAILABLE
        OString         maRepositoryName;   // user|shared|bundled
        OString         maName;
        OString         maIdentifier;
        OString         maVersion;

    public:
        ExtensionInfoEntry()
        :   meState(NOT_AVAILABLE),
            maRepositoryName(),
            maName(),
            maIdentifier(),
            maVersion()
        {
        }

        ExtensionInfoEntry(const uno::Reference< deployment::XPackage >& rxPackage)
        :   meState(NOT_AVAILABLE),
            maRepositoryName(OUStringToOString(rxPackage->getRepositoryName(), RTL_TEXTENCODING_ASCII_US)),
            maName(OUStringToOString(rxPackage->getName(), RTL_TEXTENCODING_ASCII_US)),
            maIdentifier(OUStringToOString(rxPackage->getIdentifier().Value, RTL_TEXTENCODING_ASCII_US)),
            maVersion(OUStringToOString(rxPackage->getVersion(), RTL_TEXTENCODING_ASCII_US))
        {
            const beans::Optional< beans::Ambiguous< sal_Bool > > option(
                rxPackage->isRegistered(uno::Reference< task::XAbortChannel >(),
                uno::Reference< ucb::XCommandEnvironment >()));

            if (option.IsPresent)
            {
                ::beans::Ambiguous< sal_Bool > const& reg = option.Value;

                if (reg.IsAmbiguous)
                {
                    meState = AMBIGUOUS;
                }
                else
                {
                    meState = reg.Value ? REGISTERED : NOT_REGISTERED;
                }
            }
            else
            {
                meState = NOT_AVAILABLE;
            }
        }

        bool operator<(const ExtensionInfoEntry& rComp) const
        {
            if (0 == maRepositoryName.compareTo(rComp.maRepositoryName))
            {
                if (0 == maName.compareTo(rComp.maName))
                {
                    if (0 == maVersion.compareTo(rComp.maVersion))
                    {
                        if (0 == maIdentifier.compareTo(rComp.maIdentifier))
                        {
                            return meState < rComp.meState;
                        }
                        else
                        {
                            return 0 > maIdentifier.compareTo(rComp.maIdentifier);
                        }
                    }
                    else
                    {
                        return 0 > maVersion.compareTo(rComp.maVersion);
                    }
                }
                else
                {
                    return 0 > maName.compareTo(rComp.maName);
                }
            }
            else
            {
                return 0 > maRepositoryName.compareTo(rComp.maRepositoryName);
            }
        }

        bool read_entry(FileSharedPtr& rFile)
        {
            // read meState
            sal_uInt32 nState(0);

            if (read_sal_uInt32(rFile, nState))
            {
                meState = static_cast< PackageState >(nState);
            }
            else
            {
                return false;
            }

            // read maRepositoryName;
            if (!read_OString(rFile, maRepositoryName))
            {
                return false;
            }

            // read maName;
            if (!read_OString(rFile, maName))
            {
                return false;
            }

            // read maIdentifier;
            if (!read_OString(rFile, maIdentifier))
            {
                return false;
            }

            // read maVersion;
            if (!read_OString(rFile, maVersion))
            {
                return false;
            }

            return true;
        }

        bool write_entry(oslFileHandle& rHandle) const
        {
            // write meState
            const sal_uInt32 nState(meState);

            if (!write_sal_uInt32(rHandle, nState))
            {
                return false;
            }

            // write maRepositoryName
            if (!write_OString(rHandle, maRepositoryName))
            {
                return false;
            }

            // write maName;
            if (!write_OString(rHandle, maName))
            {
                return false;
            }

            // write maIdentifier;
            if (!write_OString(rHandle, maIdentifier))
            {
                return false;
            }

            // write maVersion;
            if (!write_OString(rHandle, maVersion))
            {
                return false;
            }

            return true;
        }
    };

    typedef ::std::vector< ExtensionInfoEntry > ExtensionInfoEntryVector;

    class ExtensionInfo
    {
    private:
        ExtensionInfoEntryVector    maEntries;

    public:
        ExtensionInfo()
        :   maEntries()
        {
        }

        void reset()
        {
            // clear all data
            maEntries.clear();
        }

        void createCurrent()
        {
            // clear all data
            reset();

            // create content from current extension configuration
            uno::Sequence< uno::Sequence< uno::Reference< deployment::XPackage > > > xAllPackages;
            uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
            uno::Reference< deployment::XExtensionManager > m_xExtensionManager = deployment::ExtensionManager::get(xContext);

            try
            {
                xAllPackages = m_xExtensionManager->getAllExtensions(uno::Reference< task::XAbortChannel >(),
                    uno::Reference< ucb::XCommandEnvironment >());
            }
            catch (const deployment::DeploymentException &)
            {
                return;
            }
            catch (const ucb::CommandFailedException &)
            {
                return;
            }
            catch (const ucb::CommandAbortedException &)
            {
                return;
            }
            catch (const lang::IllegalArgumentException & e)
            {
                throw uno::RuntimeException(e.Message, e.Context);
            }

            for (sal_Int32 i = 0; i < xAllPackages.getLength(); ++i)
            {
                uno::Sequence< uno::Reference< deployment::XPackage > > xPackageList = xAllPackages[i];

                for (sal_Int32 j = 0; j < xPackageList.getLength(); ++j)
                {
                    uno::Reference< deployment::XPackage > xPackage = xPackageList[j];

                    if (xPackage.is())
                    {
                        maEntries.push_back(ExtensionInfoEntry(xPackage));
                    }
                }
            }

            if (!maEntries.empty())
            {
                // sort the list
                std::sort(maEntries.begin(), maEntries.end());
            }
        }

        bool read_entries(FileSharedPtr& rFile)
        {
            // read NumExtensionEntries
            sal_uInt32 nExtEntries(0);

            if (!read_sal_uInt32(rFile, nExtEntries))
            {
                return false;
            }

            for (sal_uInt32 a(0); a < nExtEntries; a++)
            {
                ExtensionInfoEntry aNewEntry;

                if (aNewEntry.read_entry(rFile))
                {
                    maEntries.push_back(aNewEntry);
                }
                else
                {
                    return false;
                }
            }

            return true;
        }

        bool write_entries(oslFileHandle& rHandle) const
        {
            const sal_uInt32 nExtEntries(maEntries.size());

            if (!write_sal_uInt32(rHandle, nExtEntries))
            {
                return false;
            }

            for (const auto& a : maEntries)
            {
                if (!a.write_entry(rHandle))
                {
                    return false;
                }
            }

            return true;
        }

        bool createTempFile(OUString& rTempFileName)
        {
            oslFileHandle aHandle;
            bool bRetval(false);

            // create current configuration
            if (maEntries.empty())
            {
                createCurrent();
            }

            // open target temp file and write current configuration to it - it exists until deleted
            if (osl::File::E_None == osl::FileBase::createTempFile(nullptr, &aHandle, &rTempFileName))
            {
                bRetval = write_entries(aHandle);

                // close temp file - it exists until deleted
                osl_closeFile(aHandle);
            }

            return bRetval;
        }
    };
}

namespace
{
    class PackedFileEntry
    {
    private:
        sal_uInt32          mnFullFileSize;     // size in bytes of unpacked original file
        sal_uInt32          mnPackFileSize;     // size in bytes in file backup package (smaller if compressed, same if not)
        sal_uInt32          mnOffset;           // offset in File (zero identifies new file)
        sal_uInt32          mnCrc32;            // checksum
        FileSharedPtr       maFile;             // file where to find the data (at offset)
        bool                mbDoCompress;       // flag if this file is scheduled to be compredded when written

        bool copy_content_straight(oslFileHandle& rTargetHandle)
        {
            if (maFile && osl::File::E_None == maFile->open(osl_File_OpenFlag_Read))
            {
                sal_uInt8 aArray[BACKUP_FILE_HELPER_BLOCK_SIZE];
                sal_uInt64 nBytesTransfer(0);
                sal_uInt64 nSize(getPackFileSize());

                // set offset in source file - when this is zero, a new file is to be added
                if (osl::File::E_None == maFile->setPos(osl_Pos_Absolut, sal_Int64(getOffset())))
                {
                    while (nSize != 0)
                    {
                        const sal_uInt64 nToTransfer(std::min(nSize, (sal_uInt64)BACKUP_FILE_HELPER_BLOCK_SIZE));

                        if (osl::File::E_None != maFile->read(static_cast<void*>(aArray), nToTransfer, nBytesTransfer) || nBytesTransfer != nToTransfer)
                        {
                            break;
                        }

                        if (osl_File_E_None != osl_writeFile(rTargetHandle, static_cast<const void*>(aArray), nToTransfer, &nBytesTransfer) || nBytesTransfer != nToTransfer)
                        {
                            break;
                        }

                        nSize -= nToTransfer;
                    }
                }

                maFile->close();
                return (0 == nSize);
            }

            return false;
        }

        bool copy_content_compress(oslFileHandle& rTargetHandle)
        {
            if (maFile && osl::File::E_None == maFile->open(osl_File_OpenFlag_Read))
            {
                sal_uInt8 aArray[BACKUP_FILE_HELPER_BLOCK_SIZE];
                sal_uInt8 aBuffer[BACKUP_FILE_HELPER_BLOCK_SIZE];
                sal_uInt64 nBytesTransfer(0);
                sal_uInt64 nSize(getPackFileSize());
                std::unique_ptr< z_stream > zstream(new z_stream);
                memset(zstream.get(), 0, sizeof(*zstream));

                if (Z_OK == deflateInit(zstream.get(), Z_BEST_COMPRESSION))
                {
                    // set offset in source file - when this is zero, a new file is to be added
                    if (osl::File::E_None == maFile->setPos(osl_Pos_Absolut, sal_Int64(getOffset())))
                    {
                        bool bOkay(true);

                        while (bOkay && nSize != 0)
                        {
                            const sal_uInt64 nToTransfer(std::min(nSize, (sal_uInt64)BACKUP_FILE_HELPER_BLOCK_SIZE));

                            if (osl::File::E_None != maFile->read(static_cast<void*>(aArray), nToTransfer, nBytesTransfer) || nBytesTransfer != nToTransfer)
                            {
                                break;
                            }

                            zstream->avail_in = nToTransfer;
                            zstream->next_in = reinterpret_cast<unsigned char*>(aArray);

                            do {
                                zstream->avail_out = BACKUP_FILE_HELPER_BLOCK_SIZE;
                                zstream->next_out = reinterpret_cast<unsigned char*>(aBuffer);
#if !defined Z_PREFIX
                                const sal_Int64 nRetval(deflate(zstream.get(), nSize == nToTransfer ? Z_FINISH : Z_NO_FLUSH));
#else
                                const sal_Int64 nRetval(z_deflate(zstream.get(), nSize == nToTransfer ? Z_FINISH : Z_NO_FLUSH));
#endif
                                if (Z_STREAM_ERROR == nRetval)
                                {
                                    bOkay = false;
                                }
                                else
                                {
                                    const sal_uInt64 nAvailable(BACKUP_FILE_HELPER_BLOCK_SIZE - zstream->avail_out);

                                    if (osl_File_E_None != osl_writeFile(rTargetHandle, static_cast<const void*>(aBuffer), nAvailable, &nBytesTransfer) || nBytesTransfer != nAvailable)
                                    {
                                        bOkay = false;
                                    }
                                }
                            } while (bOkay && 0 == zstream->avail_out);

                            if (!bOkay)
                            {
                                break;
                            }

                            nSize -= nToTransfer;
                        }

#if !defined Z_PREFIX
                        deflateEnd(zstream.get());
#else
                        z_deflateEnd(zstream.get());
#endif
                    }
                }

                maFile->close();

                // get compressed size and add to entry
                if (mnFullFileSize == mnPackFileSize && mnFullFileSize == zstream->total_in)
                {
                    mnPackFileSize = zstream->total_out;
                }

                return (0 == nSize);
            }

            return false;
        }

        bool copy_content_uncompress(oslFileHandle& rTargetHandle)
        {
            if (maFile && osl::File::E_None == maFile->open(osl_File_OpenFlag_Read))
            {
                sal_uInt8 aArray[BACKUP_FILE_HELPER_BLOCK_SIZE];
                sal_uInt8 aBuffer[BACKUP_FILE_HELPER_BLOCK_SIZE];
                sal_uInt64 nBytesTransfer(0);
                sal_uInt64 nSize(getPackFileSize());
                std::unique_ptr< z_stream > zstream(new z_stream);
                memset(zstream.get(), 0, sizeof(*zstream));

                if (Z_OK == inflateInit(zstream.get()))
                {
                    // set offset in source file - when this is zero, a new file is to be added
                    if (osl::File::E_None == maFile->setPos(osl_Pos_Absolut, sal_Int64(getOffset())))
                    {
                        bool bOkay(true);

                        while (bOkay && nSize != 0)
                        {
                            const sal_uInt64 nToTransfer(std::min(nSize, (sal_uInt64)BACKUP_FILE_HELPER_BLOCK_SIZE));

                            if (osl::File::E_None != maFile->read(static_cast<void*>(aArray), nToTransfer, nBytesTransfer) || nBytesTransfer != nToTransfer)
                            {
                                break;
                            }

                            zstream->avail_in = nToTransfer;
                            zstream->next_in = reinterpret_cast<unsigned char*>(aArray);

                            do {
                                zstream->avail_out = BACKUP_FILE_HELPER_BLOCK_SIZE;
                                zstream->next_out = reinterpret_cast<unsigned char*>(aBuffer);
#if !defined Z_PREFIX
                                const sal_Int64 nRetval(inflate(zstream.get(), Z_NO_FLUSH));
#else
                                const sal_Int64 nRetval(z_inflate(zstream.get(), Z_NO_FLUSH));
#endif
                                if (Z_STREAM_ERROR == nRetval)
                                {
                                    bOkay = false;
                                }
                                else
                                {
                                    const sal_uInt64 nAvailable(BACKUP_FILE_HELPER_BLOCK_SIZE - zstream->avail_out);

                                    if (osl_File_E_None != osl_writeFile(rTargetHandle, static_cast<const void*>(aBuffer), nAvailable, &nBytesTransfer) || nBytesTransfer != nAvailable)
                                    {
                                        bOkay = false;
                                    }
                                }
                            } while (bOkay && 0 == zstream->avail_out);

                            if (!bOkay)
                            {
                                break;
                            }

                            nSize -= nToTransfer;
                        }

#if !defined Z_PREFIX
                        deflateEnd(zstream.get());
#else
                        z_deflateEnd(zstream.get());
#endif
                    }
                }

                maFile->close();
                return (0 == nSize);
            }

            return false;
        }


    public:
        // create new, uncompressed entry
        PackedFileEntry(
            sal_uInt32 nFullFileSize,
            sal_uInt32 nCrc32,
            FileSharedPtr& rFile,
            bool bDoCompress)
        :   mnFullFileSize(nFullFileSize),
            mnPackFileSize(nFullFileSize),
            mnOffset(0),
            mnCrc32(nCrc32),
            maFile(rFile),
            mbDoCompress(bDoCompress)
        {
        }

        // create entry to be loaded as header (read_header)
        PackedFileEntry()
        :   mnFullFileSize(0),
            mnPackFileSize(0),
            mnOffset(0),
            mnCrc32(0),
            maFile(),
            mbDoCompress(false)
        {
        }

        sal_uInt32 getFullFileSize() const
        {
            return  mnFullFileSize;
        }

        sal_uInt32 getPackFileSize() const
        {
            return  mnPackFileSize;
        }

        sal_uInt32 getOffset() const
        {
            return mnOffset;
        }

        void setOffset(sal_uInt32 nOffset)
        {
            mnOffset = nOffset;
        }

        static sal_uInt32 getEntrySize()
        {
            return 12;
        }

        sal_uInt32 getCrc32() const
        {
            return mnCrc32;
        }

        bool read_header(FileSharedPtr& rFile)
        {
            if (!rFile)
            {
                return false;
            }

            maFile = rFile;

            // read and compute full file size
            if (!read_sal_uInt32(rFile, mnFullFileSize))
            {
                return false;
            }

            // read and compute entry crc32
            if (!read_sal_uInt32(rFile, mnCrc32))
            {
                return false;
            }

            // read and compute packed size
            if (!read_sal_uInt32(rFile, mnPackFileSize))
            {
                return false;
            }

            return true;
        }

        bool write_header(oslFileHandle& rHandle) const
        {
            // write full file size
            if (!write_sal_uInt32(rHandle, mnFullFileSize))
            {
                return false;
            }

            // write crc32
            if (!write_sal_uInt32(rHandle, mnCrc32))
            {
                return false;
            }

            // write packed file size
            if (!write_sal_uInt32(rHandle, mnPackFileSize))
            {
                return false;
            }

            return true;
        }

        bool copy_content(oslFileHandle& rTargetHandle, bool bUncompress)
        {
            if (bUncompress)
            {
                if (getFullFileSize() == getPackFileSize())
                {
                    // not compressed, just copy
                    return copy_content_straight(rTargetHandle);
                }
                else
                {
                    // compressed, need to uncompress on copy
                    return copy_content_uncompress(rTargetHandle);
                }
            }
            else if (0 == getOffset())
            {
                if (mbDoCompress)
                {
                    // compressed wanted, need to compress on copy
                    return copy_content_compress(rTargetHandle);
                }
                else
                {
                    // not compressed, straight copy
                    return copy_content_straight(rTargetHandle);
                }
            }
            else
            {
                return copy_content_straight(rTargetHandle);
            }
        }
    };
}

namespace
{
    typedef ::std::deque< PackedFileEntry > PackedFileEntryVector;

    class PackedFile
    {
    private:
        const OUString          maURL;
        PackedFileEntryVector   maPackedFileEntryVector;
        bool                    mbChanged;

    public:
        PackedFile(const OUString& rURL)
        :   maURL(rURL),
            maPackedFileEntryVector(),
            mbChanged(false)
        {
            FileSharedPtr aSourceFile(new osl::File(rURL));

            if (osl::File::E_None == aSourceFile->open(osl_File_OpenFlag_Read))
            {
                sal_uInt64 nBaseLen(0);
                aSourceFile->getSize(nBaseLen);

                // we need at least File_ID and num entries -> 8byte
                if (8 < nBaseLen)
                {
                    sal_uInt8 aArray[4];
                    sal_uInt64 nBaseRead(0);

                    // read and check File_ID
                    if (osl::File::E_None == aSourceFile->read(static_cast< void* >(aArray), 4, nBaseRead) && 4 == nBaseRead)
                    {
                        if ('P' == aArray[0] && 'A' == aArray[1] && 'C' == aArray[2] && 'K' == aArray[3])
                        {
                            // read and compute num entries in this file
                            if (osl::File::E_None == aSourceFile->read(static_cast<void*>(aArray), 4, nBaseRead) && 4 == nBaseRead)
                            {
                                sal_uInt32 nEntries((sal_uInt32(aArray[0]) << 24) + (sal_uInt32(aArray[1]) << 16) + (sal_uInt32(aArray[2]) << 8) + sal_uInt32(aArray[3]));

                                // if there are entries (and less than max), read them
                                if (nEntries >= 1 && nEntries <= 10)
                                {
                                    for (sal_uInt32 a(0); a < nEntries; a++)
                                    {
                                        // create new entry, read header (size, crc and PackedSize),
                                        // set offset and source file
                                        PackedFileEntry aEntry;

                                        if (aEntry.read_header(aSourceFile))
                                        {
                                            // add to local data
                                            maPackedFileEntryVector.push_back(aEntry);
                                        }
                                        else
                                        {
                                            // error
                                            nEntries = 0;
                                        }
                                    }

                                    if (0 == nEntries)
                                    {
                                        // on read error clear local data
                                        maPackedFileEntryVector.clear();
                                    }
                                    else
                                    {
                                        // calculate and set offsets to file binary content
                                        sal_uInt32 nHeaderSize(8);

                                        nHeaderSize += maPackedFileEntryVector.size() * PackedFileEntry::getEntrySize();

                                        sal_uInt32 nOffset(nHeaderSize);

                                        for (auto& b : maPackedFileEntryVector)
                                        {
                                            b.setOffset(nOffset);
                                            nOffset += b.getPackFileSize();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                aSourceFile->close();
            }

            if (maPackedFileEntryVector.empty())
            {
                // on error or no data get rid of pack file
                osl::File::remove(maURL);
            }
        }

        bool flush()
        {
            bool bRetval(true);

            if (maPackedFileEntryVector.empty())
            {
                // get rid of (now?) empty pack file
                osl::File::remove(maURL);
            }
            else if (mbChanged)
            {
                // need to create a new pack file, do this in a temp file to which data
                // will be copied from local file (so keep it here until this is done)
                oslFileHandle aHandle;
                OUString aTempURL;

                // open target temp file - it exists until deleted
                if (osl::File::E_None == osl::FileBase::createTempFile(nullptr, &aHandle, &aTempURL))
                {
                    sal_uInt8 aArray[4];
                    sal_uInt64 nBaseWritten(0);

                    aArray[0] = 'P';
                    aArray[1] = 'A';
                    aArray[2] = 'C';
                    aArray[3] = 'K';

                    // write File_ID
                    if (osl_File_E_None == osl_writeFile(aHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) && 4 == nBaseWritten)
                    {
                        const sal_uInt32 nSize(maPackedFileEntryVector.size());

                        // write number of entries
                        if (write_sal_uInt32(aHandle, nSize))
                        {
                            if (bRetval)
                            {
                                // write placeholder for headers. Due to the fact that
                                // PackFileSize for newly added files gets set during
                                // writing the content entry, write headers after content
                                // is written. To do so, write placeholders here
                                sal_uInt32 nWriteSize(0);

                                nWriteSize += maPackedFileEntryVector.size() * PackedFileEntry::getEntrySize();

                                aArray[0] = aArray[1] = aArray[2] = aArray[3] = 0;

                                for (sal_uInt32 a(0); bRetval && a < nWriteSize; a++)
                                {
                                    if (osl_File_E_None != osl_writeFile(aHandle, static_cast<const void*>(aArray), 1, &nBaseWritten) || 1 != nBaseWritten)
                                    {
                                        bRetval = false;
                                    }
                                }
                            }

                            if (bRetval)
                            {
                                // write contents - this may adapt PackFileSize for new
                                // files
                                for (auto& candidate : maPackedFileEntryVector)
                                {
                                    if (!candidate.copy_content(aHandle, false))
                                    {
                                        bRetval = false;
                                        break;
                                    }
                                }
                            }

                            if (bRetval)
                            {
                                // seek back to header start (at position 8)
                                if (osl_File_E_None != osl_setFilePos(aHandle, osl_Pos_Absolut, sal_Int64(8)))
                                {
                                    bRetval = false;
                                }
                            }

                            if (bRetval)
                            {
                                // write headers
                                for (auto& candidate : maPackedFileEntryVector)
                                {
                                    if (!candidate.write_header(aHandle))
                                    {
                                        // error
                                        bRetval = false;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }

                // close temp file (in all cases) - it exists until deleted
                osl_closeFile(aHandle);

                if (bRetval)
                {
                    // copy over existing file by first deleting original
                    // and moving the temp file to old original
                    osl::File::remove(maURL);
                    osl::File::move(aTempURL, maURL);
                }

                // delete temp file (in all cases - it may be moved already)
                osl::File::remove(aTempURL);
            }

            return bRetval;
        }

        bool tryPush(FileSharedPtr& rFileCandidate, bool bCompress)
        {
            sal_uInt64 nFileSize(0);

            if (rFileCandidate && osl::File::E_None == rFileCandidate->open(osl_File_OpenFlag_Read))
            {
                rFileCandidate->getSize(nFileSize);
                rFileCandidate->close();
            }

            if (0 == nFileSize)
            {
                // empty file offered
                return false;
            }

            bool bNeedToAdd(false);
            sal_uInt32 nCrc32(0);

            if (maPackedFileEntryVector.empty())
            {
                // no backup yet, add as 1st backup
                bNeedToAdd = true;
            }
            else
            {
                // already backups there, check if different from last entry
                const PackedFileEntry& aLastEntry = maPackedFileEntryVector.back();

                // check if file is different
                if (aLastEntry.getFullFileSize() != static_cast<sal_uInt32>(nFileSize))
                {
                    // different size, different file
                    bNeedToAdd = true;
                }
                else
                {
                    // same size, check crc32
                    nCrc32 = createCrc32(rFileCandidate, 0);

                    if (nCrc32 != aLastEntry.getCrc32())
                    {
                        // different crc, different file
                        bNeedToAdd = true;
                    }
                }
            }

            if (bNeedToAdd)
            {
                // create crc32 if not yet done
                if (0 == nCrc32)
                {
                    nCrc32 = createCrc32(rFileCandidate, 0);
                }

                // create a file entry for a new file. Offset is set automatically
                // to 0 to mark the entry as new file entry
                maPackedFileEntryVector.push_back(
                    PackedFileEntry(
                        static_cast< sal_uInt32 >(nFileSize),
                        nCrc32,
                        rFileCandidate,
                        bCompress));

                mbChanged = true;
            }

            return bNeedToAdd;
        }

        bool tryPop(oslFileHandle& rHandle)
        {
            bool bRetval(false);

            if (!maPackedFileEntryVector.empty())
            {
                // already backups there, check if different from last entry
                PackedFileEntry& aLastEntry = maPackedFileEntryVector.back();

                // here the uncompress flag has to be determined, true
                // means to add the file compressed, false means to add it
                // uncompressed
                bRetval = aLastEntry.copy_content(rHandle, true);

                if (bRetval)
                {
                    maPackedFileEntryVector.pop_back();
                    mbChanged = true;
                }

                return bRetval;
            }

            return false;
        }

        void tryReduceToNumBackups(sal_uInt16 nNumBackups)
        {
            while (maPackedFileEntryVector.size() > nNumBackups)
            {
                maPackedFileEntryVector.pop_front();
                mbChanged = true;
            }
        }

        bool empty()
        {
            return maPackedFileEntryVector.empty();
        }
    };
}

namespace comphelper
{
    sal_uInt16 BackupFileHelper::mnMaxAllowedBackups = 10;
    bool BackupFileHelper::mbExitWasCalled = false;

    BackupFileHelper::BackupFileHelper(
        const OUString& rBaseURL,
        sal_uInt16 nNumBackups)
    :   mrBaseURL(rBaseURL),
        mnNumBackups(::std::min(::std::max(nNumBackups, sal_uInt16(1)), mnMaxAllowedBackups)),
        maBase(),
        maName(),
        maExt()
    {
    }

    void BackupFileHelper::setExitWasCalled()
    {
        mbExitWasCalled = true;
    }

    bool BackupFileHelper::getExitWasCalled()
    {
        return mbExitWasCalled;
    }

    bool BackupFileHelper::getSecureUserConfig(sal_uInt16& rnSecureUserConfigNumCopies)
    {
        // init to not active
        bool bRetval(false);
        rnSecureUserConfigNumCopies = 0;
        OUString sTokenOut;

        if (rtl::Bootstrap::get("SecureUserConfig", sTokenOut))
        {
            bRetval = sTokenOut.toBoolean();
        }

        if (bRetval && rtl::Bootstrap::get("SecureUserConfigNumCopies", sTokenOut))
        {
            rnSecureUserConfigNumCopies = static_cast< sal_uInt16 >(sTokenOut.toUInt32());
        }

        return bRetval;
    }

    bool BackupFileHelper::tryPush(bool bCompress)
    {
        bool bDidPush(false);

        if (splitBaseURL())
        {
            // ensure directory existence
            osl::Directory::createPath(getPackDirName());

            // try push for base file (usually registrymodifications)
            bDidPush = tryPush_basefile(bCompress);

            // Try Push of ExtensionInfo
            bDidPush |= tryPush_extensionInfo(bCompress);
        }

        return bDidPush;
    }

    bool BackupFileHelper::isPopPossible()
    {
        bool bPopPossible(false);

        if (splitBaseURL())
        {
            // try for base file (usually registrymodifications)
            bPopPossible = isPopPossible_basefile();

            // try for ExtensionInfo
            bPopPossible |= isPopPossible_extensionInfo();
        }

        return bPopPossible;
    }

    bool BackupFileHelper::tryPop()
    {
        bool bDidPop(false);

        if (splitBaseURL())
        {
            // try for base file (usually registrymodifications)
            bDidPop = tryPop_basefile();

            // try for ExtensionInfo
            bDidPop |= tryPop_extensionInfo();

            if (bDidPop)
            {
                // try removal of evtl. empty directory
                osl::Directory::remove(getPackDirName());
            }
        }

        return bDidPop;
    }

    bool BackupFileHelper::splitBaseURL()
    {
        if (maBase.isEmpty() && !mrBaseURL.isEmpty())
        {
            // split URL at extension and at last path separator
            maBase = splitAtLastToken(splitAtLastToken(mrBaseURL, '.', maExt), '/', maName);
        }

        return !maBase.isEmpty() && !maName.isEmpty();
    }

    const rtl::OUString BackupFileHelper::getPackDirName() const
    {
        return rtl::OUString(maBase + "/pack");
    }

    const rtl::OUString BackupFileHelper::getPackFileName(const rtl::OUString& rFileName) const
    {
        return rtl::OUString(getPackDirName() + "/" + rFileName + ".pack");
    }

    bool BackupFileHelper::tryPush_basefile(bool bCompress)
    {
        if (fileExists(mrBaseURL))
        {
            PackedFile aPackedFile(getPackFileName(maName));
            FileSharedPtr aBaseFile(new osl::File(mrBaseURL));

            if (aPackedFile.tryPush(aBaseFile, bCompress))
            {
                // reduce to allowed number and flush
                aPackedFile.tryReduceToNumBackups(mnNumBackups);
                aPackedFile.flush();

                return true;
            }
        }

        return false;
    }

    bool BackupFileHelper::tryPush_extensionInfo(bool bCompress)
    {
        ExtensionInfo aExtensionInfo;
        OUString aTempURL;
        bool bRetval(false);

        // create current configuration and write to temp file - it exists until deleted
        if (aExtensionInfo.createTempFile(aTempURL))
        {
            PackedFile aPackedFile(getPackFileName("ExtensionInfo"));
            FileSharedPtr aBaseFile(new osl::File(aTempURL));

            if (aPackedFile.tryPush(aBaseFile, bCompress))
            {
                // reduce to allowed number and flush
                aPackedFile.tryReduceToNumBackups(mnNumBackups);
                aPackedFile.flush();
                bRetval = true;
            }
        }

        // delete temp file (in all cases)
        osl::File::remove(aTempURL);
        return bRetval;
    }

    bool BackupFileHelper::isPopPossible_basefile()
    {
        if (fileExists(mrBaseURL))
        {
            PackedFile aPackedFile(getPackFileName(maName));

            return !aPackedFile.empty();
        }

        return false;
    }

    bool BackupFileHelper::isPopPossible_extensionInfo()
    {
        // extensionInfo always exists internally, no test needed
        PackedFile aPackedFile(getPackFileName("ExtensionInfo"));

        return !aPackedFile.empty();
    }

    bool BackupFileHelper::tryPop_basefile()
    {
        if (fileExists(mrBaseURL))
        {
            // try Pop for base file (usually registrymodifications)
            PackedFile aPackedFile(getPackFileName(maName));

            if (!aPackedFile.empty())
            {
                oslFileHandle aHandle;
                OUString aTempURL;

                // open target temp file - it exists until deleted
                if (osl::File::E_None == osl::FileBase::createTempFile(nullptr, &aHandle, &aTempURL))
                {
                    bool bRetval(aPackedFile.tryPop(aHandle));

                    // close temp file (in all cases) - it exists until deleted
                    osl_closeFile(aHandle);

                    if (bRetval)
                    {
                        // copy over existing file by first deleting original
                        // and moving the temp file to old original
                        osl::File::remove(mrBaseURL);
                        osl::File::move(aTempURL, mrBaseURL);

                        // reduce to allowed number and flush
                        aPackedFile.tryReduceToNumBackups(mnNumBackups);
                        aPackedFile.flush();
                    }

                    // delete temp file (in all cases - it may be moved already)
                    osl::File::remove(aTempURL);

                    return bRetval;
                }
            }
        }

        return false;
    }

    bool BackupFileHelper::tryPop_extensionInfo()
    {
        // extensionInfo always exists internally, no test needed
        PackedFile aPackedFile(getPackFileName("ExtensionInfo"));

        if (!aPackedFile.empty())
        {
            oslFileHandle aHandle;
            OUString aTempURL;

            // open target temp file - it exists until deleted
            if (osl::File::E_None == osl::FileBase::createTempFile(nullptr, &aHandle, &aTempURL))
            {
                bool bRetval(aPackedFile.tryPop(aHandle));

                // close temp file (in all cases) - it exists until deleted
                osl_closeFile(aHandle);

                if (bRetval)
                {
                    // last config is in temp file, load it to ExtensionInfo
                    ExtensionInfo aLoadedExtensionInfo;
                    FileSharedPtr aBaseFile(new osl::File(aTempURL));

                    if (osl::File::E_None == aBaseFile->open(osl_File_OpenFlag_Read))
                    {
                        if (aLoadedExtensionInfo.read_entries(aBaseFile))
                        {
                            ExtensionInfo aCurrentExtensionInfo;

                            aCurrentExtensionInfo.createCurrent();

                            // now we have loaded and current ExtensionInfo and may react on differences





                            bRetval = true;
                        }
                    }

                    // reduce to allowed number and flush
                    aPackedFile.tryReduceToNumBackups(mnNumBackups);
                    aPackedFile.flush();
                }

                // delete temp file (in all cases - it may be moved already)
                osl::File::remove(aTempURL);

                return bRetval;
            }
        }

        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
