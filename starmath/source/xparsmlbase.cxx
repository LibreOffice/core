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
#include "xparsmlbase.hxx"

static OUString icustomMathmlHtmlEntitiesNamesData[2] = { u"sigma", u"infin" };

static OUString icustomMathmlHtmlEntitiesValuesData[2] = { u"\u03C3", u"\u221E" };

const ::css::uno::Sequence<::rtl::OUString>
    starmathdatabase::icustomMathmlHtmlEntitiesNames(icustomMathmlHtmlEntitiesNamesData, 2);

const ::css::uno::Sequence<::rtl::OUString>
    starmathdatabase::icustomMathmlHtmlEntitiesValues(icustomMathmlHtmlEntitiesValuesData, 2);

/*
#include "xparsmlbase.hxx"

const com::sun::star::uno::Sequence<OUString> starmathdatabase::icustomMathmlHtmlEntitiesNames
    = { { OUString::createFromAscii("sigma") }, { OUString::createFromAscii("infin") } };

const com::sun::star::uno::Sequence<OUString> starmathdatabase::icustomMathmlHtmlEntitiesValues
    = { { OUString("\u03C3!", 2, RTL_TEXTENCODING_UTF8) },
        { OUString("\u221E!", 2, RTL_TEXTENCODING_UTF8) } };
        */
