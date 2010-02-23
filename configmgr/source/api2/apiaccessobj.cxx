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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_configmgr.hxx"
#include <stdio.h>
#include "apiaccessobj.hxx"

#include "apiserviceinfo.hxx"
#include "confsvccomponent.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//========================================================================
//= service infos
//========================================================================
/*
const AsciiServiceName c_aUserContainerServices[] =
{
    "com.sun.star.configuration.UserAdministration",
    "com.sun.star.configuration.ConfigurationContainer",
    "com.sun.star.configuration.ConfigurationUpdateAccess",
    "com.sun.star.configuration.ConfigurationAccess",
    NULL
};
const AsciiServiceName c_aContainerServices[] =
{
    "com.sun.star.configuration.ConfigurationContainer",
    "com.sun.star.configuration.ConfigurationUpdateAccess",
    "com.sun.star.configuration.ConfigurationAccess",
    NULL
};
const AsciiServiceName c_aUpdateServices[] =
{
    "com.sun.star.configuration.ConfigurationUpdateAccess",
    "com.sun.star.configuration.ConfigurationAccess",
    NULL
};

const AsciiServiceName c_aAccessServices[] =
{
    "com.sun.star.configuration.ConfigurationAccess",
    NULL
};

const AsciiServiceName c_aNoServices[] =
{
    NULL
};
//-----------------------------------------------------------------------------

ServiceInfo const aInnerGroupInfoSI =
{
    "com.sun.star.configuration.configmgr.OInnerGroupInfoAccess",
    c_aNoServices
};
ServiceInfo const aInnerGroupUpdateSI =
{
    "com.sun.star.configuration.configmgr.OInnerGroupUpdateAccess",
    c_aNoServices
};
ServiceInfo const aInnerSetInfoSI =
{
    "com.sun.star.configuration.configmgr.OInnerSetInfoAccess",
    c_aNoServices
};
ServiceInfo const aInnerTreeSetSI =
{
    "com.sun.star.configuration.configmgr.OInnerTreeSetUpdateAccess",
    c_aNoServices
};
ServiceInfo const aInnerValueSetSI =
{
    "com.sun.star.configuration.configmgr.OInnerValueSetUpdateAccess",
    c_aNoServices
};
//-----------------------------------------------------------------------------

ServiceInfo const aSetElementGroupInfoSI =
{
    "com.sun.star.configuration.configmgr.OSetElementGroupInfoAccess",
    c_aAccessServices
};
ServiceInfo const aSetElementGroupUpdateSI =
{
    "com.sun.star.configuration.configmgr.OSetElementGroupUpdateAccess",
    c_aUpdateServices
};
ServiceInfo const aSetElementSetInfoSI =
{
    "com.sun.star.configuration.configmgr.OSetElementSetInfoAccess",
    c_aAccessServices
};
ServiceInfo const aSetElementTreeSetSI =
{
    "com.sun.star.configuration.configmgr.OSetElementTreeSetUpdateAccess",
    c_aContainerServices
};
ServiceInfo const aSetElementValueSetSI =
{
    "com.sun.star.configuration.configmgr.OSetElementValueSetUpdateAccess",
    c_aContainerServices
};
//-----------------------------------------------------------------------------

ServiceInfo const aRootElementGroupInfoSI =
{
    "com.sun.star.configuration.configmgr.ORootElementGroupInfoAccess",
    c_aAccessServices
};
ServiceInfo const aRootElementGroupUpdateSI =
{
    "com.sun.star.configuration.configmgr.ORootElementGroupUpdateAccess",
    c_aUpdateServices
};
ServiceInfo const aRootElementSetInfoSI =
{
    "com.sun.star.configuration.configmgr.ORootElementSetInfoAccess",
    c_aAccessServices
};
ServiceInfo const aRootElementTreeSetUpdateSI =
{
    "com.sun.star.configuration.configmgr.ORootElementTreeSetUpdateAccess",
    c_aContainerServices
};
ServiceInfo const aRootElementValueSetUpdateSI =
{
    "com.sun.star.configuration.configmgr.ORootElementValueSetUpdateAccess",
    c_aContainerServices
};
//-----------------------------------------------------------------------------

ServiceInfo const aRootElementReadAccessSI =
{
    "com.sun.star.configuration.configmgr.ORootElementReadAccess",
    c_aAccessServices
};
ServiceInfo const aRootElementUpdateAccessSI =
{
    "com.sun.star.configuration.configmgr.ORootElementUpdateAccess",
    c_aUpdateServices
};
ServiceInfo const aRootElementAdminAccessSI =
{
    "com.sun.star.configuration.configmgr.ORootElementUserAdminAccess",
    c_aUserContainerServices
};*/

