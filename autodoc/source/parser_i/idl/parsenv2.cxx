/*************************************************************************
 *
 *  $RCSfile: parsenv2.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mh $ $Date: 2002-08-13 14:49:28 $
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

// [ed] 6/15/02 The OS X compilers require full class definitions at the time
// of template instantiation
#ifdef MACOSX
#include <ary_i/codeinf2.hxx>
#endif

#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/uidl_tok.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary_i/uidl/gate.hxx>
#include <ary_i/codeinf2.hxx>




namespace csi
{
namespace uidl
{


UnoIDL_PE::~UnoIDL_PE()
{
}

void
UnoIDL_PE::EstablishContacts( UnoIDL_PE *               io_pParentPE,
                              ary::Repository &         io_rRepository,
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
                csv_assert(false);
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
                csv_assert(false);
                break;
        default:
            csv_assert(false);
    }   // end switch
}

void
UnoIDL_PE::SetOptional()
{
    if (NOT pDocu)
    {
        Cout() << "OPTIONAL without documentation found (no dysfunction, just info)." << Endl();
        pDocu = new ary::info::CodeInformation;
    }

    pDocu->SetOptional();
}

void
UnoIDL_PE::PassDocuAt( ary::Cei i_nCeId )
{
    if (pDocu)
    {
        ary::CodeEntity2 * pCe = Gate().FindCe(i_nCeId);
        csv_assert(pCe != 0);
        pCe->SetDocu(*pDocu.Release());
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

ary::uidl::CeNamespace &
UnoIDL_PE::CurNamespace() const
{
    if ( Parent() != 0 )
        return Parent()->CurNamespace();
    else
    {
        csv_assert(false);
        return *(ary::uidl::CeNamespace*)0;
    }
}

ary::Cei
UnoIDL_PE::MatchingNamespace( const QuName & i_rQualification )
{
    if (i_rQualification.IsQualified())
        return Gate().CheckInModule(i_rQualification).Id();
    else
        return CurNamespace().Id();
}


}   // namespace uidl
}   // namespace csi

