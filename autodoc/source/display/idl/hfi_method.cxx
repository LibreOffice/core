/*************************************************************************
 *
 *  $RCSfile: hfi_method.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:26:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
        aDecl(CurOut()) ;
    Xml::Element &
        front = aDecl.Add_ReturnLine();

    // Front:
    if (i_bOneway)
        front << "[oneway] ";
    if (i_nReturnType.IsValid())
    {   // Normal function, but not constructors:
        HF_IdlTypeText
            aReturn(Env(), front,true);
        aReturn.Produce_byData(i_nReturnType);
        front
            << new Html::LineBreak;

    }
    front
        >> *new Html::Bold
            << i_sName;

    //  Main line:
    Xml::Element &
        types = aDecl.Types();
    Xml::Element &
        names = aDecl.Names();
    bool bParams = i_rParams.operator bool();
    if (bParams)
    {
        front
            << "(";
        HF_IdlTypeText
            aType( Env(), types, true );

        write_Param( aType, names, (*i_rParams) );

        for (++i_rParams; i_rParams; ++i_rParams)
        {
            types
                << new Html::LineBreak;
            names
                << ","
                << new Html::LineBreak;
            write_Param( aType, names, (*i_rParams) );
        }   // end for

        if (i_bEllipse)
        {
            names
                << " ...";
        }
        names
            << " )";
    }
    else
        front
            << "()";


    if ( i_rExceptions.operator bool() )
    {
        Xml::Element &
            rExcOut = aDecl.Add_RaisesLine("raises", NOT bParams);
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
    else
    {
        if (bParams)
            aDecl.Names() << ";";
        else
            aDecl.Front() << ";";
    }
}

void
HF_IdlMethod::write_Param( HF_IdlTypeText &             o_type,
                           Xml::Element &               o_names,
                           const ary::idl::Parameter &  i_param ) const
{
    switch ( i_param.Direction() )
    {
        case ary::idl::param_in:
                    o_type.CurOut() << "[in] ";
                    break;
        case ary::idl::param_out:
                    o_type.CurOut() << "[out] ";
                    break;
        case ary::idl::param_inout:
                    o_type.CurOut() << "[inout] ";
                    break;
    }   // end switch

    o_type.Produce_byData( i_param.Type() );
    o_names
        << i_param.Name();
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

