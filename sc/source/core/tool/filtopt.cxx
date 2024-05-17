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

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <osl/diagnose.h>

#include <filtopt.hxx>
#include <miscuno.hxx>

using namespace utl;
using namespace css::uno;

constexpr OUStringLiteral CFGPATH_FILTER = u"Office.Calc/Filter/Import";

#define SCFILTOPT_WK3           2

ScFilterOptions::ScFilterOptions() :
    ConfigItem( CFGPATH_FILTER ),
    bWK3Flag( false )
{
    Sequence<OUString> aNames { u"MS_Excel/ColScale"_ustr,  // SCFILTOPT_COLSCALE
                                u"MS_Excel/RowScale"_ustr,  // SCFILTOPT_ROWSCALE
                                u"Lotus123/WK3"_ustr };     // SCFILTOPT_WK3
    Sequence<Any> aValues = GetProperties(aNames);
    const Any* pValues = aValues.getConstArray();
    OSL_ENSURE(aValues.getLength() == aNames.getLength(), "GetProperties failed");
    if(aValues.getLength() != aNames.getLength())
        return;

    for(int nProp = 0; nProp < aNames.getLength(); nProp++)
    {
        OSL_ENSURE(pValues[nProp].hasValue(), "property value missing");
        if(pValues[nProp].hasValue())
        {
            switch(nProp)
            {
                case SCFILTOPT_WK3:
                    bWK3Flag = ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] );
                    break;
            }
        }
    }
}

void ScFilterOptions::ImplCommit()
{
    // options are never modified from office

    OSL_FAIL("trying to commit changed ScFilterOptions?");
}

void ScFilterOptions::Notify( const Sequence<OUString>& /* aPropertyNames */ )
{
    OSL_FAIL("properties have been changed");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
