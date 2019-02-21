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

#include <memory>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sfx2/docfile.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfillit0.hxx>

#include <sddll.hxx>
#include <sdpage.hxx>
#include <drawdoc.hxx>
#include <sdcgmfilter.hxx>

#include <DrawDocShell.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::frame;

typedef sal_uInt32 ( *ImportCGMPointer )(SvStream&, Reference< XModel > const &, Reference< XStatusIndicator > const &);

#ifdef DISABLE_DYNLOADING

extern "C" sal_uInt32 ImportCGM(SvStream&, Reference< XModel > const &, Reference< XStatusIndicator > const &);

#endif

SdCGMFilter::SdCGMFilter( SfxMedium& rMedium, ::sd::DrawDocShell& rDocShell ) :
    SdFilter( rMedium, rDocShell )
{
}

SdCGMFilter::~SdCGMFilter()
{
}

namespace
{
    class CGMPointer
    {
        ImportCGMPointer m_pPointer;
    public:
        CGMPointer()
        {
#ifdef DISABLE_DYNLOADING
            m_pPointer = ImportCGM;
#else
            m_pPointer = reinterpret_cast<ImportCGMPointer>(
                SdFilter::GetLibrarySymbol("icg", "ImportCGM"));
#endif
        }
        ImportCGMPointer get() { return m_pPointer; }
    };
}

bool SdCGMFilter::Import()
{
    bool        bRet = false;

    CGMPointer aPointer;
    ImportCGMPointer FncImportCGM = aPointer.get();
    if (FncImportCGM && mxModel.is())
    {
        OUString aFileURL( mrMedium.GetURLObject().GetMainURL( INetURLObject::DecodeMechanism::NONE ) );
        sal_uInt32          nRetValue;

        if( mrDocument.GetPageCount() == 0 )
            mrDocument.CreateFirstPages();

        CreateStatusIndicator();
        std::unique_ptr<SvStream> xIn(::utl::UcbStreamHelper::CreateStream(aFileURL, StreamMode::READ));
        nRetValue = xIn ? FncImportCGM(*xIn, mxModel, mxStatusIndicator) : 0;

        if( nRetValue )
        {
            bRet = true;

            if( ( nRetValue &~0xff000000 ) != 0xffffff )    // maybe the backgroundcolor is already white
            {                                               // so we must not set a master page
                mrDocument.StopWorkStartupDelay();
                SdPage* pSdPage = mrDocument.GetMasterSdPage(0, PageKind::Standard);

                if(pSdPage)
                {
                    // set PageFill to given color
                    const Color aColor(static_cast<sal_uInt8>(nRetValue >> 16), static_cast<sal_uInt8>(nRetValue >> 8), static_cast<sal_uInt8>(nRetValue >> 16));
                    pSdPage->getSdrPageProperties().PutItem(XFillColorItem(OUString(), aColor));
                    pSdPage->getSdrPageProperties().PutItem(XFillStyleItem(drawing::FillStyle_SOLID));
                }
            }
        }
    }
    return bRet;
}

bool SdCGMFilter::Export()
{
    // No ExportCGM function exists(!)
    return false;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportCGM(SvStream &rStream)
{
    SdDLL::Init();

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Impress);

    CGMPointer aPointer;

    xDocShRef->GetDoc()->EnableUndo(false);
    bool bRet = aPointer.get()(rStream, xDocShRef->GetModel(), css::uno::Reference<css::task::XStatusIndicator>()) == 0;

    xDocShRef->DoClose();

    return bRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
