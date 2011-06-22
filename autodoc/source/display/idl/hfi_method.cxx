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
#include "hfi_method.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_exception.hxx>
#include <ary/idl/i_param.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_funcdecl.hxx>
#include "hfi_doc.hxx"
#include "hfi_globalindex.hxx"
#include "hfi_typetext.hxx"





HF_IdlMethod::HF_IdlMethod( Environment &           io_rEnv,
                            Xml::Element &          o_cell)
    :   HtmlFactory_Idl(io_rEnv,&o_cell)
{
}


HF_IdlMethod::~HF_IdlMethod()
{
}


void
HF_IdlMethod::Produce_byData( const String &      i_sName,
                              type_id             i_nReturnType,
                              param_list &        i_rParams,
                              type_list &         i_rExceptions,
                              bool                i_bOneway,
                              bool                i_bEllipse,
                              const client &      i_ce ) const
{
    CurOut()
        >> *new Html::Label(i_sName)
            << new Html::ClassAttr(C_sMemberTitle)
            << i_sName;
    enter_ContentCell();
    write_Declaration( i_sName,
                       i_nReturnType,
                       i_rParams,
                       i_rExceptions,
                       i_bOneway,
                       i_bEllipse );
    CurOut() << new Html::HorizontalLine;
    write_Docu(CurOut(), i_ce);
    leave_ContentCell();
}

void
HF_IdlMethod::write_Declaration( const String &      i_sName,
                                 type_id             i_nReturnType,
                                 param_list &        i_rParams,
                                 type_list &         i_rExceptions,
                                 bool                i_bOneway,
                                 bool                i_bEllipse ) const
{
    HF_FunctionDeclaration
        aDecl(CurOut(), "raises") ;
    Xml::Element &
        rReturnLine = aDecl.ReturnCell();

    // Return line:
    if (i_bOneway)
        rReturnLine << "[oneway] ";
    if (i_nReturnType.IsValid())
    {   // Normal function, but not constructors:
        HF_IdlTypeText
            aReturn(Env(), rReturnLine, true);
        aReturn.Produce_byData(i_nReturnType);
    }

    //  Main line:
    Xml::Element &
        rNameCell = aDecl.NameCell();
    rNameCell
        >> *new Html::Bold
            << i_sName;

    Xml::Element *
        pParamEnd  = 0;

    bool bParams = i_rParams.operator bool();
    if (bParams)
    {
        rNameCell
            << "(";

        pParamEnd  = write_Param( aDecl, *i_rParams );
        for (++i_rParams; i_rParams; ++i_rParams)
        {
            *pParamEnd << ",";
            pParamEnd  = write_Param( aDecl, *i_rParams );
        }   // end for

        if (i_bEllipse)
        {
            Xml::Element &
                rParamType = aDecl.NewParamTypeCell();
            rParamType
                << " ...";
            pParamEnd = &rParamType;
        }
        *pParamEnd
            << " )";
    }
    else
    {
        rNameCell
            << "()";
    }

    if ( i_rExceptions.operator bool() )
    {
        Xml::Element &
            rExcOut = aDecl.ExceptionCell();
        HF_IdlTypeText
            aExc(Env(), rExcOut, true);
        aExc.Produce_byData(*i_rExceptions);

        for (++i_rExceptions; i_rExceptions; ++i_rExceptions)
        {
            rExcOut
                << ","
                << new Html::LineBreak;
            aExc.Produce_byData(*i_rExceptions);
        }   // end for

        rExcOut << " );";
    }
    else if (bParams)
    {
        *pParamEnd << ";";
    }
    else
    {
        rNameCell << ";";
    }
}

Xml::Element *
HF_IdlMethod::write_Param( HF_FunctionDeclaration &     o_decl,
                           const ary::idl::Parameter &  i_param ) const
{
    Xml::Element &
        rTypeCell  = o_decl.NewParamTypeCell();
    Xml::Element &
        rNameCell  = o_decl.ParamNameCell();

    switch ( i_param.Direction() )
    {
        case ary::idl::param_in:
                    rTypeCell << "[in] ";
                    break;
        case ary::idl::param_out:
                    rTypeCell << "[out] ";
                    break;
        case ary::idl::param_inout:
                    rTypeCell << "[inout] ";
                    break;
    }   // end switch

    HF_IdlTypeText
        aTypeWriter(Env(), rTypeCell, true);
    aTypeWriter.Produce_byData( i_param.Type() );

    rNameCell
        << i_param.Name();
    return &rNameCell;
}

const String sContentBorder("0");
const String sContentWidth("96%");
const String sContentPadding("5");
const String sContentSpacing("0");

const String sBgWhite("#ffffff");
const String sCenter("center");

void
HF_IdlMethod::enter_ContentCell() const
{

    Xml::Element &
        rContentCell = CurOut()
                        >> *new Html::Table( sContentBorder,
                                             sContentWidth,
                                             sContentPadding,
                                             sContentSpacing )
                            << new Html::ClassAttr("table-in-method")
                            << new Html::BgColorAttr(sBgWhite)
                            << new Html::AlignAttr(sCenter)
                            >> *new Html::TableRow
                                >> *new Html::TableCell;
    Out().Enter(rContentCell);
}


void
HF_IdlMethod::leave_ContentCell() const
{
    Out().Leave();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
