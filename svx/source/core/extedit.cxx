/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 */

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/extedit.hxx>
#include <svx/graphichelper.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <osl/process.h>
#include <osl/time.h>
#include <svtools/filechangedchecker.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <boost/bind.hpp>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

using namespace css::uno;
using namespace css::system;

ExternalToolEdit::ExternalToolEdit()
{}

ExternalToolEdit::~ExternalToolEdit()
{}

void ExternalToolEdit::HandleCloseEvent(ExternalToolEdit* pData)
{
    Graphic newGraphic;

    
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(pData->m_aFileName, STREAM_READ);
    if(pStream)
    {
        GraphicConverter::Import(*pStream, newGraphic);

        
        pData->Update( newGraphic );

        delete(pStream);
    }
}

IMPL_LINK (ExternalToolEdit, StartListeningEvent, void*, pEvent)
{
    
    ExternalToolEdit* pData = ( ExternalToolEdit* )pEvent;

    new FileChangedChecker(pData->m_aFileName, ::boost::bind(&HandleCloseEvent, pData));

    return 0;
}

void ExternalToolEdit::threadWorker(void* pThreadData)
{
    ExternalToolEdit* pData = (ExternalToolEdit*) pThreadData;

    
    
    Application::PostUserEvent( LINK( NULL, ExternalToolEdit, StartListeningEvent ), pThreadData);

    Reference<XSystemShellExecute> xSystemShellExecute(
        SystemShellExecute::create( ::comphelper::getProcessComponentContext() ) );
    xSystemShellExecute->execute( pData->m_aFileName, OUString(), SystemShellExecuteFlags::URIS_ONLY );
}

void ExternalToolEdit::Edit( GraphicObject* pGraphicObject )
{
    
    m_pGraphicObject = pGraphicObject;
    const Graphic aGraphic = pGraphicObject->GetGraphic();

    
    OUString fExtension;
    GraphicHelper::GetPreferredExtension(fExtension, aGraphic);

    
    OUString aTempFileBase;
    OUString aTempFileName;

    oslFileHandle pHandle;
    osl::FileBase::createTempFile(0, &pHandle, &aTempFileBase);

    
    aTempFileName = aTempFileBase + OUString('.') + OUString(fExtension);
    osl::File::move(aTempFileBase, aTempFileName);

    
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilter(rGraphicFilter.GetExportFormatNumber(fExtension));

    OUString aFilter(rGraphicFilter.GetExportFormatShortName(nFilter));

    
    XOutBitmap::WriteGraphic(aGraphic, aTempFileName, aFilter, XOUTBMP_USE_NATIVE_IF_POSSIBLE | XOUTBMP_DONT_EXPAND_FILENAME);

    
    
    m_aFileName = aTempFileName;

    

    
    osl_createThread(ExternalToolEdit::threadWorker, this);
}
