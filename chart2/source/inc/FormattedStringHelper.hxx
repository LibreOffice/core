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
#ifndef INCLUDED_CHART2_SOURCE_INC_FORMATTEDSTRINGHELPER_HXX
#define INCLUDED_CHART2_SOURCE_INC_FORMATTEDSTRINGHELPER_HXX

#include <com/sun/star/chart2/XFormattedString2.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include "charttoolsdllapi.hxx"

namespace chart
{

class OOO_DLLPUBLIC_CHARTTOOLS FormattedStringHelper
{
public:
    static css::uno::Sequence<
            css::uno::Reference< css::chart2::XFormattedString2 > >
            createFormattedStringSequence(
                     const css::uno::Reference< css::uno::XComponentContext > & xContext
                    , const OUString & rString
                    , const css::uno::Reference< css::beans::XPropertySet > & xTextProperties ) throw();
};

} //namespace chart
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
