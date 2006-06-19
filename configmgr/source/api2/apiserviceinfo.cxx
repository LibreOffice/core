/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: apiserviceinfo.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 23:14:47 $
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

const AsciiServiceName c_aNoServices[] =
{
    NULL
};

//-----------------------------------------------------------------------------

const AsciiServiceName c_aCreateAccessServiceNames[] =
{
    "com.sun.star.configuration.ConfigurationAccess",
    "com.sun.star.configuration.HierarchyAccess",
    "com.sun.star.configuration.HierarchyElement",
    NULL
};

const AsciiServiceName c_aCreateUpdateServiceNames[] =
{
    "com.sun.star.configuration.ConfigurationUpdateAccess",
    "com.sun.star.configuration.ConfigurationAccess",
    "com.sun.star.configuration.HierarchyAccess",
    "com.sun.star.configuration.HierarchyElement",
    NULL
};

//-----------------------------------------------------------------------------

const AsciiServiceName c_aUserAdministrationServices[] =
{
    "com.sun.star.configuration.UserAdministration",
    "com.sun.star.configuration.SimpleSetUpdate",
    "com.sun.star.configuration.SimpleSetAccess",
    "com.sun.star.configuration.ConfigurationContainer", // Obsolete
    NULL
};

const AsciiServiceName c_aGroupAdministrationServices[] =
{
    "com.sun.star.configuration.GroupAdministration",
    "com.sun.star.configuration.SimpleSetUpdate",
    "com.sun.star.configuration.SimpleSetAccess",
    "com.sun.star.configuration.ConfigurationContainer", // Obsolete
    NULL
};

//-- ROLE COMBINATIONS ------------------

//-- GroupElements

const AsciiServiceName c_aInnerGroupAccessServices[] =
{
    CFG_SVCLIST_GROUPACCESS,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

const AsciiServiceName c_aInnerGroupUpdateServices[] =
{
    CFG_SVCLIST_GROUPUPDATE,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

const AsciiServiceName c_aInnerSetAccessServices[] =
{
    CFG_SVCLIST_SETACCESS,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

const AsciiServiceName c_aInnerSetUpdateServices[] =
{
    CFG_SVCLIST_SETUPDATE,
    CFG_SVCLIST_GROUPELEMENT,
    NULL
};

//-- SetElements

const AsciiServiceName c_aSetElementGroupAccessServices[] =
{
    CFG_SVCLIST_GROUPACCESS,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

const AsciiServiceName c_aSetElementGroupUpdateServices[] =
{
    CFG_SVCLIST_GROUPUPDATE,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

const AsciiServiceName c_aSetElementSetAccessServices[] =
{
    CFG_SVCLIST_SETACCESS,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

const AsciiServiceName c_aSetElementSetUpdateServices[] =
{
    CFG_SVCLIST_SETUPDATE,
    CFG_SVCLIST_SETELEMENT,
    NULL
};

//-- RootElements

const AsciiServiceName c_aRootGroupAccessServices[] =
{
    CFG_SVCLIST_GROUPACCESS,
    CFG_SVCLIST_ACCESSROOTELEMENT,
    NULL
};

const AsciiServiceName c_aRootGroupUpdateServices[] =
{
    CFG_SVCLIST_GROUPUPDATE,
    CFG_SVCLIST_UPDATEROOTELEMENT,
    NULL
};

const AsciiServiceName c_aRootSetAccessServices[] =
{
    CFG_SVCLIST_SETACCESS,
    CFG_SVCLIST_ACCESSROOTELEMENT,
    NULL
};

const AsciiServiceName c_aRootSetUpdateServices[] =
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
