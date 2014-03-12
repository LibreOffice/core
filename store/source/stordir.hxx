/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _STORE_STORDIR_HXX_
#define _STORE_STORDIR_HXX_

#include <sal/types.h>

#include <rtl/textcvt.h>
#include <rtl/string.h>
#include <rtl/ref.hxx>

#include "object.hxx"
#include "storbase.hxx"
#include "storpage.hxx"

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
    virtual bool isKindOf (sal_uInt32 nTypeId) SAL_OVERRIDE;

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
SAL_CALL query (IStoreHandle *pHandle, SAL_UNUSED_PARAMETER OStoreDirectory_Impl*)
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
