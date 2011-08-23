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

#ifndef _SW_APPLET_IMPL_HXX
#define _SW_APPLET_IMPL_HXX

#define SWHTML_OPTTYPE_IGNORE 0
#define SWHTML_OPTTYPE_TAG 1
#define SWHTML_OPTTYPE_PARAM 2
#define SWHTML_OPTTYPE_SIZE 3

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _HTMLKYWD_HXX
#include <bf_svtools/htmlkywd.hxx>
#endif
#ifndef _FRAMEOBJ_HXX //autogen
#include <bf_sfx2/frameobj.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <bf_so3/svstor.hxx>
#endif
#ifndef _APPLET_HXX //autogen
#include <bf_so3/applet.hxx>
#endif
#ifndef _PLUGIN_HXX //autogen
#include <bf_so3/plugin.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <bf_svtools/itemset.hxx>
#endif
namespace binfilter {

class SfxItemSet; 
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_hidden, "HIDDEN" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_HIDDEN_false, "FALSE" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_archive, "ARCHIVE" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_archives, "ARCHIVES" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_object, "OBJECT" );

class SwApplet_Impl
{
    SvAppletObjectRef xApplet;		// das aktuelle Applet
    SfxItemSet 		  aItemSet;

public:
    static USHORT GetOptionType( const String& rName, BOOL bApplet ){DBG_BF_ASSERT(0, "STRIP"); return 0;} //STRIP001 static USHORT GetOptionType( const String& rName, BOOL bApplet );
    SwApplet_Impl( SfxItemSet& rSet ): aItemSet ( rSet) {}
    ~SwApplet_Impl(){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 ~SwApplet_Impl();

    void CreateApplet( const String& rCode, const String& rName,//STRIP001 	void CreateApplet( const String& rCode, const String& rName,
        BOOL bMayScript, const String& rCodeBase ){DBG_BF_ASSERT(0, "STRIP");}; //STRIP001 //STRIP001 					   BOOL bMayScript, const String& rCodeBase );
        sal_Bool CreateApplet();
    SvAppletObject* GetApplet() { return &xApplet; }
    SfxItemSet& GetItemSet() { return aItemSet; }
};
} //namespace binfilter
#endif
