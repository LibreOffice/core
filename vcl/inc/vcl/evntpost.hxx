/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: evntpost.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 17:52:40 $
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
#ifndef _VCL_EVNTPOST_HXX
#define _VCL_EVNTPOST_HXX

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif

#ifndef _VCL_DLLAPI_H
#include <vcl/dllapi.h>
#endif

//===================================================================

namespace vcl
{
    struct UserEvent
    {
        ULONG           m_nWhich;
        void*           m_pData;
    };

    class VCL_DLLPUBLIC EventPoster
    {
        ULONG           m_nId;
        Link            m_aLink;

//#if 0 // _SOLAR__PRIVATE
        DECL_DLLPRIVATE_LINK( DoEvent_Impl, UserEvent* );
//#endif

    public:
                        EventPoster( const Link& rLink );
                        ~EventPoster();
        void            Post( UserEvent* pEvent );
    };
}

#endif
