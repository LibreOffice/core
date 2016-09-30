/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <rtl/ustring.hxx>
#include <rtl/bootstrap.hxx>
#include <comphelper/backupfilehelper.hxx>
#include <rtl/crc.h>
#include <deque>
#include <zlib.h>

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
}

namespace
{
    class PackedFileEntry
    {
    private:
        sal_uInt32          mnFullFileSize; // size in bytes of unpacked original file
        sal_uInt32          mnPackFileSize; // size in bytes in file backup package (smaller if compressed, same if not)
        sal_uInt32          mnOffset;       // offset in File (zero identifies new file)
        sal_uInt32          mnCrc32;        // checksum
        FileSharedPtr       maFile;         // file where to find the data (at offset)
        bool                mbDoCompress;   // flag if this file is scheduled to be compredded when written

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

        sal_uInt32 getCrc32() const
        {
            return mnCrc32;
        }

        bool read_header(
            FileSharedPtr& rFile,
            sal_uInt32 nOffset)
        {
            mnOffset = nOffset;
            maFile = rFile;

            if (maFile)
            {
                sal_uInt8 aArray[4];
                sal_uInt64 nBaseRead(0);

                // read and compute full file size
                if (osl::File::E_None == maFile->read(static_cast<void*>(aArray), 4, nBaseRead) && 4 == nBaseRead)
                {
                    mnFullFileSize = (sal_uInt32(aArray[0]) << 24) + (sal_uInt32(aArray[1]) << 16) + (sal_uInt32(aArray[2]) << 8) + sal_uInt32(aArray[3]);
                }
                else
                {
                    return false;
                }

                // read and compute entry crc32
                if (osl::File::E_None == maFile->read(static_cast<void*>(aArray), 4, nBaseRead) && 4 == nBaseRead)
                {
                    mnCrc32 = (sal_uInt32(aArray[0]) << 24) + (sal_uInt32(aArray[1]) << 16) + (sal_uInt32(aArray[2]) << 8) + sal_uInt32(aArray[3]);
                }
                else
                {
                    return false;
                }

                // read and compute packed size
                if (osl::File::E_None == maFile->read(static_cast<void*>(aArray), 4, nBaseRead) && 4 == nBaseRead)
                {
                    mnPackFileSize = (sal_uInt32(aArray[0]) << 24) + (sal_uInt32(aArray[1]) << 16) + (sal_uInt32(aArray[2]) << 8) + sal_uInt32(aArray[3]);
                }
                else
                {
                    return false;
                }

                return true;
            }

            return false;
        }

