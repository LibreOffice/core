/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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

    //import the temp file image stream into the newGraphic
    SvStream* pStream = utl::UcbStreamHelper::CreateStream(pData->m_aFileName, STREAM_READ);
    if(pStream)
    {
        GraphicConverter::Import(*pStream, newGraphic);

        // Now update the Graphic in the shell by re-reading from the newGraphic
        pData->Update( newGraphic );

        delete(pStream);
    }
}

IMPL_LINK (ExternalToolEdit, StartListeningEvent, void*, pEvent)
{
    //Start an event listener implemented via VCL timeout
    ExternalToolEdit* pData = ( ExternalToolEdit* )pEvent;

    new FileChangedChecker(pData->m_aFileName, ::boost::bind(&HandleCloseEvent, pData));

    return 0;
}

void ExternalToolEdit::threadWorker(void* pThreadData)
{
    ExternalToolEdit* pData = (ExternalToolEdit*) pThreadData;

    // Make an asynchronous call to listen to the event of temporary image file
    // getting changed
    Application::PostUserEvent( LINK( NULL, ExternalToolEdit, StartListeningEvent ), pThreadData);

    Reference<XSystemShellExecute> xSystemShellExecute(
        SystemShellExecute::create( ::comphelper::getProcessComponentContext() ) );
    xSystemShellExecute->execute( pData->m_aFileName, OUString(), SystemShellExecuteFlags::URIS_ONLY );
}

void ExternalToolEdit::Edit( GraphicObject* pGraphicObject )
{
    //Get the graphic from the GraphicObject
    m_pGraphicObject = pGraphicObject;
    const Graphic aGraphic = pGraphicObject->GetGraphic();

    //get the Preferred File Extension for this graphic
    OUString fExtension;
    GraphicHelper::GetPreferedExtension(fExtension, aGraphic);

    //Create the temp File
    OUString aTempFileBase;
    OUString aTempFileName;

    oslFileHandle pHandle;
    osl::FileBase::createTempFile(0, &pHandle, &aTempFileBase);

    // Move it to a file name with image extension properly set
    aTempFileName = aTempFileBase + OUString('.') + OUString(fExtension);
    osl::File::move(aTempFileBase, aTempFileName);

    //Write Graphic to the Temp File
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilter(rGraphicFilter.GetExportFormatNumber(fExtension));

    OUString aFilter(rGraphicFilter.GetExportFormatShortName(nFilter));

    // Write the Graphic to the file now
    XOutBitmap::WriteGraphic(aGraphic, aTempFileName, aFilter, XOUTBMP_USE_NATIVE_IF_POSSIBLE | XOUTBMP_DONT_EXPAND_FILENAME);

    // There is a possiblity that sPath extension might have been changed if the
    // provided extension is not writable
    m_aFileName = aTempFileName;

    //Create a thread

    // Create the data that is needed by the thread later
    osl_createThread(ExternalToolEdit::threadWorker, this);
}
