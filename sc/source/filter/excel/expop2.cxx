/*************************************************************************
 *
 *  $RCSfile: expop2.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 13:25:53 $
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

#ifdef PCH
#include "filt_pch.hxx"
#endif

#pragma hdrstop

//------------------------------------------------------------------------

#include <svtools/fltrcfg.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/docinf.hxx>
#include <svx/svxmsbas.hxx>

#include "scerrors.hxx"
#include "scextopt.hxx"

#include "root.hxx"
#include "exp_op.hxx"
#include "excdoc.hxx"

#include "xcl97esc.hxx"

#include "document.hxx"
#include "rangenam.hxx"
#include "filtopt.hxx"

#ifndef SC_XLTOOLS_HXX
#include "xltools.hxx"
#endif
#ifndef SC_XELINK_HXX
#include "xelink.hxx"
#endif


ExportBiff5::ExportBiff5( XclExpRootData& rExpData ):
    ExportTyp( rExpData.mrBookStrm, &rExpData.mrDoc, rExpData.meCharSet ),
    XclExpRoot( rExpData )
{
    // nur Teil der Root-Daten gebraucht
    pExcRoot = &GetOldRoot();
    pExcRoot->pER = this;   // ExcRoot -> XclExpRoot
    pExcRoot->eDateiTyp = Biff5;
    pExcDoc = new ExcDocument( *this );
}


ExportBiff5::~ExportBiff5()
{
    delete pExcDoc;
}


FltError ExportBiff5::Write()
{
    FltError                eRet = eERR_OK;

    SfxObjectShell* pDocShell = GetDocShell();
    DBG_ASSERT( pDocShell, "ExportBiff5::Write - no document shell" );

    SotStorageRef xRootStrg = GetRootStorage();
    DBG_ASSERT( xRootStrg.Is(), "ExportBiff5::Write - no root storage" );

    bool bWriteBasicCode = false;
    bool bWriteBasicStrg = false;
    if( GetBiff() == EXC_BIFF8 )
    {
        if( SvtFilterOptions* pFilterOpt = SvtFilterOptions::Get() )
        {
            bWriteBasicCode = pFilterOpt->IsLoadExcelBasicCode();
            bWriteBasicStrg = pFilterOpt->IsLoadExcelBasicStorage();
        }
    }

    if( pDocShell && xRootStrg.Is() && bWriteBasicStrg )
    {
        SvxImportMSVBasic aBasicImport( *pDocShell, *xRootStrg, bWriteBasicCode, bWriteBasicStrg );
        ULONG nErr = aBasicImport.SaveOrDelMSVBAStorage( TRUE, EXC_STORAGE_VBA_PROJECT );
        if( nErr != ERRCODE_NONE )
            pDocShell->SetError( nErr );
    }

    pExcDoc->ReadDoc();         // ScDoc -> ExcDoc
    pExcDoc->Write( aOut );     // wechstreamen

    if( pDocShell && xRootStrg.Is() )
    {
        SfxDocumentInfo& rInfo = pDocShell->GetDocInfo();
        rInfo.SavePropertySet( xRootStrg );
    }

    //! TODO: separate warnings for columns and sheets
    const XclExpAddressConverter& rAddrConv = GetAddressConverter();
    if( rAddrConv.IsColTruncated() || rAddrConv.IsRowTruncated() || rAddrConv.IsTabTruncated() )
        return SCWARN_EXPORT_MAXROW;

    return eERR_OK;
}



ExportBiff8::ExportBiff8( XclExpRootData& rExpData ) :
    ExportBiff5( rExpData )
{
    pExcRoot->eDateiTyp = Biff8;
    pExcRoot->pEscher = new XclEscher( GetDoc().GetTableCount(), *pExcRoot );
}


ExportBiff8::~ExportBiff8()
{
    delete pExcRoot->pEscher;
    pExcRoot->pEscher = NULL;
}