//========================================================================
//= service info static members
//========================================================================

//-----------------------------------------------------------------------------
// Inner Elements
//-----------------------------------------------------------------------------

template <>
ServiceImplementationInfo const *
const OInnerElement<NodeGroupInfoAccess>::s_pServiceInfo = &aInnerGroupInfoSI;

template <>
ServiceImplementationInfo const *
const OInnerElement<NodeGroupAccess>::s_pServiceInfo = &aInnerGroupUpdateSI;

template <>
ServiceImplementationInfo const *
const OInnerElement<NodeSetInfoAccess>::s_pServiceInfo = &aInnerSetInfoSI;

template <>
ServiceImplementationInfo const *
const OInnerElement<NodeTreeSetAccess>::s_pServiceInfo = &aInnerTreeSetSI;

template <>
ServiceImplementationInfo const *
const OInnerElement<NodeValueSetAccess>::s_pServiceInfo = &aInnerValueSetSI;


//-----------------------------------------------------------------------------
// Set Elements
//-----------------------------------------------------------------------------

template <>
ServiceImplementationInfo const *
const OSetElement<NodeGroupInfoAccess>::s_pServiceInfo = &aSetElementGroupInfoSI;

template <>
ServiceImplementationInfo const *
const OSetElement<NodeGroupAccess>::s_pServiceInfo = &aSetElementGroupUpdateSI;

template <>
ServiceImplementationInfo const *
const OSetElement<NodeSetInfoAccess>::s_pServiceInfo = &aSetElementSetInfoSI;

template <>
ServiceImplementationInfo const *
const OSetElement<NodeTreeSetAccess>::s_pServiceInfo = &aSetElementTreeSetSI;

template <>
ServiceImplementationInfo const *
const OSetElement<NodeValueSetAccess>::s_pServiceInfo = &aSetElementValueSetSI;

//-----------------------------------------------------------------------------
// Root Elements
//-----------------------------------------------------------------------------

template <>
ServiceImplementationInfo const *
const OReadRootElement<NodeGroupInfoAccess>::s_pServiceInfo = &aRootElementGroupInfoSI;

template <>
ServiceImplementationInfo const *
const OUpdateRootElement<NodeGroupAccess>::s_pServiceInfo = &aRootElementGroupUpdateSI;

template <>
ServiceImplementationInfo const *
const OReadRootElement<NodeSetInfoAccess>::s_pServiceInfo = &aRootElementSetInfoSI;

template <>
ServiceImplementationInfo const *
const OUpdateRootElement<NodeTreeSetAccess>::s_pServiceInfo = &aRootElementTreeSetUpdateSI;

template <>
ServiceImplementationInfo const *
const OUpdateRootElement<NodeValueSetAccess>::s_pServiceInfo = &aRootElementValueSetUpdateSI;



//========================================================================
//= Instantiations
//========================================================================
/*
//-----------------------------------------------------------------------------
// Inner Elements
//-----------------------------------------------------------------------------

template class OInnerElement<NodeGroupInfoAccess>;  // OInnerGroupInfoAccess
template class OInnerElement<NodeGroupAccess>;      // OInnerGroupUpdateAccess
template class OInnerElement<NodeSetInfoAccess>;    // OInnerSetInfoAccess
template class OInnerElement<NodeTreeSetAccess>;    // OInnerTreeSetUpdateAccess
template class OInnerElement<NodeValueSetAccess>;   // OInnerValueSetUpdateAccess

//-----------------------------------------------------------------------------
// Set Elements
//-----------------------------------------------------------------------------
template class OSetElement<NodeGroupInfoAccess>;    // OSetElementGroupInfoAccess
template class OSetElement<NodeGroupAccess>;        // OSetElementGroupUpdateAccess
template class OSetElement<NodeSetInfoAccess>;      // OSetElementSetInfoAccess
template class OSetElement<NodeTreeSetAccess>;      // OSetElementTreeSetUpdateAccess
template class OSetElement<NodeValueSetAccess>;     // OSetElementValueSetUpdateAccess

//-----------------------------------------------------------------------------
// Root Elements
//-----------------------------------------------------------------------------

template class OReadRootElement<NodeGroupInfoAccess>;   // ORootElementGroupInfoAccess
template class OUpdateRootElement<NodeGroupAccess>;     // ORootElementGroupUpdateAccess
template class OReadRootElement<NodeSetInfoAccess>;     // ORootElementSetInfoAccess
template class OUpdateRootElement<NodeTreeSetAccess>;   // ORootElementTreeSetUpdateAccess
template class OUpdateRootElement<NodeValueSetAccess>;  // ORootElementValueSetUpdateAccess
*/
//-----------------------------------------------------------------------------
    }
}
