/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: evntpost.hxx,v $
 * $Revision: 1.3 $
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
#ifndef _VCL_EVNTPOST_HXX
#define _VCL_EVNTPOST_HXX

#include <tools/link.hxx>
#include <vcl/dllapi.h>

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
