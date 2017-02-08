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

#ifndef INCLUDED_TOOLS_RESID_HXX
#define INCLUDED_TOOLS_RESID_HXX

#include <osl/diagnose.h>
#include <rtl/ustring.hxx>
#include <tools/solar.h>
#include <tools/toolsdllapi.h>

struct RSHEADER_TYPE;
typedef sal_uInt32 RESOURCE_TYPE;
#define RSC_NOTYPE              0x100
#define RSC_DONTRELEASE         (sal_uInt32(1U << 31))

class ResMgr;

class SAL_WARN_UNUSED ResId
{
    /*
    Consider two cases:
    either
    (a) m_pResource is valid and points to a resource data buffer;
        then m_nResId and m_pResMgr are not used and may be 0 and nullptr respectively
    or
    (b) m_pResource is NULL, then m_nResId and m_pResMgr must be valid.
        In this case the highest bit, if set, decides whether or not to
        release the Resource context after loading this id.
    */
    RSHEADER_TYPE*          m_pResource;
    mutable sal_uInt32      m_nResId;      // Resource Identifier
    mutable RESOURCE_TYPE   m_nRT;         // type for loading (mutable to be set later)
    mutable ResMgr *        m_pResMgr;     // load from this ResMgr (mutable for setting on demand)

    void ImplInit( sal_uInt32 nId, ResMgr& rMgr, RSHEADER_TYPE* pRes )
    {
        m_pResource = pRes; m_nResId = nId; m_nRT = RSC_NOTYPE; m_pResMgr = &rMgr;
        OSL_ENSURE( m_pResMgr != nullptr, "ResId without ResMgr created" );
    }

public:
    ResId( RSHEADER_TYPE * pRc, ResMgr& rMgr )
    {
        ImplInit( 0, rMgr, pRc );
    }
    ResId( sal_uInt32 nId, ResMgr& rMgr )
    {
        ImplInit( nId, rMgr, nullptr );
    }

    RESOURCE_TYPE   GetRT() const { return m_nRT; }

    /** Set the type if not already set. Ask for type with GetRT()

    [Example]
    ResId aId( 1000 );
    aId.SetRT( RSC_RESOURCE );    // settype window Resource
    //aId.GetRT() == RSC_RESOURCE is true

    @see
    ResId::GetRT2(), ResId::GetRT()
    */
     const ResId &   SetRT( RESOURCE_TYPE nType ) const
     {
         if( RSC_NOTYPE == m_nRT )
             m_nRT = nType;
         return *this;
     }

    ResMgr *        GetResMgr() const { return m_pResMgr; }
    void            ClearResMgr() const { m_pResMgr = nullptr; }

    bool            IsAutoRelease()  const { return !(m_nResId & RSC_DONTRELEASE); }

    sal_uInt32      GetId()          const { return m_nResId & ~RSC_DONTRELEASE; }
    RSHEADER_TYPE*  GetpResource()   const { return m_pResource; }

    TOOLS_DLLPUBLIC OUString toString() const;
    TOOLS_DLLPUBLIC operator OUString() const { return toString(); }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
