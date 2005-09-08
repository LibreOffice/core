/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: BGroup.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 06:43:06 $
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

#ifndef _CONNECTIVITY_ADABAS_GROUP_HXX_
#define _CONNECTIVITY_ADABAS_GROUP_HXX_

#ifndef _CONNECTIVITY_SDBCX_GROUP_HXX_
#include "connectivity/sdbcx/VGroup.hxx"
#endif

namespace connectivity
{
    namespace adabas
    {
        class OAdabasConnection;
        class OAdabasGroup : public sdbcx::OGroup
        {
            OAdabasConnection*    m_pConnection;
        public:
            virtual void refreshUsers();
        public:
            OAdabasGroup( OAdabasConnection* _pConnection);
            OAdabasGroup( OAdabasConnection* _pConnection,const ::rtl::OUString& _Name);
        };
    }
}
#endif // _CONNECTIVITY_ADABAS_GROUP_HXX_

