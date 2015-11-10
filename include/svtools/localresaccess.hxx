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

#ifndef INCLUDED_SVTOOLS_LOCALRESACCESS_HXX
#define INCLUDED_SVTOOLS_LOCALRESACCESS_HXX

#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <osl/diagnose.h>


namespace svt
{



    //= OLocalResourceAccess

    /** helper class for acessing local resources
    */
    class OLocalResourceAccess : public Resource
    {
    protected:
        ResMgr*     m_pManager;

    public:
        OLocalResourceAccess( const ResId& _rId )
            :Resource( _rId.SetAutoRelease( false ) )
            ,m_pManager( _rId.GetResMgr() )
        {
        }

        OLocalResourceAccess(const ResId& _rId, RESOURCE_TYPE _rType)
            :Resource(_rId.SetRT(_rType).SetAutoRelease(false))
            ,m_pManager(_rId.GetResMgr())
        {
            OSL_ENSURE( m_pManager != nullptr, "OLocalResourceAccess::OLocalResourceAccess: invalid resource manager!" );
        }

        ~OLocalResourceAccess()
        {
            if ( m_pManager )
                m_pManager->Increment( m_pManager->GetRemainSize() );
            FreeResource();
        }

        inline bool IsAvailableRes( const ResId& _rId ) const
        {
            return Resource::IsAvailableRes( _rId );
        }
    };


}   // namespace svt


#endif // INCLUDED_SVTOOLS_LOCALRESACCESS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
