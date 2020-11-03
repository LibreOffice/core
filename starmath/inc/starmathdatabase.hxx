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

#include "token.hxx"
#include "types.hxx"

namespace starmathdatabase
{
/**
      * Identifies operator chars tokens for importing mathml.
      * Identifies from char cChar
      * @param cChar
      * @return closing fences' token
      */
SmToken Identify_SmXMLOperatorContext_Impl(sal_Unicode cChar, bool bIsStretchy = true);

/**
      * Identifies opening / closing brace tokens for importing mathml.
      * Identifies from char cChar
      * @param cChar
      * @return closing fences' token
      */
SmToken Identify_PrefixPostfix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
      * Identifies opening brace tokens for importing mathml.
      * Identifies from char cChar
      * @param cChar
      * @return closing fences' token
      */
SmToken Identify_Prefix_SmXMLOperatorContext_Impl(sal_Unicode cChar);

/**
      * Identifies closing brace tokens for importing mathml.
      * Identifies from char cChar
      * @param cChar
      * @return closing fences' token
      */
SmToken Identify_Postfix_SmXMLOperatorContext_Impl(sal_Unicode cChar);
}
