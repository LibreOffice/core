/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef _STORE_STORE_HXX_
#define _STORE_STORE_HXX_ "$Revision: 1.5 $"

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <store/store.h>

namespace store
{

/*========================================================================
 *
 * OStoreStream interface.
 *
 *======================================================================*/
class OStoreStream
{
public:
    /** Construction.
     */
    inline OStoreStream (void) SAL_THROW(());

    /** Destruction.
     */
    inline ~OStoreStream (void) SAL_THROW(());

    /** Copy construction.
     */
    inline OStoreStream (
        const OStoreStream& rOther) SAL_THROW(());

    /** Assignment.
     */
    inline OStoreStream& operator= (
        const OStoreStream& rOther) SAL_THROW(());


    /** Construction from Stream Handle.
     */
    inline OStoreStream (storeStreamHandle Handle) SAL_THROW(());

    /** Conversion into Stream Handle.
     */
    inline operator storeStreamHandle (void) const SAL_THROW(());

    /** Check for a valid Stream Handle.
        @return sal_True if valid, sal_False otherwise.
     */
    inline sal_Bool isValid (void) const SAL_THROW(());


    /** Open the stream.
        @see store_openStream()
     */
    inline storeError create (
        storeFileHandle      hFile,
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        storeAccessMode      eMode
    ) SAL_THROW(());

    /** Close the stream.
        @see store_closeStream()
     */
    inline void close (void) SAL_THROW(());

    /** Read from the stream.
        @see store_readStream()
     */
    inline storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone
    ) SAL_THROW(());

    /** Write to the stream.
        @see store_writeStream()
     */
    inline storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone
    ) SAL_THROW(());

    /** Flush the stream.
        @see store_flushStream()
     */
    inline storeError flush (void) const SAL_THROW(());

    /** Get the stream size.
        @see store_getStreamSize()
     */
    inline storeError getSize (sal_uInt32 &rnSize) const SAL_THROW(());

    /** Set the stream size.
        @see store_setStreamSize()
     */
    inline storeError setSize (sal_uInt32 nSize) SAL_THROW(());

private:
    /** Representation.
     */
    storeStreamHandle m_hImpl;
};

/*========================================================================
 *
 * OStoreDirectory interface.
 *
 *======================================================================*/
class OStoreDirectory
{
public:
    /** Construction.
     */
    inline OStoreDirectory (void) SAL_THROW(());

    /** Destruction.
     */
    inline ~OStoreDirectory (void) SAL_THROW(());

    /** Copy construction.
     */
    inline OStoreDirectory (
        const OStoreDirectory& rOther) SAL_THROW(());

    /** Assignment.
     */
    inline OStoreDirectory& operator= (
        const OStoreDirectory& rOther) SAL_THROW(());


    /** Construction from Directory Handle.
     */
    inline OStoreDirectory (storeDirectoryHandle Handle) SAL_THROW(());

    /** Conversion into Directory Handle.
     */
    inline operator storeDirectoryHandle (void) const SAL_THROW(());

    /** Check for a valid Directory Handle.
        @return sal_True if valid, sal_False otherwise.
     */
    inline sal_Bool isValid (void) const SAL_THROW(());


    /** Open the directory.
        @see store_openDirectory()
     */
    inline storeError create (
        storeFileHandle      hFile,
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        storeAccessMode      eMode
    ) SAL_THROW(());

    /** Close the directory.
        @see store_closeDirectory()
     */
    inline void close (void) SAL_THROW(());


    /** Directory iterator type.
        @see first()
        @see next()
     */
    typedef storeFindData iterator;

    /** Find first directory entry.
        @see store_findFirst()
     */
    inline storeError first (iterator& it) SAL_THROW(());

    /** Find next directory entry.
        @see store_findNext()
     */
    inline storeError next  (iterator& it) SAL_THROW(());

    /** Directory traversal helper.
        @see travel()
     */
    class traveller
    {
    public:
        /** Directory traversal callback.
            @param  it [in] current directory entry.
            @return sal_True to continue iteration, sal_False to stop.
         */
        virtual sal_Bool visit (const iterator& it) = 0;
    };

