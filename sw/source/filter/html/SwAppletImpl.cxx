/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SwAppletImpl.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 09:45:17 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#ifndef _SW_APPLET_IMPL_HXX
#include <SwAppletImpl.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif

#include <comphelper/embeddedobjectcontainer.hxx>
#include <sot/clsids.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <svtools/embedhlp.hxx>

using namespace com::sun::star;

/* Some MIB magic...*/

//sal_Char SVT_DLLPUBLIC const SVTOOLS_CONSTASCII_DEF( sHTML_O_archive, "ARCHIVE" );

sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_Hidden, "HIDDEN" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_Hidden_False, "FALSE" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_Archives, "ARCHIVES" );
sal_Char const SVTOOLS_CONSTASCII_DEF( sHTML_O_Object, "OBJECT" );

USHORT SwApplet_Impl::GetOptionType( const String& rName, BOOL bApplet )
{
    USHORT nType = bApplet ? SWHTML_OPTTYPE_PARAM : SWHTML_OPTTYPE_TAG;

    switch( rName.GetChar(0) )
    {
    case 'A':
    case 'a':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_align ) ||
            rName.EqualsIgnoreCaseAscii( sHTML_O_alt ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        else if( bApplet &&
                 (rName.EqualsIgnoreCaseAscii( sHTML_O_archive ) ||
                 rName.EqualsIgnoreCaseAscii( sHTML_O_Archives )) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'C':
    case 'c':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_class ) ||
            (bApplet && (rName.EqualsIgnoreCaseAscii( sHTML_O_code ) ||
                         rName.EqualsIgnoreCaseAscii( sHTML_O_codebase ))) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'H':
    case 'h':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_height ) )
            nType = SWHTML_OPTTYPE_SIZE;
        else if( rName.EqualsIgnoreCaseAscii( sHTML_O_hspace ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_Hidden )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'I':
    case 'i':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_id ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'M':
    case 'm':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_mayscript ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'N':
    case 'n':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_name ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'O':
    case 'o':
        if( bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_Object ) )
            nType = SWHTML_OPTTYPE_TAG;
        break;
    case 'S':
    case 's':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_style ) ||
            (!bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_src )) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'T':
    case 't':
        if( !bApplet && rName.EqualsIgnoreCaseAscii( sHTML_O_type ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'V':
    case 'v':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_vspace ) )
            nType = SWHTML_OPTTYPE_IGNORE;
        break;
    case 'W':
    case 'w':
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_width ) )
            nType = SWHTML_OPTTYPE_SIZE;
        break;
    }

    return nType;
}
SwApplet_Impl::SwApplet_Impl( SfxItemPool& rPool, USHORT nWhich1, USHORT nWhich2 ) :
        aItemSet( rPool, nWhich1, nWhich2 )
{
}

void SwApplet_Impl::CreateApplet( const String& rCode, const String& rName,
                                      BOOL bMayScript, const String& rCodeBase,
                                      const String& rDocumentBaseURL )
{
    comphelper::EmbeddedObjectContainer aCnt;
    ::rtl::OUString aName;

    // create Applet; it will be in running state
    xApplet = aCnt.CreateEmbeddedObject( SvGlobalName( SO3_APPLET_CLASSID ).GetByteSequence(), aName );
    ::svt::EmbeddedObjectRef::TryRunningState( xApplet );

    INetURLObject aUrlBase(rDocumentBaseURL);
    aUrlBase.removeSegment();

    String sDocBase = aUrlBase.GetMainURL(INetURLObject::NO_DECODE);
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCode"), uno::makeAny( ::rtl::OUString( rCode ) ) );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletName"), uno::makeAny( ::rtl::OUString( rName ) ) );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletIsScript"), uno::makeAny( sal_Bool(bMayScript) ) );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletDocBase"), uno::makeAny( ::rtl::OUString(sDocBase) ) );
        if ( rCodeBase.Len() )
            xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCodeBase"), uno::makeAny( ::rtl::OUString( rCodeBase ) ) );
        else
            xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCodeBase"), uno::makeAny( ::rtl::OUString( sDocBase ) ) );
    }
}
#ifdef SOLAR_JAVA
sal_Bool SwApplet_Impl::CreateApplet( const String& rBaseURL )
{
    String aCode, aName, aCodeBase;
    sal_Bool bMayScript = sal_False;

    sal_uInt32 nArgCount = aCommandList.Count();
    for( sal_uInt32 i=0; i<nArgCount; i++ )
    {
        const SvCommand& rArg = aCommandList[i];
        const String& rName = rArg.GetCommand();
        if( rName.EqualsIgnoreCaseAscii( sHTML_O_code ) )
            aCode = rArg.GetArgument();
        else if( rName.EqualsIgnoreCaseAscii( sHTML_O_codebase ) )
            aCodeBase = INetURLObject::GetAbsURL( rBaseURL, rArg.GetArgument() );
        else if( rName.EqualsIgnoreCaseAscii( sHTML_O_name ) )
            aName = rArg.GetArgument();
        else if( rName.EqualsIgnoreCaseAscii( sHTML_O_mayscript ) )
            bMayScript = sal_True;
    }

    if( !aCode.Len() )
        return sal_False;
    CreateApplet( aCode, aName, bMayScript, aCodeBase, rBaseURL );
    return sal_True;
}
#endif

SwApplet_Impl::~SwApplet_Impl()
{
}
void SwApplet_Impl::FinishApplet()
{
    //xApplet->EnableSetModified( TRUE );
    uno::Reference < beans::XPropertySet > xSet( xApplet->getComponent(), uno::UNO_QUERY );
    if ( xSet.is() )
    {
        uno::Sequence < beans::PropertyValue > aProps;
        aCommandList.FillSequence( aProps );
        xSet->setPropertyValue( ::rtl::OUString::createFromAscii("AppletCommands"), uno::makeAny( aProps ) );
    }
}

#ifdef SOLAR_JAVA
void SwApplet_Impl::AppendParam( const String& rName, const String& rValue )
{
    aCommandList.Append( rName, rValue );
}
#endif
