/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: a_rdocu.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:46:23 $
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
#include <adoc/a_rdocu.hxx>


// NOT FULLY DECLARED SERVICES
#include <adoc/docu_pe.hxx>
#include <adoc/adoc_tok.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <doc_deal.hxx>



namespace adoc
{


DocuExplorer::DocuExplorer()
    :   pDocuDistributor(0),
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

