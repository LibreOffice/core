/*************************************************************************
 *
 *  $RCSfile: SwAppletImpl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-19 12:27:01 $
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
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _FRMHTML_HXX //autogen
#include <sfx2/frmhtml.hxx>
#endif
#ifndef _FRAMEOBJ_HXX //autogen
#include <sfx2/frameobj.hxx>
#endif
#ifndef _FRMHTMLW_HXX //autogen
#include <sfx2/frmhtmlw.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#ifndef _SVSTOR_HXX //autogen
#include <so3/svstor.hxx>
#endif
#ifndef _APPLET_HXX //autogen
#include <so3/applet.hxx>
#endif
#ifndef _PLUGIN_HXX //autogen
#include <so3/plugin.hxx>
#endif
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

class SfxItemSet;

extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_hidden, "HIDDEN" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_HIDDEN_false, "FALSE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_archive, "ARCHIVE" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_archives, "ARCHIVES" );
extern sal_Char __FAR_DATA SVTOOLS_CONSTASCII_DECL( sHTML_O_object, "OBJECT" );

class SwApplet_Impl
{
    SvAppletObjectRef xApplet;      // das aktuelle Applet
    SvCommandList     aCommandList; // und die szugehorige Command-List
    SfxItemSet        aItemSet;
    String            sAlt;

public:
    static USHORT GetOptionType( const String& rName, BOOL bApplet );
    SwApplet_Impl( SfxItemPool& rPool, USHORT nWhich1, USHORT nWhich2 );
    SwApplet_Impl( SfxItemSet& rSet ): aItemSet ( rSet) {}
    ~SwApplet_Impl();

    void CreateApplet( const String& rCode, const String& rName,
                       BOOL bMayScript, const String& rCodeBase );
    sal_Bool CreateApplet();
                       //const String& rAlt );
    void FinishApplet();
    void AppendParam( const String& rName, const String& rValue );
    SvAppletObject* GetApplet() { return &xApplet; }
    SfxItemSet& GetItemSet() { return aItemSet; }
    const String& GetAltText() { return sAlt; }
    void          SetAltText( const String& rAlt ) {sAlt = rAlt;}
};
#endif
