/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#define _SVTOOLS_LOCALRESACCESS_HXX_

#include <tools/rc.hxx>
#include <tools/rcid.h>
#include <osl/diagnose.h>

//.........................................................................
namespace binfilter
{
//.........................................................................

    //=========================================================================
    //= OLocalResourceAccess
    //=========================================================================
    /** helper class for acessing local resources
    */
    class OLocalResourceAccess : public Resource
    {
    protected:
        ResMgr*		m_pManager;

    public:
        OLocalResourceAccess( const ResId& _rId )
            :Resource( _rId.SetAutoRelease( sal_False ) )
            ,m_pManager( _rId.GetResMgr() )
        {
        }

        OLocalResourceAccess(const ResId& _rId, RESOURCE_TYPE _rType)
            :Resource(_rId.SetRT(_rType).SetAutoRelease(sal_False))
            ,m_pManager(_rId.GetResMgr())
        {
            OSL_ENSURE( m_pManager != NULL, "OLocalResourceAccess::OLocalResourceAccess: invalid resource manager!" );
        }

        ~OLocalResourceAccess()
        {
            if ( m_pManager )
                m_pManager->Increment( m_pManager->GetRemainSize() );
            FreeResource();
        }

        inline BOOL IsAvailableRes( const ResId& _rId ) const
        {
            return Resource::IsAvailableRes( _rId );
        }
    };

//.........................................................................
}
//.........................................................................

#endif // _SVTOOLS_LOCALRESACCESS_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
