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

#include <precomp.h>
#include <toolkit/hf_funcdecl.hxx>


// NOT FULLY DEFINED SERVICES

const String C_sValignTop("top");
const String C_sValignBottom("bottom");



HF_FunctionDeclaration::HF_FunctionDeclaration( Xml::Element & o_rParent,
                                                const String & i_sRaisesText )
    :   HtmlMaker(o_rParent),
        sRaisesText(i_sRaisesText),
        pTable(0),
        pReturnCell(0),
        pNameCell(0),
        pParameterLine(0),
        pLastParameterCell(0),
        pExceptionCell(0)
{
    pTable = new Html::Table;
    CurOut()
        >> *pTable
            << new Html::ClassAttr("table-in-method")
            << new Xml::AnAttribute("border","0");
}

HF_FunctionDeclaration::~HF_FunctionDeclaration()
{
}

Xml::Element &
HF_FunctionDeclaration::ReturnCell()
{
    if (pReturnCell != 0)
        return *pReturnCell;

    pReturnCell = &( *pTable
                        >> *new Html::TableRow
                            >> *new Html::TableCell
                                << new Html::VAlignAttr(C_sValignTop)
                                << new Xml::AnAttribute("colspan", "3")
                    );
    return *pReturnCell;
}

Xml::Element &
HF_FunctionDeclaration::NameCell()
{
    if (pNameCell != 0)
        return *pNameCell;

    pNameCell = &( ParameterLine()
                    >> *new Html::TableCell
                        << new Html::VAlignAttr(C_sValignTop)
                 );
    pLastParameterCell = pNameCell;

    return *pNameCell;
}

Xml::Element &
HF_FunctionDeclaration::NewParamTypeCell()
{
    if (pLastParameterCell != pNameCell)
    {
        pParameterLine = 0;
        ParameterLine()
            >> *new Html::TableCell;
    }

    Xml::Element &
        rParamType = ParameterLine()
                        >> *new Html::TableCell
                            << new Html::VAlignAttr(C_sValignTop);
    pLastParameterCell
                   = &( ParameterLine()
                            >> *new Html::TableCell
                                << new Html::VAlignAttr(C_sValignBottom)
                                << new Xml::XmlCode("&nbsp;")
                      );
    return rParamType;
}

Xml::Element &
HF_FunctionDeclaration::ParamNameCell()
{
    csv_assert(pLastParameterCell != pNameCell);
    return *pLastParameterCell;
}

Xml::Element &
HF_FunctionDeclaration::ExceptionCell()
{
    if (pExceptionCell != 0)
        return *pExceptionCell;

    Xml::Element &
        rExceptionRow = *pTable
                            >> *new Html::TableRow;
    rExceptionRow
        >> *new Html::TableCell
            << new Html::VAlignAttr(C_sValignTop)
            << new Xml::AnAttribute("align", "right")
            << sRaisesText
            << "( ";

    pExceptionCell = &( rExceptionRow
                            >> *new Html::TableCell
                                << new Html::VAlignAttr(C_sValignTop)
                                << new Xml::AnAttribute("colspan", "2")
                      );
    return *pExceptionCell;
}

Html::TableRow &
HF_FunctionDeclaration::ParameterLine()
{
    if (pParameterLine != 0)
        return *pParameterLine;

    pParameterLine = new Html::TableRow;
    *pTable
        >> *pParameterLine;

    return *pParameterLine;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
