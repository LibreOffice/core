/*************************************************************************
 *
 *  $RCSfile: store.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mhu $ $Date: 2001-03-13 20:28:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Matthias Huetsch <matthias.huetsch@sun.com>
 *
 *
 ************************************************************************/

#ifndef _STORE_STORE_HXX_
#define _STORE_STORE_HXX_ "$Revision: 1.2 $"

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _STORE_STORE_H_
#include <store/store.h>
#endif

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
    inline OStoreStream (void) SAL_THROW(());
    inline ~OStoreStream (void) SAL_THROW(());

    inline OStoreStream (
        const OStoreStream& rOther) SAL_THROW(());
    inline OStoreStream& operator= (
        const OStoreStream& rOther) SAL_THROW(());

    inline OStoreStream (storeStreamHandle Handle) SAL_THROW(());
    inline operator storeStreamHandle (void) const SAL_THROW(());

    inline sal_Bool isValid (void) const SAL_THROW(());

    inline storeError create (
        storeFileHandle      hFile,
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        storeAccessMode      eMode
    ) SAL_THROW(());

    inline void close (void) SAL_THROW(());

    inline storeError readAt (
        sal_uInt32  nOffset,
        void       *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone
    ) SAL_THROW(());

    inline storeError writeAt (
        sal_uInt32  nOffset,
        const void *pBuffer,
        sal_uInt32  nBytes,
        sal_uInt32 &rnDone
    ) SAL_THROW(());

    inline storeError flush (void) const SAL_THROW(());

    inline storeError getSize (sal_uInt32 &rnSize) const SAL_THROW(());

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
    inline OStoreDirectory (void) SAL_THROW(());
    inline ~OStoreDirectory (void) SAL_THROW(());

    inline OStoreDirectory (
        const OStoreDirectory& rOther) SAL_THROW(());
    inline OStoreDirectory& operator= (
        const OStoreDirectory& rOther) SAL_THROW(());

    inline OStoreDirectory (storeDirectoryHandle Handle) SAL_THROW(());
    inline operator storeDirectoryHandle (void) const SAL_THROW(());

    inline sal_Bool isValid (void) const SAL_THROW(());

    /** create.
     */
    inline storeError create (
        storeFileHandle      hFile,
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        storeAccessMode      eMode
    ) SAL_THROW(());

    /** close.
     */
    inline void close (void) SAL_THROW(());

    /** Iteration.
     */
    typedef storeFindData iterator;

    inline storeError first (iterator& it) SAL_THROW(());
    inline storeError next  (iterator& it) SAL_THROW(());

    /** Traversal.
     */
    class traveller
    {
    public:
        virtual sal_Bool visit (const iterator& it) = 0;
    };

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
    inline OStoreFile (void) SAL_THROW(());
    inline ~OStoreFile (void) SAL_THROW(());

    inline OStoreFile (
        const OStoreFile& rOther) SAL_THROW(());
    inline OStoreFile& operator= (
        const OStoreFile& rOther) SAL_THROW(());

    inline OStoreFile (storeFileHandle Handle) SAL_THROW(());
    inline operator storeFileHandle (void) const SAL_THROW(());

    inline sal_Bool isValid (void) const SAL_THROW(());

    inline storeError create (
        const rtl::OUString &rFilename,
        storeAccessMode      eAccessMode,
        sal_uInt16           nPageSize = STORE_DEFAULT_PAGESIZE
    ) SAL_THROW(());

    inline storeError createInMemory (
        sal_uInt16 nPageSize = STORE_DEFAULT_PAGESIZE
    ) SAL_THROW(());

    inline void close (void) SAL_THROW(());

    inline storeError flush (void) const SAL_THROW(());

    inline storeError getRefererCount (
        sal_uInt32 &rnRefCount) const SAL_THROW(());

    inline storeError getSize (
        sal_uInt32 &rnSize) const SAL_THROW(());

    /** Directory and Stream Manipulation.
     */
    inline storeError attrib (
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        sal_uInt32           nMask1,
        sal_uInt32           nMask2,
        sal_uInt32          &rnAttrib
    ) SAL_THROW(());

    inline storeError attrib (
        const rtl::OUString &rPath,
        const rtl::OUString &rName,
        sal_uInt32           nMask1,
        sal_uInt32           nMask2
    ) SAL_THROW(());

    inline storeError link (
        const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
        const rtl::OUString &rDstPath, const rtl::OUString &rDstName
    ) SAL_THROW(());

    inline storeError symlink (
        const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
        const rtl::OUString &rDstPath, const rtl::OUString &rDstName
    ) SAL_THROW(());

    inline storeError rename (
        const rtl::OUString &rSrcPath, const rtl::OUString &rSrcName,
        const rtl::OUString &rDstPath, const rtl::OUString &rDstName
    ) SAL_THROW(());

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




