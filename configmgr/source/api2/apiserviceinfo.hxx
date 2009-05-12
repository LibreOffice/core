/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apiserviceinfo.hxx,v $
 * $Revision: 1.6 $
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

#ifndef CONFIGMGR_API_SERVICEINFO_HXX_
#define CONFIGMGR_API_SERVICEINFO_HXX_

#include "serviceinfohelper.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//-----------------------------------------------------------------------------

 extern sal_Char const * const c_aUserAdministrationServices[];
 extern sal_Char const * const c_aGroupAdministrationServices[];

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
