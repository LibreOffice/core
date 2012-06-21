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

package org.openoffice.xmerge.converter.xml.sxc.pexcel.records.formula;

public interface ParseToken
{
        public boolean isOperand();
        public boolean isOperator();
        public int getTokenType();

        //GENERIC TOKENS (MOSTLY UNUSED
    public static final int TOKEN_OPERATOR          = 1;
    public static final int TOKEN_OPERAND           = 2;
    public static final int TOKEN_FUNCTION_FIXED    = 3;
    public static final int TOKEN_FUNCTION_VARIABLE = 4;

}
