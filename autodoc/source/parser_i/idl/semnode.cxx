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
    :	pParentPE(0),
        pAryGate(0),
        pTokenResult(0)
{
}

void
SemanticNode::EstablishContacts( UnoIDL_PE *				io_pParentPE,
                                 ary::idl::Gate &           io_rGate,
                                 TokenProcessing_Result & 	o_rResult )
{
    pParentPE 		=  io_pParentPE;
    pAryGate 		= &io_rGate;
    pTokenResult    = &o_rResult;
}

SemanticNode::~SemanticNode()
{
}

void
SemanticNode::SetTokenResult( E_TokenDone		i_eDone,
                              E_EnvStackAction	i_eWhat2DoWithEnvStack,
                              UnoIDL_PE *		i_pParseEnv2Push )
{
    csv_assert(pTokenResult != 0);

    pTokenResult->eDone 		= i_eDone;
    pTokenResult->eStackAction 	= i_eWhat2DoWithEnvStack;
    pTokenResult->pEnv2Push 	= i_pParseEnv2Push;
}


}   // namespace uidl
}   // namespace csi

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
