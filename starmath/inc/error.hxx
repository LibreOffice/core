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
#ifndef INCLUDED_STARMATH_INC_ERROR_HXX
#define INCLUDED_STARMATH_INC_ERROR_HXX

#include <rtl/ustring.hxx>

class SmNode;

enum SmParseError
{
    PE_NONE,                    PE_UNEXPECTED_END_OF_INPUT,
    PE_UNEXPECTED_CHAR,         PE_UNEXPECTED_TOKEN,
    PE_FUNC_EXPECTED,           PE_UNOPER_EXPECTED,
    PE_BINOPER_EXPECTED,        PE_SYMBOL_EXPECTED,
    PE_IDENTIFIER_EXPECTED,     PE_POUND_EXPECTED,
    PE_COLOR_EXPECTED,          PE_LGROUP_EXPECTED,
    PE_RGROUP_EXPECTED,         PE_LBRACE_EXPECTED,
    PE_RBRACE_EXPECTED,         PE_PARENT_MISMATCH,
    PE_RIGHT_EXPECTED,          PE_FONT_EXPECTED,
    PE_SIZE_EXPECTED,           PE_DOUBLE_ALIGN,
    PE_DOUBLE_SUBSUPSCRIPT
};


struct SmErrorDesc
{
    SmParseError m_eType;
    SmNode      *m_pNode;
    OUString     m_aText;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
