/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <hintids.hxx>
#include <doc.hxx>
#include <docufld.hxx>
#include <unofldmid.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>

using namespace ::com::sun::star;

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


SwMacroField::SwMacroField(SwMacroFieldType* pInitType,
                           const OUString& rLibAndName, const OUString& rTxt) :
    SwField(pInitType), aMacro(rLibAndName), aText(rTxt), bIsScriptURL(sal_False)
{
    bIsScriptURL = isScriptURL(aMacro);
}

OUString SwMacroField::Expand() const
{
    return aText ;
}

SwField* SwMacroField::Copy() const
{
    return new SwMacroField((SwMacroFieldType*)GetTyp(), aMacro, aText);
}

OUString SwMacroField::GetFieldName() const
{
    return GetTyp()->GetName() + " " + aMacro;
}

OUString SwMacroField::GetLibName() const
{
    // if it is a Scripting Framework macro return an empty string
    if (bIsScriptURL)
    {
        return OUString();
    }

    if (!aMacro.isEmpty())
    {
        sal_Int32 nPos = aMacro.getLength();

        for (sal_Int32 i = 0; i < 3 && nPos > 0; i++)
            while (aMacro[--nPos] != '.' && nPos > 0) ;

        return aMacro.copy(0, nPos);
    }

    OSL_FAIL("No LibName");
    return OUString();
}

OUString SwMacroField::GetMacroName() const
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

    OSL_FAIL("No MacroName");
    return OUString();
}

SvxMacro SwMacroField::GetSvxMacro() const
{
    if (bIsScriptURL)
    {
        return SvxMacro(aMacro, OUString(), EXTENDED_STYPE);
    }
    else
    {
        return SvxMacro(GetMacroName(), GetLibName(), STARBASIC);
    }
}

/// LibName and MacroName
void SwMacroField::SetPar1(const OUString& rStr)
{
    aMacro = rStr;
    bIsScriptURL = isScriptURL(aMacro);
}

/// Get macro
OUString SwMacroField::GetPar1() const
{
    return aMacro;
}

/// set macro text
void SwMacroField::SetPar2(const OUString& rStr)
{
    aText = rStr;
}

/// get macro text
OUString SwMacroField::GetPar2() const
{
    return aText;
}

bool SwMacroField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= GetMacroName();
        break;
    case FIELD_PROP_PAR2:
        rAny <<= aText;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= GetLibName();
        break;
    case FIELD_PROP_PAR4:
        rAny <<= bIsScriptURL ? GetMacroName() : OUString();
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwMacroField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    OUString sTmp;
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

/// create an internally used macro name from the library and macro name parts
void SwMacroField::CreateMacroString(
    OUString& rMacro,
    const OUString& rMacroName,
    const OUString& rLibraryName )
{
    // concatenate library and name; use dot only if both strings have content
    rMacro = rLibraryName;
    if ( !rLibraryName.isEmpty() && !rMacroName.isEmpty() )
        rMacro += OUString('.');
    rMacro += rMacroName;
}

sal_Bool SwMacroField::isScriptURL( const OUString& str )
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
