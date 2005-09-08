/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apiserviceinfo.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 03:08:52 $
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

#ifndef CONFIGMGR_API_SERVICEINFO_HXX_
#define CONFIGMGR_API_SERVICEINFO_HXX_

#ifndef CONFIGMGR_SERVICEINFOHELPER_HXX_
#include "serviceinfohelper.hxx"
#endif

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------

 extern const AsciiServiceName c_aUserAdministrationServices[];
 extern const AsciiServiceName c_aGroupAdministrationServices[];

//-----------------------------------------------------------------------------
 extern ServiceImplementationInfo const aInnerGroupInfoSI;
 extern ServiceImplementationInfo const aInnerGroupUpdateSI;
 extern ServiceImplementationInfo const aInnerSetInfoSI;
 extern ServiceImplementationInfo const aInnerTreeSetSI;
 extern ServiceImplementationInfo const aInnerValueSetSI;
 extern ServiceImplementationInfo const aSetElementGroupInfoSI;
 extern ServiceImplementationInfo const aSetElementGroupUpdateSI;
 extern ServiceImplementationInfo const aSetElementSetInfoSI;
 extern ServiceImplementationInfo const aSetElementTreeSetSI;
 extern ServiceImplementationInfo const aSetElementValueSetSI;
 extern ServiceImplementationInfo const aRootElementGroupInfoSI;
 extern ServiceImplementationInfo const aRootElementGroupUpdateSI;
 extern ServiceImplementationInfo const aRootElementSetInfoSI;
 extern ServiceImplementationInfo const aRootElementTreeSetUpdateSI;
 extern ServiceImplementationInfo const aRootElementValueSetUpdateSI;

//-----------------------------------------------------------------------------
 extern ServiceRegistrationInfo const aCreateReadAccessSI;
 extern ServiceRegistrationInfo const aCreateUpdateAccessSI;

//-----------------------------------------------------------------------------
    }
}
//-----------------------------------------------------------------------------

#endif // CONFIGMGR_API_SERVICEINFO_HXX_
