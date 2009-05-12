/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: apiserviceinfo.cxx,v $
 * $Revision: 1.8 $
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
#include "apiserviceinfo.hxx"

namespace configmgr
{
//-----------------------------------------------------------------------------
    namespace configapi
    {
//========================================================================
//= service names
//========================================================================

//========================================================================
//= service infos
//========================================================================

//-- ACCESS (CONTAINER) ROLES -----------------------------

#define CFG_SVCLIST_SETACCESS \
    "com.sun.star.configuration.ConfigurationAccess",   \
    "com.sun.star.configuration.SetAccess",             \
    "com.sun.star.configuration.HierarchyAccess",       \
    "com.sun.star.configuration.SimpleSetAccess"

#define CFG_SVCLIST_GROUPACCESS \
    "com.sun.star.configuration.ConfigurationAccess",   \
    "com.sun.star.configuration.GroupAccess",           \
    "com.sun.star.configuration.HierarchyAccess",       \
    "com.sun.star.configuration.PropertyHierarchy"

#define CFG_SVCLIST_SETUPDATE \
    "com.sun.star.configuration.ConfigurationUpdateAccess", \
    "com.sun.star.configuration.SetUpdate",                 \
    "com.sun.star.configuration.SimpleSetUpdate",           \
    "com.sun.star.configuration.ConfigurationContainer", /* Obsolete */ \
    CFG_SVCLIST_SETACCESS

#define CFG_SVCLIST_GROUPUPDATE \
    "com.sun.star.configuration.ConfigurationUpdateAccess", \
    "com.sun.star.configuration.GroupUpdate",               \
    CFG_SVCLIST_GROUPACCESS

//-- ELEMENT ROLES ----------------------------------------

#define CFG_SVCLIST_SETELEMENT \
    "com.sun.star.configuration.SetElement",        \
    "com.sun.star.configuration.HierarchyElement"

#define CFG_SVCLIST_GROUPELEMENT \
    "com.sun.star.configuration.GroupElement",      \
    "com.sun.star.configuration.HierarchyElement"

#define CFG_SVCLIST_ACCESSROOTELEMENT \
    "com.sun.star.configuration.AccessRootElement", \
    "com.sun.star.configuration.HierarchyElement"

#define CFG_SVCLIST_UPDATEROOTELEMENT \
    "com.sun.star.configuration.UpdateRootElement", \
    CFG_SVCLIST_ACCESSROOTELEMENT

//-----------------------------------------------------------------------------

sal_Char const * const c_aNoServices[] =
{
    NULL
};

//-----------------------------------------------------------------------------

sal_Char const * const c_aCreateAccessServiceNames[] =
{
    "com.sun.star.configuration.ConfigurationAccess",
    "com.sun.star.configuration.HierarchyAccess",
    "com.sun.star.configuration.HierarchyElement",
    NULL
};

sal_Char const * const c_aCreateUpdateServiceNames[] =
{
    "com.sun.star.configuration.ConfigurationUpdateAccess",
    "com.sun.star.configuration.ConfigurationAccess",
    "com.sun.star.configuration.HierarchyAccess",
    "com.sun.star.configuration.HierarchyElement",
    NULL
};

//-----------------------------------------------------------------------------

sal_Char const * const c_aUserAdministrationServices[] =
{
    "com.sun.star.configuration.UserAdministration",
    "com.sun.star.configuration.SimpleSetUpdate",
    "com.sun.star.configuration.SimpleSetAccess",
    "com.sun.star.configuration.ConfigurationContainer", // Obsolete
    NULL
};

sal_Char const * const c_aGroupAdministrationServices[] =
{
    "com.sun.star.configuration.GroupAdministration",
    "com.sun.star.configuration.SimpleSetUpdate",
    "com.sun.star.configuration.SimpleSetAccess",
    "com.sun.star.configuration.ConfigurationContainer", // Obsolete
    NULL
};

//-- ROLE COMBINATIONS ------------------

//-- GroupElements

sal_Char const * const c_aInnerGroupAccessServices[] =
{
    CFG_SVCLIST_GROUPACCESS,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

sal_Char const * const c_aInnerGroupUpdateServices[] =
{
    CFG_SVCLIST_GROUPUPDATE,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

sal_Char const * const c_aInnerSetAccessServices[] =
{
    CFG_SVCLIST_SETACCESS,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

sal_Char const * const c_aInnerSetUpdateServices[] =
{
    CFG_SVCLIST_SETUPDATE,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

//-- SetElements

sal_Char const * const c_aSetElementGroupAccessServices[] =
{
    CFG_SVCLIST_GROUPACCESS,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

sal_Char const * const c_aSetElementGroupUpdateServices[] =
{
    CFG_SVCLIST_GROUPUPDATE,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

sal_Char const * const c_aSetElementSetAccessServices[] =
{
    CFG_SVCLIST_SETACCESS,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

sal_Char const * const c_aSetElementSetUpdateServices[] =
{
    CFG_SVCLIST_SETUPDATE,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

//-- RootElements

sal_Char const * const c_aRootGroupAccessServices[] =
{
    CFG_SVCLIST_GROUPACCESS,
    CFG_SVCLIST_ACCESSROOTELEMENT,
    NULL
};

sal_Char const * const c_aRootGroupUpdateServices[] =
{
    CFG_SVCLIST_GROUPUPDATE,
    CFG_SVCLIST_UPDATEROOTELEMENT,
    NULL
};

sal_Char const * const c_aRootSetAccessServices[] =
{
    CFG_SVCLIST_SETACCESS,
    CFG_SVCLIST_ACCESSROOTELEMENT,
    NULL
};

sal_Char const * const c_aRootSetUpdateServices[] =
{
    CFG_SVCLIST_SETUPDATE,
    CFG_SVCLIST_UPDATEROOTELEMENT,
    NULL
};
//-----------------------------------------------------------------------------

ServiceImplementationInfo const aInnerGroupInfoSI =
{
    "com.sun.star.comp.configuration.OInnerGroupInfoAccess",
    c_aInnerGroupAccessServices,
    NULL
};
ServiceImplementationInfo const aInnerGroupUpdateSI =
{
    "com.sun.star.comp.configuration.OInnerGroupUpdateAccess",
    c_aInnerGroupUpdateServices,
    NULL
};
ServiceImplementationInfo const aInnerSetInfoSI =
{
    "com.sun.star.comp.configuration.OInnerSetInfoAccess",
    c_aInnerSetAccessServices,
    NULL
};
ServiceImplementationInfo const aInnerTreeSetSI =
{
    "com.sun.star.comp.configuration.OInnerTreeSetUpdateAccess",
    c_aInnerSetUpdateServices,
    NULL
};
ServiceImplementationInfo const aInnerValueSetSI =
{
    "com.sun.star.comp.configuration.OInnerValueSetUpdateAccess",
    c_aInnerSetUpdateServices,
    NULL
};
//-----------------------------------------------------------------------------

ServiceImplementationInfo const aSetElementGroupInfoSI =
{
    "com.sun.star.comp.configuration.OSetElementGroupInfoAccess",
    c_aSetElementGroupUpdateServices,
    NULL
};
ServiceImplementationInfo const aSetElementGroupUpdateSI =
{
    "com.sun.star.comp.configuration.OSetElementGroupUpdateAccess",
    c_aSetElementGroupUpdateServices,
    NULL
};
ServiceImplementationInfo const aSetElementSetInfoSI =
{
    "com.sun.star.comp.configuration.OSetElementSetInfoAccess",
    c_aSetElementSetAccessServices,
    NULL
};
ServiceImplementationInfo const aSetElementTreeSetSI =
{
    "com.sun.star.comp.configuration.OSetElementTreeSetUpdateAccess",
    c_aSetElementSetUpdateServices,
    NULL
};
ServiceImplementationInfo const aSetElementValueSetSI =
{
    "com.sun.star.comp.configuration.OSetElementValueSetUpdateAccess",
    c_aSetElementSetUpdateServices,
    NULL
};
//-----------------------------------------------------------------------------

ServiceImplementationInfo const aRootElementGroupInfoSI =
{
    "com.sun.star.comp.configuration.ORootElementGroupInfoAccess",
    c_aRootGroupAccessServices,
    NULL
};
ServiceImplementationInfo const aRootElementGroupUpdateSI =
{
    "com.sun.star.comp.configuration.ORootElementGroupUpdateAccess",
    c_aRootGroupUpdateServices,
    NULL
};
ServiceImplementationInfo const aRootElementSetInfoSI =
{
    "com.sun.star.comp.configuration.ORootElementSetInfoAccess",
    c_aRootSetAccessServices,
    NULL
};
ServiceImplementationInfo const aRootElementTreeSetUpdateSI =
{
    "com.sun.star.comp.configuration.ORootElementTreeSetUpdateAccess",
    c_aRootSetUpdateServices,
    NULL
};
ServiceImplementationInfo const aRootElementValueSetUpdateSI =
{
    "com.sun.star.comp.configuration.ORootElementValueSetUpdateAccess",
    c_aRootSetUpdateServices,
    NULL
};
//-----------------------------------------------------------------------------

ServiceRegistrationInfo const aCreateReadAccessSI =
{
    NULL, //"com.sun.star.comp.configuration.ORootElementReadAccess",
    c_aCreateAccessServiceNames
};
ServiceRegistrationInfo const aCreateUpdateAccessSI =
{
    NULL, //"com.sun.star.comp.configuration.ORootElementUpdateAccess",
    c_aCreateUpdateServiceNames
};
//-----------------------------------------------------------------------------

    }
}
