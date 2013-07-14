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
#include <comcore.hrc>
#include <tools/resid.hxx>

using namespace ::com::sun::star;

SwScriptFieldType::SwScriptFieldType( SwDoc* pD )
    : SwFieldType( RES_SCRIPTFLD ), pDoc( pD )
{}

SwFieldType* SwScriptFieldType::Copy() const
{
    return new SwScriptFieldType( pDoc );
}

SwScriptField::SwScriptField( SwScriptFieldType* pInitType,
                                const OUString& rType, const OUString& rCode,
                                sal_Bool bURL )
    : SwField( pInitType ), sType( rType ), sCode( rCode ), bCodeURL( bURL )
{
}

OUString SwScriptField::GetDescription() const
{
    return SW_RES(STR_SCRIPT);
}

OUString SwScriptField::Expand() const
{
    return aEmptyStr;
}

SwField* SwScriptField::Copy() const
{
    return new SwScriptField( (SwScriptFieldType*)GetTyp(), sType, sCode, bCodeURL );
}

/// set type
void SwScriptField::SetPar1( const OUString& rStr )
{
    sType = rStr;
}

OUString SwScriptField::GetPar1() const
{
    return sType;
}

/// set code
void SwScriptField::SetPar2( const OUString& rStr )
{
    sCode = rStr;
}

OUString SwScriptField::GetPar2() const
{
    return sCode;
}

bool SwScriptField::QueryValue( uno::Any& rAny, sal_uInt16 nWhichId ) const
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny <<= sType;
        break;
    case FIELD_PROP_PAR2:
        rAny <<= sCode;
        break;
    case FIELD_PROP_BOOL1:
        rAny.setValue(&bCodeURL, ::getBooleanCppuType());
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

bool SwScriptField::PutValue( const uno::Any& rAny, sal_uInt16 nWhichId )
{
    switch( nWhichId )
    {
    case FIELD_PROP_PAR1:
        rAny >>= sType;
        break;
    case FIELD_PROP_PAR2:
        rAny >>= sCode;
        break;
    case FIELD_PROP_BOOL1:
        bCodeURL = *(sal_Bool*)rAny.getValue();
        break;
    default:
        OSL_FAIL("illegal property");
    }
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
