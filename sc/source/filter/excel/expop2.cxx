/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
            pDocShell->SetError( nErr, OUString( RTL_CONSTASCII_USTRINGPARAM( OSL_LOG_PREFIX ) ) );
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
