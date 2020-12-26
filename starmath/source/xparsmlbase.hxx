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

#ifndef XPARSEMLBASE
#define XPARSEMLBASE

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/Pair.hpp>
namespace starmathdatabase
{
/**
  * w3 documentation has been used for this.
  * See: https://www.w3.org/2003/entities/2007/htmlmathml-f.ent
  * Copyright 1998 - 2011 W3C.
  * We allow the import of HTML5 entities because are compatible with mathml
  * and ill formated are expected.
  * On export only mathml entities are allowed.
  * Some documentation: https://www.w3.org/TR/MathML3/chapter7.html
  */

constexpr sal_Int32 STARMATH_MATHMLHTML_ENTITY_NUMBER 2125;

/**
  * Entity names for mathml. Example: &infin;
  * These ones are to be used on import.
  * Must be in sync with customMathmlHtmlEntitiesNames.
  */
const extern ::css::uno::Sequence<::css::beans::Pair<::rtl::OUString, ::rtl::OUString>>
    icustomMathmlHtmlEntities;
};

#endif /*XPARSEMLBASE*/
