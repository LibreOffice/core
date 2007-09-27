/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwAppletImpl.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 07:54:37 $
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

#ifndef _SW_APPLET_IMPL_HXX
#define _SW_APPLET_IMPL_HXX

#define SWHTML_OPTTYPE_IGNORE 0
#define SWHTML_OPTTYPE_TAG 1
#define SWHTML_OPTTYPE_PARAM 2
#define SWHTML_OPTTYPE_SIZE 3

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _HTMLKYWD_HXX
#include <svtools/htmlkywd.hxx>
#endif
#ifndef _FRMHTML_HXX //autogen
#include <sfx2/frmhtml.hxx>
#endif
#ifndef _FRMHTMLW_HXX //autogen
#include <sfx2/frmhtmlw.hxx>
#endif
#ifndef _WRKWIN_HXX //autogen
#include <vcl/wrkwin.hxx>
#endif
#include <sot/storage.hxx>
#ifndef _SFXITEMSET_HXX
#include <svtools/itemset.hxx>
#endif

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
