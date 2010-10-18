/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
