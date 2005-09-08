/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localresaccess.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:54:45 $
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
        }

        ~OLocalResourceAccess()
        {
            m_pManager->Increment(m_pManager->GetRemainSize());
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

