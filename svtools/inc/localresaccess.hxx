/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localresaccess.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-10-24 15:07:20 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SVTOOLS_LOCALRESACCESS_HXX_
#define _SVTOOLS_LOCALRESACCESS_HXX_

#ifndef _TOOLS_RC_HXX
#include <tools/rc.hxx>
#endif
#ifndef _TOOLS_RCID_H
#include <tools/rcid.h>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

//.........................................................................
namespace svt
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
        ResMgr*     m_pManager;

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
}   // namespace svt
//.........................................................................

#endif // _SVTOOLS_LOCALRESACCESS_HXX_

