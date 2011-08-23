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
#include <adoc/a_rdocu.hxx>


// NOT FULLY DECLARED SERVICES
#include <adoc/docu_pe.hxx>
#include <adoc/adoc_tok.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <doc_deal.hxx>



namespace adoc
{


DocuExplorer::DocuExplorer()
    :	pDocuDistributor(0),
        pPE(new Adoc_PE),
        bIsPassedFirstDocu(false)
{
}

DocuExplorer::~DocuExplorer()
{
}

void
DocuExplorer::StartNewFile( DocuDealer & o_rDocuDistributor )
{
    pDocuDistributor = &o_rDocuDistributor;
    bIsPassedFirstDocu = false;
}


void
DocuExplorer::Process_Token( DYN adoc::Token & let_drToken )
{
    csv_assert(pDocuDistributor != 0);

    let_drToken.Trigger(*pPE);
    if ( pPE->IsComplete() )
    {
        ary::doc::OldCppDocu *
            pDocu = pPE->ReleaseJustParsedDocu();
        if ( pDocu != 0 )
        {
            if (bIsPassedFirstDocu)
                pDocuDistributor->TakeDocu( *pDocu );
            else
            {
                delete pDocu;
                bIsPassedFirstDocu = true;
            }
        }
    }

    delete &let_drToken;
}


}   // namespace adoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
