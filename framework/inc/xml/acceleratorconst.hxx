/*************************************************************************
 *
 *  $RCSfile: acceleratorconst.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-09-20 10:04:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _FRAMEWORK_XML_ACCELERATORCONST_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONST_HXX_

//_______________________________________________
// own includes

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_______________________________________________
// interface includes

//_______________________________________________
// other includes

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

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

#define AL_ATTRIBUTE_URL                DECLARE_ASCII("xlink^href" )
#define AL_ATTRIBUTE_KEYCODE            DECLARE_ASCII("accel^code" )
#define AL_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("accel^shift")
#define AL_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("accel^mod1" )
#define AL_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("accel^mod2" )

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

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONST_HXX_
