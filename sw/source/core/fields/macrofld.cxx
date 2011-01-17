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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <hintids.hxx>
#include <doc.hxx>
#include <docufld.hxx>
#include <unofldmid.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>


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
    SwField(pInitType), aMacro(rLibAndName), aText(rTxt), bIsScriptURL(sal_False)
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

String SwMacroField::GetFieldName() const
{
    String aStr(GetTyp()->GetName());
    aStr += ' ';
    aStr += aMacro;
    return aStr;
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
        sal_uInt16 nPos = aMacro.Len();

        for (sal_uInt16 i = 0; i < 3 && nPos > 0; i++)
            while (aMacro.GetChar(--nPos) != '.' && nPos > 0) ;

        return aMacro.Copy(0, nPos );
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden");
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
            sal_uInt16 nPos = aMacro.Len();

            for (sal_uInt16 i = 0; i < 3 && nPos > 0; i++)
                while (aMacro.GetChar(--nPos) != '.' && nPos > 0) ;

            return aMacro.Copy( ++nPos );
        }
    }

    DBG_ASSERT(0, "Kein Macroname vorhanden");
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
sal_Bool SwMacroField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
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
    return sal_True;
}
/*-----------------05.03.98 13:38-------------------

--------------------------------------------------*/
sal_Bool SwMacroField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
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

    return sal_True;
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

sal_Bool SwMacroField::isScriptURL( const String& str )
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
            return sal_True;
        }
    }
    return sal_False;
}
