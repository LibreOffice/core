/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <hintids.hxx>
#include <doc.hxx>
#include <docufld.hxx>
#include <unofldmid.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
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

    if (!aMacro.isEmpty())
    {
        sal_Int32 nPos = aMacro.getLength();

        for (sal_Int32 i = 0; i < 3 && nPos > 0; i++)
            while (aMacro[--nPos] != '.' && nPos > 0) ;

        return aMacro.copy(0, nPos);
    }

    OSL_FAIL("Kein Macroname vorhanden");
    return aEmptyStr;
}

String SwMacroField::GetMacroName() const
{
    if (!aMacro.isEmpty())
    {
        if (bIsScriptURL)
        {
            return aMacro;
        }
        else
        {
            sal_Int32 nPos = aMacro.getLength();

            for (sal_Int32 i = 0; i < 3 && nPos > 0; i++)
                while (aMacro[--nPos] != '.' && nPos > 0) ;

            return aMacro.copy( ++nPos );
        }
    }

    OSL_FAIL("Kein Macroname vorhanden");
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

void SwMacroField::SetPar1(const rtl::OUString& rStr)
{
    aMacro = rStr;
    bIsScriptURL = isScriptURL(aMacro);
}

const rtl::OUString& SwMacroField::GetPar1() const
{
    return aMacro;
}

/*--------------------------------------------------------------------
    Beschreibung: Macrotext
 --------------------------------------------------------------------*/

void SwMacroField::SetPar2(const rtl::OUString& rStr)
{
    aText = rStr;
}

rtl::OUString SwMacroField::GetPar2() const
{
    return aText;
}

bool SwMacroField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
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
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwMacroField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    rtl::OUString sTmp;
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= sTmp;
        CreateMacroString( aMacro, sTmp, GetLibName());
        break;
    case FIELD_PROP_PAR2:
        rAny >>= aText;
        break;
    case FIELD_PROP_PAR3:
        rAny >>= sTmp;
        CreateMacroString(aMacro, GetMacroName(), sTmp );
        break;
    case FIELD_PROP_PAR4:
        rAny >>= aMacro;
        bIsScriptURL = isScriptURL(aMacro);
        break;
    default:
        OSL_FAIL("illegal property");
    }

    return true;
}

// create an internally used macro name from the library and macro name parts
void SwMacroField::CreateMacroString(
    rtl::OUString& rMacro,
    const String& rMacroName,
    const String& rLibraryName )
{
    // concatenate library and name; use dot only if both strings have content
    rMacro = rLibraryName;
    if ( rLibraryName.Len() > 0 && rMacroName.Len() > 0 )
        rMacro += rtl::OUString('.');
    rMacro += rMacroName;
}

sal_Bool SwMacroField::isScriptURL( const String& str )
{
    uno::Reference< uno::XComponentContext > xContext =
        ::comphelper::getProcessComponentContext();

    uno::Reference< uri::XUriReferenceFactory >
        xFactory = uri::UriReferenceFactory::create( xContext );

    uno::Reference< uri::XVndSunStarScriptUrl >
        xUrl( xFactory->parse( str ), uno::UNO_QUERY );

    if ( xUrl.is() )
    {
        return sal_True;
    }
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
