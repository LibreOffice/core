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
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XVndSunStarScriptUrl.hpp>
#include <comphelper/processfactory.hxx>
#include <osl/diagnose.h>

using namespace ::com::sun::star;

SwMacroFieldType::SwMacroFieldType(SwDoc* pDocument)
    : SwFieldType( SwFieldIds::Macro ),
      m_pDoc(pDocument)
{
}

SwFieldType* SwMacroFieldType::Copy() const
{
    SwMacroFieldType* pType = new SwMacroFieldType(m_pDoc);
    return pType;
}

SwMacroField::SwMacroField(SwMacroFieldType* pInitType,
                           const OUString& rLibAndName, const OUString& rText) :
    SwField(pInitType), m_aMacro(rLibAndName), m_aText(rText), m_bIsScriptURL(false)
{
    m_bIsScriptURL = isScriptURL(m_aMacro);
}

OUString SwMacroField::ExpandImpl(SwRootFrame const*const) const
{
    return m_aText ;
}

std::unique_ptr<SwField> SwMacroField::Copy() const
{
    return std::make_unique<SwMacroField>(static_cast<SwMacroFieldType*>(GetTyp()), m_aMacro, m_aText);
}

OUString SwMacroField::GetFieldName() const
{
    return GetTyp()->GetName() + " " + m_aMacro;
}

OUString SwMacroField::GetLibName() const
{
    // if it is a Scripting Framework macro return an empty string
    if (m_bIsScriptURL)
    {
        return OUString();
    }

    if (!m_aMacro.isEmpty())
    {
        sal_Int32 nPos = m_aMacro.getLength();

        for (sal_Int32 i = 0; i < 3 && nPos > 0; i++)
            while (m_aMacro[--nPos] != '.' && nPos > 0) ;

        return m_aMacro.copy(0, nPos);
    }

    OSL_FAIL("No LibName");
    return OUString();
}

OUString SwMacroField::GetMacroName() const
{
    if (!m_aMacro.isEmpty())
    {
        if (m_bIsScriptURL)
        {
            return m_aMacro;
        }
        else
        {
            sal_Int32 nPos = m_aMacro.getLength();

            for (sal_Int32 i = 0; i < 3 && nPos > 0; i++)
                while (m_aMacro[--nPos] != '.' && nPos > 0) ;

            return m_aMacro.copy( ++nPos );
        }
    }

    OSL_FAIL("No MacroName");
    return OUString();
}

SvxMacro SwMacroField::GetSvxMacro() const
{
    if (m_bIsScriptURL)
    {
        return SvxMacro(m_aMacro, OUString(), EXTENDED_STYPE);
    }
    else
    {
        return SvxMacro(GetMacroName(), GetLibName(), STARBASIC);
    }
}

/// LibName and MacroName
void SwMacroField::SetPar1(const OUString& rStr)
{
    m_aMacro = rStr;
    m_bIsScriptURL = isScriptURL(m_aMacro);
}

/// Get macro
OUString SwMacroField::GetPar1() const
{
    return m_aMacro;
}

/// set macro text
void SwMacroField::SetPar2(const OUString& rStr)
{
    m_aText = rStr;
}

/// get macro text
OUString SwMacroField::GetPar2() const
{
    return m_aText;
}

bool SwMacroField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= GetMacroName();
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_aText;
        break;
    case FIELD_PROP_PAR3:
        rAny <<= GetLibName();
        break;
    case FIELD_PROP_PAR4:
        rAny <<= m_bIsScriptURL ? GetMacroName() : OUString();
        break;
    default:
        assert(false);
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
        CreateMacroString( m_aMacro, sTmp, GetLibName());
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_aText;
        break;
    case FIELD_PROP_PAR3:
        rAny >>= sTmp;
        CreateMacroString(m_aMacro, GetMacroName(), sTmp );
        break;
    case FIELD_PROP_PAR4:
        rAny >>= m_aMacro;
        m_bIsScriptURL = isScriptURL(m_aMacro);
        break;
    default:
        assert(false);
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
        rMacro += ".";
    rMacro += rMacroName;
}

bool SwMacroField::isScriptURL( const OUString& str )
{
    try
    {
        uno::Reference<uno::XComponentContext> xContext = ::comphelper::getProcessComponentContext();
        uno::Reference<uri::XUriReferenceFactory> xFactory = uri::UriReferenceFactory::create(xContext);
        uno::Reference<uri::XVndSunStarScriptUrl> xUrl(xFactory->parse(str), uno::UNO_QUERY);
        return xUrl.is();
    }
    catch (...)
    {
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