        bool write_header(oslFileHandle& rHandle)
        {
            sal_uInt8 aArray[4];
            sal_uInt64 nBaseWritten(0);

            // write full file size
            aArray[0] = sal_uInt8((mnFullFileSize & 0xff000000) >> 24);
            aArray[1] = sal_uInt8((mnFullFileSize & 0x00ff0000) >> 16);
            aArray[2] = sal_uInt8((mnFullFileSize & 0x0000ff00) >> 8);
            aArray[3] = sal_uInt8(mnFullFileSize & 0x000000ff);

            if (osl_File_E_None != osl_writeFile(rHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) || 4 != nBaseWritten)
            {
                return false;
            }

            // write crc32
            aArray[0] = sal_uInt8((mnCrc32 & 0xff000000) >> 24);
            aArray[1] = sal_uInt8((mnCrc32 & 0x00ff0000) >> 16);
            aArray[2] = sal_uInt8((mnCrc32 & 0x0000ff00) >> 8);
            aArray[3] = sal_uInt8(mnCrc32 & 0x000000ff);

            if (osl_File_E_None != osl_writeFile(rHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) || 4 != nBaseWritten)
            {
                return false;
            }

            // write packed file size
            aArray[0] = sal_uInt8((mnPackFileSize & 0xff000000) >> 24);
            aArray[1] = sal_uInt8((mnPackFileSize & 0x00ff0000) >> 16);
            aArray[2] = sal_uInt8((mnPackFileSize & 0x0000ff00) >> 8);
            aArray[3] = sal_uInt8(mnPackFileSize & 0x000000ff);

            if (osl_File_E_None != osl_writeFile(rHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) || 4 != nBaseWritten)
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
                                    // offset in souce file starts with 8 Byte for header + numEntries and
                                    // 12 byte for each entry (size, crc32 and PackedSize)
                                    sal_uInt32 nOffset(8 + (12 * nEntries));

                                    for (sal_uInt32 a(0); a < nEntries; a++)
                                    {
                                        // create new entry, read header (size, crc and PackedSize),
                                        // set offset and source file
                                        PackedFileEntry aEntry;

                                        if (aEntry.read_header(aSourceFile, nOffset))
                                        {
                                            // add to local data
                                            maPackedFileEntryVector.push_back(aEntry);

                                            // increase offset for next entry
                                            nOffset += aEntry.getPackFileSize();
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

                // open target temp file
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
                        aArray[0] = sal_uInt8((nSize & 0xff000000) >> 24);
                        aArray[1] = sal_uInt8((nSize & 0x00ff0000) >> 16);
                        aArray[2] = sal_uInt8((nSize & 0x0000ff00) >> 8);
                        aArray[3] = sal_uInt8(nSize & 0x000000ff);

                        // write number of entries
                        if (osl_File_E_None == osl_writeFile(aHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) && 4 == nBaseWritten)
                        {
                            if (bRetval)
                            {
                                // write placeholder for headers. Due to the fact that
                                // PackFileSize for newly added files gets set during
                                // writing the content entry, write headers after content
                                // is written. To do so, write placeholders here. We know
                                // the number of entries to write
                                const sal_uInt32 nWriteSize(3 * maPackedFileEntryVector.size());
                                aArray[0] = aArray[1] = aArray[2] = aArray[3] = 0;

                                for (sal_uInt32 a(0); bRetval && a < nWriteSize; a++)
                                {
                                    if (osl_File_E_None != osl_writeFile(aHandle, static_cast<const void*>(aArray), 4, &nBaseWritten) || 4 != nBaseWritten)
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

                // close temp file (in all cases)
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

            if (!maPackedFileEntryVector.empty())
            {
                // already backups there, check if different from last entry
                const PackedFileEntry& aLastEntry = maPackedFileEntryVector.back();

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
            else
            {
                // no backup yet, add
                bNeedToAdd = true;
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

    rtl::OUString BackupFileHelper::getName()
    {
        return OUString(maBase + "/." + maName + ".pack");
    }

    bool BackupFileHelper::tryPush(bool bCompress)
    {
        if (splitBaseURL() && baseFileExists())
        {
            PackedFile aPackedFile(getName());
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

    bool BackupFileHelper::isPopPossible()
    {
        if (splitBaseURL() && baseFileExists())
        {
            PackedFile aPackedFile(getName());

            return !aPackedFile.empty();
        }

        return false;
    }

    bool BackupFileHelper::tryPop()
    {
        if (splitBaseURL() && baseFileExists())
        {
            PackedFile aPackedFile(getName());

            if (!aPackedFile.empty())
            {
                oslFileHandle aHandle;
                OUString aTempURL;

                // open target temp file
                if (osl::File::E_None == osl::FileBase::createTempFile(nullptr, &aHandle, &aTempURL))
                {
                    bool bRetval(aPackedFile.tryPop(aHandle));

                    // close temp file (in all cases)
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

    bool BackupFileHelper::splitBaseURL()
    {
        if (maBase.isEmpty() && !mrBaseURL.isEmpty())
        {
            // split URL at extension and at last path separator
            maBase = splitAtLastToken(splitAtLastToken(mrBaseURL, '.', maExt), '/', maName);
        }

        return !maBase.isEmpty() && !maName.isEmpty();
    }

    bool BackupFileHelper::baseFileExists()
    {
        if (!mrBaseURL.isEmpty())
        {
            FileSharedPtr aBaseFile(new osl::File(mrBaseURL));

            return (osl::File::E_None == aBaseFile->open(osl_File_OpenFlag_Read));
        }

        return false;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
