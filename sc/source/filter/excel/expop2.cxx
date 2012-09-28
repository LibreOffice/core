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

#include <unotools/fltrcfg.hxx>

#include <sfx2/objsh.hxx>
#include <sfx2/docinf.hxx>
#include <filter/msfilter/svxmsbas.hxx>

#include "scerrors.hxx"
#include "scextopt.hxx"

#include "root.hxx"
#include "excdoc.hxx"
#include "exp_op.hxx"

#include "xcl97esc.hxx"

#include "document.hxx"
#include "rangenam.hxx"
#include "filtopt.hxx"
#include "xltools.hxx"
#include "xelink.hxx"

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
    OSL_ENSURE( pDocShell, "ExportBiff5::Write - no document shell" );

    SotStorageRef xRootStrg = GetRootStorage();
    OSL_ENSURE( xRootStrg.Is(), "ExportBiff5::Write - no root storage" );

    bool bWriteBasicStrg = false;
    if( GetBiff() == EXC_BIFF8 )
    {
        const SvtFilterOptions& rFilterOpt = SvtFilterOptions::Get();
        bWriteBasicStrg = rFilterOpt.IsLoadExcelBasicStorage();
    }

    if( pDocShell && xRootStrg.Is() && bWriteBasicStrg )
    {
        SvxImportMSVBasic aBasicImport( *pDocShell, *xRootStrg );
        sal_uLong nErr = aBasicImport.SaveOrDelMSVBAStorage( sal_True, EXC_STORAGE_VBA_PROJECT );
        if( nErr != ERRCODE_NONE )
            pDocShell->SetError( nErr, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
    }

    pExcDoc->ReadDoc();         // ScDoc -> ExcDoc
    pExcDoc->Write( aOut );     // wechstreamen

    if( pDocShell && xRootStrg.Is() )
    {
        // #i88642# update doc info (revision etc)
        pDocShell->UpdateDocInfoForSave();

        using namespace ::com::sun::star;
        uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pDocShell->GetModel(), uno::UNO_QUERY_THROW);
        uno::Reference<document::XDocumentProperties> xDocProps
                = xDPS->getDocumentProperties();
        if ( SvtFilterOptions::Get().IsEnableCalcPreview() )
        {
            ::boost::shared_ptr<GDIMetaFile> pMetaFile =
                pDocShell->GetPreviewMetaFile (false);
            uno::Sequence<sal_uInt8> metaFile(
                sfx2::convertMetaFile(pMetaFile.get()));
            sfx2::SaveOlePropertySet(xDocProps, xRootStrg, &metaFile);
        }
        else
            sfx2::SaveOlePropertySet(xDocProps, xRootStrg );
    }

    const XclExpAddressConverter& rAddrConv = GetAddressConverter();
    if( rAddrConv.IsRowTruncated() )
        return SCWARN_EXPORT_MAXROW;
    if( rAddrConv.IsColTruncated() )
        return SCWARN_EXPORT_MAXCOL;
    if( rAddrConv.IsTabTruncated() )
        return SCWARN_EXPORT_MAXTAB;

    return eERR_OK;
}



ExportBiff8::ExportBiff8( XclExpRootData& rExpData, SvStream& rStrm ) :
    ExportBiff5( rExpData, rStrm )
{
    pExcRoot->eDateiTyp = Biff8;
}


ExportBiff8::~ExportBiff8()
{
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
