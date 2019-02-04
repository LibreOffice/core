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

#include <docufld.hxx>
#include <unofldmid.h>
#include <strings.hrc>
#include <o3tl/any.hxx>
#include <swtypes.hxx>

using namespace ::com::sun::star;

SwScriptFieldType::SwScriptFieldType( SwDoc* pD )
    : SwFieldType( SwFieldIds::Script ), m_pDoc( pD )
{}

SwFieldType* SwScriptFieldType::Copy() const
{
    return new SwScriptFieldType( m_pDoc );
}

SwScriptField::SwScriptField( SwScriptFieldType* pInitType,
                                const OUString& rType, const OUString& rCode,
                                bool bURL )
    : SwField( pInitType ), m_sType( rType ), m_sCode( rCode ), m_bCodeURL( bURL )
{
}

OUString SwScriptField::GetDescription() const
{
    return SwResId(STR_SCRIPT);
}

OUString SwScriptField::ExpandImpl(SwRootFrame const*const) const
{
    return OUString();
}

std::unique_ptr<SwField> SwScriptField::Copy() const
{
    return std::make_unique<SwScriptField>( static_cast<SwScriptFieldType*>(GetTyp()), m_sType, m_sCode, m_bCodeURL );
}

/// set type
void SwScriptField::SetPar1( const OUString& rStr )
{
    m_sType = rStr;
}

OUString SwScriptField::GetPar1() const
{
    return m_sType;
}

/// set code
void SwScriptField::SetPar2( const OUString& rStr )
{
    m_sCode = rStr;
}

OUString SwScriptField::GetPar2() const
{
    return m_sCode;
}

bool SwScriptField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= m_sType;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= m_sCode;
        break;
    case FIELD_PROP_BOOL1:
        rAny <<= m_bCodeURL;
        break;
    default:
        assert(false);
    }
    return true;
}

bool SwScriptField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= m_sType;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= m_sCode;
        break;
    case FIELD_PROP_BOOL1:
        m_bCodeURL = *o3tl::doAccess<bool>(rAny);
        break;
    default:
        assert(false);
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
