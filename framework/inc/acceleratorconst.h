/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: acceleratorconst.h,v $
 * $Revision: 1.4 $
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

#ifndef _FRAMEWORK_ACCELERATORCONST_H_
#define _FRAMEWORK_ACCELERATORCONST_H_

//_______________________________________________
// own includes

#include <general.h>

//_______________________________________________
// interface includes

//_______________________________________________
// other includes

namespace framework{

#define DOCTYPE_ACCELERATORS            DECLARE_ASCII("<!DOCTYPE accel:acceleratorlist PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"accelerator.dtd\">")

#define ATTRIBUTE_TYPE_CDATA            DECLARE_ASCII("CDATA")

// items without any namespace value!
#define ELEMENT_ACCELERATORLIS          DECLARE_ASCII("acceleratorlist")
#define ELEMENT_ITEM                    DECLARE_ASCII("item"           )

#define XMLNS_ACCEL                     DECLARE_ASCII("accel")
#define XMLNS_XLINK                     DECLARE_ASCII("xlink")

#define ATTRIBUTE_URL                   DECLARE_ASCII("href" )
#define ATTRIBUTE_KEYCODE               DECLARE_ASCII("code" )
#define ATTRIBUTE_MOD_SHIFT             DECLARE_ASCII("shift")
#define ATTRIBUTE_MOD_MOD1              DECLARE_ASCII("mod1" )
#define ATTRIBUTE_MOD_MOD2              DECLARE_ASCII("mod2" )

// same items with a name space alias
#define AL_ELEMENT_ACCELERATORLIST      DECLARE_ASCII("accel:acceleratorlist")
#define AL_ELEMENT_ITEM                 DECLARE_ASCII("accel:item"           )

#define AL_XMLNS_ACCEL                  DECLARE_ASCII("xmlns:accel")
#define AL_XMLNS_XLINK                  DECLARE_ASCII("xmlns:xlink")

#define AL_ATTRIBUTE_URL                DECLARE_ASCII("xlink:href" )
#define AL_ATTRIBUTE_KEYCODE            DECLARE_ASCII("accel:code" )
#define AL_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("accel:shift")
#define AL_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("accel:mod1" )
#define AL_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("accel:mod2" )

// same items with full qualified name space
#define NS_ELEMENT_ACCELERATORLIST      DECLARE_ASCII("http://openoffice.org/2001/accel^acceleratorlist")
#define NS_ELEMENT_ITEM                 DECLARE_ASCII("http://openoffice.org/2001/accel^item"           )

#define NS_XMLNS_ACCEL                  DECLARE_ASCII("http://openoffice.org/2001/accel")
#define NS_XMLNS_XLINK                  DECLARE_ASCII("http://www.w3.org/1999/xlink"    )

#define NS_ATTRIBUTE_URL                DECLARE_ASCII("http://www.w3.org/1999/xlink^href"     )
#define NS_ATTRIBUTE_KEYCODE            DECLARE_ASCII("http://openoffice.org/2001/accel^code" )
#define NS_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("http://openoffice.org/2001/accel^shift")
#define NS_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("http://openoffice.org/2001/accel^mod1" )
#define NS_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("http://openoffice.org/2001/accel^mod2" )

// some storage related values (pathes, names of sub storages etcpp).
#define PRESET_DEFAULT_XML              DECLARE_ASCII("default.xml")
#define PRESET_CURRENT_XML              DECLARE_ASCII("current.xml")

} // namespace framework

#endif // _FRAMEWORK_ACCELERATORCONST_H_
