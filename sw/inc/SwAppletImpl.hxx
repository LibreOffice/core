/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SwAppletImpl.hxx,v $
 * $Revision: 1.10 $
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

#include <com/sun/star/embed/XEmbeddedObject.hpp>


#include <tools/string.hxx>
#include <svtools/htmlkywd.hxx>
#include <sfx2/frmhtml.hxx>
#include <sfx2/frmhtmlw.hxx>
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#include <sot/storage.hxx>
#include <svtools/itemset.hxx>

#include <svtools/ownlist.hxx>

class SfxItemSet;

extern SVT_DLLPUBLIC sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_archive, "ARCHIVE" );

extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_Hidden, "HIDDEN" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_Hidden_False, "FALSE" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_Archives, "ARCHIVES" );
extern sal_Char const SVTOOLS_CONSTASCII_DECL( sHTML_O_Object, "OBJECT" );

class SwApplet_Impl
{
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > xApplet;
    SvCommandList     aCommandList; // und die szugehorige Command-List
    SfxItemSet        aItemSet;
    String            sAlt;

public:
    static USHORT GetOptionType( const String& rName, BOOL bApplet );
    SwApplet_Impl( SfxItemPool& rPool, USHORT nWhich1, USHORT nWhich2 );
    SwApplet_Impl( SfxItemSet& rSet ): aItemSet ( rSet) {}
    ~SwApplet_Impl();
    void CreateApplet( const String& rCode, const String& rName,
                       BOOL bMayScript, const String& rCodeBase,
                       const String& rBaseURL );
#ifdef SOLAR_JAVA
    sal_Bool CreateApplet( const String& rBaseURL );
    void AppendParam( const String& rName, const String& rValue );
#endif
    void FinishApplet();
    com::sun::star::uno::Reference < com::sun::star::embed::XEmbeddedObject > GetApplet() { return xApplet; }
    SfxItemSet& GetItemSet() { return aItemSet; }
    const String& GetAltText() { return sAlt; }
    void          SetAltText( const String& rAlt ) {sAlt = rAlt;}
};
#endif