    /** Directory traversal.
        @see store_findFirst()
        @see store_findNext()

        @param  rTraveller [in] the traversal callback.
        @return store_E_NoMoreFiles upon end of iteration.
     */
    inline storeError travel (traveller& rTraveller) const;

private:
    /** Representation.
     */
    storeDirectoryHandle m_hImpl;
};

/*========================================================================
 *
 * OStoreFile interface.
 *
 *======================================================================*/
class OStoreFile
{
public:
    /** Construction.
     */
    inline OStoreFile (void) SAL_THROW(());

    /** Destruction.
     */
    inline ~OStoreFile (void) SAL_THROW(());

    /** Copy construction.
     */
    inline OStoreFile (const OStoreFile& rOther) SAL_THROW(());

    /** Assignment.
     */
    inline OStoreFile& operator= (const OStoreFile& rOther) SAL_THROW(());


    /** Construction from File Handle.
     */
    inline OStoreFile (storeFileHandle Handle) SAL_THROW(());

    /** Conversion into File Handle.
     */
    inline operator storeFileHandle (void) const SAL_THROW(());

    /** Check for a valid File Handle.
        @return sal_True if valid, sal_False otherwise.
     */
    inline sal_Bool isValid (void) const SAL_THROW(());


    /** Open the file.
        @see store_openFile()
     */
    inline storeError create (
        const rtl::OUString &rFilename,
        storeAccessMode      eAccessMode,
        sal_uInt16           nPageSize = STORE_DEFAULT_PAGESIZE
    ) SAL_THROW(());

    /** Open the temporary file in memory.
        @see store_createMemoryFile()
     */
    inline storeError createInMemory (
        sal_uInt16 nPageSize = STORE_DEFAULT_PAGESIZE
    ) SAL_THROW(());

    /** Close the file.
        @see store_closeFile()
     */
    inline void close (void) SAL_THROW(());

    /** Flush the file.
        @see store_flushFile()
     */
    inline storeError flush (void) const SAL_THROW(());

    /** Get the number of referers to the file.
        @see store_getFileRefererCount()
     */
    inline storeError getRefererCount (
        sal_uInt32 &rnRefCount) const SAL_THROW(());

    /** Get the file size.
        @see store_getFileSize()
     */
    inline storeError getSize (
        sal_uInt32 &rnSize) const SAL_THROW(());


    /** Set attributes of a file entry.
        @see store_attrib()
     */
    inline storeError attrib (
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        sal_uInt32           nMask1,
        sal_uInt32           nMask2,
        sal_uInt32          &rnAttrib
    ) SAL_THROW(());

    /** Set attributes of a file entry.
        @see store_attrib()
     */
    inline storeError attrib (
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        sal_uInt32           nMask1,
        sal_uInt32           nMask2
    ) SAL_THROW(());

    /** Insert a file entry as 'hard link' to another file entry.
        @see store_link()
     */
    inline storeError link (
        const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
        const rtl::OUString &rDstPath, const rtl::OUString &rDstName
    ) SAL_THROW(());

    /** Insert a file entry as 'symbolic link' to another file entry.
        @see store_symlink()
     */
    inline storeError symlink (
        const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
        const rtl::OUString &rDstPath, const rtl::OUString &rDstName
    ) SAL_THROW(());

    /** Rename a file entry.
        @see store_rename()
     */
    inline storeError rename (
        const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
        const rtl::OUString &rDstPath, const rtl::OUString &rDstName
    ) SAL_THROW(());

    /** Remove a file entry.
        @see store_remove()
     */
    inline storeError remove (
        const rtl::OUString &rPath,
        const rtl::OUString &rName
    ) SAL_THROW(());

private:
    /** Representation.
     */
    storeFileHandle m_hImpl;
};

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

#include <store/store.inl>

} // namespace store

#endif /* !_STORE_STORE_HXX_ */




