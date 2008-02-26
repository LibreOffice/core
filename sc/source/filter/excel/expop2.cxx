/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: expop2.cxx,v $
 *
 *  $Revision: 1.32 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 14:52:49 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"



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

#include <com/sun/star/document/XDocumentProperties.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>


ExportBiff5::ExportBiff5( XclExpRootData& rExpData, SvStream& rStrm ):
    ExportTyp( rStrm, &rExpData.mrDoc, rExpData.meTextEnc ),
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
        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
                = xDPS->getDocumentProperties();
        ::boost::shared_ptr<GDIMetaFile> pMetaFile =
            pDocShell->GetPreviewMetaFile (sal_False);
        uno::Sequence<sal_uInt8> metaFile(
            sfx2::convertMetaFile(pMetaFile.get()));
        sfx2::SaveOlePropertySet(xDocProps, xRootStrg, &metaFile);
    }

    //! TODO: separate warnings for columns and sheets
    const XclExpAddressConverter& rAddrConv = GetAddressConverter();
    if( rAddrConv.IsColTruncated() || rAddrConv.IsRowTruncated() || rAddrConv.IsTabTruncated() )
        return SCWARN_EXPORT_MAXROW;

    return eERR_OK;
}



ExportBiff8::ExportBiff8( XclExpRootData& rExpData, SvStream& rStrm ) :
    ExportBiff5( rExpData, rStrm )
{
    pExcRoot->eDateiTyp = Biff8;
    pExcRoot->pEscher = new XclEscher( GetDoc().GetTableCount(), *pExcRoot );
}


ExportBiff8::~ExportBiff8()
{
    delete pExcRoot->pEscher;
    pExcRoot->pEscher = NULL;
}


