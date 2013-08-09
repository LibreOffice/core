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

#include <numithdl.hxx>

using namespace ::com::sun::star;

//
// class XMLNumRulePropHdl
//

XMLNumRulePropHdl::XMLNumRulePropHdl( ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > xNumRuleCompare )
: mxNumRuleCompare( xNumRuleCompare )
{
}

XMLNumRulePropHdl::~XMLNumRulePropHdl()
{
    // Nothing to do
}

bool XMLNumRulePropHdl::equals( const uno::Any& r1, const uno::Any& r2 ) const
{
    return mxNumRuleCompare.is() && mxNumRuleCompare->compare( r1, r2 ) == 0;
}

sal_Bool XMLNumRulePropHdl::importXML( const OUString& /*rStrImpValue*/, ::com::sun::star::uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_False;
}

sal_Bool XMLNumRulePropHdl::exportXML( OUString& /*rStrExpValue*/, const ::com::sun::star::uno::Any& /*rValue*/, const SvXMLUnitConverter& /*rUnitConverter*/ ) const
{
    return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
