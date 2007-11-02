/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: semnode.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:10:35 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <precomp.h>
#include <s2_luidl/semnode.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ary.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/parsenv2.hxx>


namespace csi
{
namespace uidl
{



SemanticNode::SemanticNode()
    :   pParentPE(0),
        pAryGate(0),
        pTokenResult(0)
{
}

void
SemanticNode::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                 ary::idl::Gate &           io_rGate,
                                 TokenProcessing_Result &   o_rResult )
{
    pParentPE       =  io_pParentPE;
    pAryGate        = &io_rGate;
    pTokenResult    = &o_rResult;
}

SemanticNode::~SemanticNode()
{
}

void
SemanticNode::SetTokenResult( E_TokenDone       i_eDone,
                              E_EnvStackAction  i_eWhat2DoWithEnvStack,
                              UnoIDL_PE *       i_pParseEnv2Push )
{
    csv_assert(pTokenResult != 0);

    pTokenResult->eDone         = i_eDone;
    pTokenResult->eStackAction  = i_eWhat2DoWithEnvStack;
    pTokenResult->pEnv2Push     = i_pParseEnv2Push;
}


}   // namespace uidl
}   // namespace csi
