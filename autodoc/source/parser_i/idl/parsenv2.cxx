/*************************************************************************
 *
 *  $RCSfile: parsenv2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:40:14 $
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
#include <s2_luidl/parsenv2.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/qualiname.hxx>
#include <ary_i/codeinf2.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_ce.hxx>
#include <ary/idl/ip_ce.hxx>
#include <s2_luidl/uidl_tok.hxx>
#include <x_parse2.hxx>




namespace csi
{
namespace uidl
{


UnoIDL_PE::~UnoIDL_PE()
{
}

void
UnoIDL_PE::EstablishContacts( UnoIDL_PE *               io_pParentPE,
                              ary::n22::Repository &    io_rRepository,
                              TokenProcessing_Result &  o_rResult )
{
    aMyNode.EstablishContacts(io_pParentPE, io_rRepository, o_rResult);
}

void
UnoIDL_PE::Enter( E_EnvStackAction  i_eWayOfEntering )
{
    switch (i_eWayOfEntering)
    {
        case push_sure:
                InitData();
                break;
        case push_try:
                csv_assert(false);
                break;
        case pop_success:
                ReceiveData();
                break;
        case pop_failure:
                throw X_AutodocParser(X_AutodocParser::x_Any);
                break;
        default:
            csv_assert(false);
    }   // end switch
}

void
UnoIDL_PE::Leave( E_EnvStackAction  i_eWayOfLeaving )
{
    switch (i_eWayOfLeaving)
    {
        case push_sure:
                break;
        case push_try:
                csv_assert(false);
                break;
        case pop_success:
                TransferData();
                break;
        case pop_failure:
                throw X_AutodocParser(X_AutodocParser::x_Any);
                break;
        default:
            csv_assert(false);
    }   // end switch
}

void
UnoIDL_PE::SetDocu( DYN ary::info::CodeInformation * let_dpDocu )
{
    pDocu = let_dpDocu;
}

void
UnoIDL_PE::SetPublished()
{
    if (NOT pDocu)
    {
        pDocu = new ary::info::CodeInformation;
    }
    pDocu->SetPublished();
}

void
UnoIDL_PE::SetOptional()
{
    if (NOT pDocu)
    {
        pDocu = new ary::info::CodeInformation;
    }
    pDocu->SetOptional();
}

void
UnoIDL_PE::PassDocuAt( ary::idl::CodeEntity & io_rCe )
{
    if (pDocu)
    {
        io_rCe.Set_Docu(pDocu.Release());
    }
    else if (io_rCe.ClassId() != ary::idl::Module::class_id)
    {
         Cout() << "Warning: "
               << io_rCe.LocalName()
               << " has no documentation."
               << Endl();
    }
}

void
UnoIDL_PE::InitData()
{
    // Needs not anything to do.
}

void
UnoIDL_PE::ReceiveData()
{
    // Needs not anything to do.
}

const ary::idl::Module &
UnoIDL_PE::CurNamespace() const
{
    if ( Parent() != 0 )
        return Parent()->CurNamespace();
    else
    {
        csv_assert(false);
        return *(const ary::idl::Module*)0;
    }
}


}   // namespace uidl
}   // namespace csi

