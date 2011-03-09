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

#ifndef _STORE_STORDIR_HXX_
#define _STORE_STORDIR_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_TEXTCVT_H_
#include <rtl/textcvt.h>
#endif
#ifndef _RTL_STRING_H_
#include <rtl/string.h>
#endif
#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _STORE_OBJECT_HXX_
#include "object.hxx"
#endif
#ifndef _STORE_STORBASE_HXX_
#include "storbase.hxx"
#endif
#ifndef _STORE_STORPAGE_HXX_
#include "storpage.hxx"
#endif

namespace store
{

struct OStoreDirectoryPageData;

/*========================================================================
 *
 * OStoreDirectory_Impl interface.
 *
 *======================================================================*/
class OStoreDirectory_Impl : public store::OStoreObject
{
public:
    /** Construction.
     */
    OStoreDirectory_Impl (void);

    /** create (two-phase construction).
     *  @param  pManager [in]
     *  @param  pPath [in]
     *  @param  pName [in]
     *  @param  eAccessMode [in]
     *  @return store_E_None upon success.
     */
    storeError create (
        OStorePageManager *pManager,
        rtl_String        *pPath,
        rtl_String        *pName,
        storeAccessMode    eAccessMode);

    /** iterate.
     *  @param  rFindData [out]
     *  @return store_E_None        upon success,
     *          store_E_NoMoreFiles upon end of iteration.
     */
    storeError iterate (
        storeFindData &rFindData);

    /** IStoreHandle.
     */
    virtual sal_Bool SAL_CALL isKindOf (sal_uInt32 nTypeId);

protected:
    /** Destruction.
     */
    virtual ~OStoreDirectory_Impl (void);

private:
    /** IStoreHandle TypeId.
     */
    static const sal_uInt32 m_nTypeId;

    /** IStoreHandle query() template function specialization.
     */
    friend OStoreDirectory_Impl*
    SAL_CALL query<> (IStoreHandle *pHandle, OStoreDirectory_Impl*);

    /** Representation.
     */
    typedef OStoreDirectoryPageData   inode;
    typedef PageHolderObject< inode > inode_holder_type;

    rtl::Reference<OStorePageManager> m_xManager;

    OStorePageDescriptor       m_aDescr;
    sal_uInt32                 m_nPath;
    rtl_TextToUnicodeConverter m_hTextCvt;

    /** Not implemented.
     */
    OStoreDirectory_Impl (const OStoreDirectory_Impl&);
    OStoreDirectory_Impl& operator= (const OStoreDirectory_Impl&);
};

template<> inline OStoreDirectory_Impl*
SAL_CALL query (IStoreHandle *pHandle, OStoreDirectory_Impl*)
{
    if (pHandle && pHandle->isKindOf (OStoreDirectory_Impl::m_nTypeId))
    {
        // Handle is kind of OStoreDirectory_Impl.
        return static_cast<OStoreDirectory_Impl*>(pHandle);
    }
    return 0;
}

/*========================================================================
 *
 * The End.
 *
 *======================================================================*/

} // namespace store

#endif /* !_STORE_STORDIR_HXX_ */

