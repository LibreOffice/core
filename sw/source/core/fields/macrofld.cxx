/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: macrofld.cxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 12:20:08 $
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



#ifndef _HINTIDS_HXX
#include <hintids.hxx>
#endif

#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _DOCUFLD_HXX
#include <docufld.hxx>
#endif
#ifndef _UNOFLDMID_H
#include <unofldmid.h>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XURIREFERENCEFACTORY_HPP_
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_URI_XVNDSUNSTARSCRIPTURL_HPP_
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif

using namespace ::com::sun::star;
using ::rtl::OUString;
/*--------------------------------------------------------------------
    Beschreibung: MacroFeldtypen
 --------------------------------------------------------------------*/

SwMacroFieldType::SwMacroFieldType(SwDoc* pDocument)
    : SwFieldType( RES_MACROFLD ),
      pDoc(pDocument)
{
}

SwFieldType* SwMacroFieldType::Copy() const
{
    SwMacroFieldType* pType = new SwMacroFieldType(pDoc);
    return pType;
}

/*--------------------------------------------------------------------
    Beschreibung: Das Macrofeld selbst
 --------------------------------------------------------------------*/

SwMacroField::SwMacroField(SwMacroFieldType* pInitType,
                           const String& rLibAndName, const String& rTxt) :
    SwField(pInitType), aMacro(rLibAndName), aText(rTxt), bIsScriptURL(FALSE)
{
    bIsScriptURL = isScriptURL(aMacro);
}

String SwMacroField::Expand() const
{   // Button malen anstatt von
    return aText ;
}

SwField* SwMacroField::Copy() const
{
    return new SwMacroField((SwMacroFieldType*)GetTyp(), aMacro, aText);
}

String SwMacroField::GetCntnt(BOOL bName) const
{
    if(bName)
    {
        String aStr(GetTyp()->GetName());
        aStr += ' ';
        aStr += aMacro;
        return aStr;
    }
    return Expand();
}

String SwMacroField::GetLibName() const
{
    // if it is a Scripting Framework macro return an empty string
    if (bIsScriptURL)
    {
        return String();
    }

    if (aMacro.Len())
    {
        USHORT nPos = aMacro.Len();

        for (USHORT i = 0; i < 3 && nPos > 0; i++)
            while (aMacro.GetChar(--nPos) != '.' && nPos > 0);

        return aMacro.Copy(0, nPos );
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden")
    return aEmptyStr;
}

String SwMacroField::GetMacroName() const
{
    if (aMacro.Len())
    {
        if (bIsScriptURL)
        {
            return aMacro.Copy( 0 );
        }
        else
        {
            USHORT nPos = aMacro.Len();

            for (USHORT i = 0; i < 3 && nPos > 0; i++)
                while (aMacro.GetChar(--nPos) != '.' && nPos > 0);

            return aMacro.Copy( ++nPos );
        }
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden")
    return aEmptyStr;
}

SvxMacro SwMacroField::GetSvxMacro() const
{
  if (bIsScriptURL)
    {
        return SvxMacro(aMacro, String(), EXTENDED_STYPE);
    }
    else
    {
        return SvxMacro(GetMacroName(), GetLibName(), STARBASIC);
    }
}

/*--------------------------------------------------------------------
    Beschreibung: LibName und MacroName
 --------------------------------------------------------------------*/

void SwMacroField::SetPar1(const String& rStr)
{
    aMacro = rStr;
    bIsScriptURL = isScriptURL(aMacro);
}

const String& SwMacroField::GetPar1() const
{
    return aMacro;
}

/*--------------------------------------------------------------------
    Beschreibung: Macrotext
 --------------------------------------------------------------------*/

void SwMacroField::SetPar2(const String& rStr)
{
    aText = rStr;
}

String SwMacroField::GetPar2() const
{
    return aText;
}

/*-----------------05.03.98 13:38-------------------

--------------------------------------------------*/
BOOL SwMacroField::QueryValue( uno::Any& rAny, USHORT nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= OUString(GetMacroName());
        break;
    case FIELD_PROP_PAR2:
        rAny <<= OUString(aText);
        break;
    case FIELD_PROP_PAR3:
        rAny <<= OUString(GetLibName());
        break;
    case FIELD_PROP_PAR4:
        rAny <<= bIsScriptURL ? OUString(GetMacroName()): OUString();
        break;
    default:
        DBG_ERROR("illegal property");
    }
    return TRUE;
}
/*-----------------05.03.98 13:38-------------------

--------------------------------------------------*/
BOOL SwMacroField::PutValue( const uno::Any& rAny, USHORT nWhichId )
{
    String sTmp;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        CreateMacroString( aMacro, ::GetString(rAny, sTmp), GetLibName());
        break;
    case FIELD_PROP_PAR2:
        ::GetString( rAny, aText );
        break;
    case FIELD_PROP_PAR3:
        CreateMacroString(aMacro, GetMacroName(), ::GetString(rAny, sTmp) );
        break;
    case FIELD_PROP_PAR4:
        ::GetString(rAny, aMacro);
        bIsScriptURL = isScriptURL(aMacro);
        break;
    default:
        DBG_ERROR("illegal property");
    }

    return TRUE;
}

// create an internally used macro name from the library and macro name parts
void SwMacroField::CreateMacroString(
    String& rMacro,
    const String& rMacroName,
    const String& rLibraryName )
{
    // concatenate library and name; use dot only if both strings have content
    rMacro = rLibraryName;
    if ( rLibraryName.Len() > 0 && rMacroName.Len() > 0 )
        rMacro += '.';
    rMacro += rMacroName;
}

BOOL SwMacroField::isScriptURL( const String& str )
{
    uno::Reference< lang::XMultiServiceFactory > xSMgr =
        ::comphelper::getProcessServiceFactory();

    uno::Reference< uri::XUriReferenceFactory >
        xFactory( xSMgr->createInstance(
            OUString::createFromAscii(
                "com.sun.star.uri.UriReferenceFactory" ) ), uno::UNO_QUERY );

    if ( xFactory.is() )
    {
        uno::Reference< uri::XVndSunStarScriptUrl >
            xUrl( xFactory->parse( str ), uno::UNO_QUERY );

        if ( xUrl.is() )
        {
            return TRUE;
        }
    }
    return FALSE;
}
