/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       [ Surendran Mahendran <surenspost@gmail.com>]
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

    String aFilter(rGraphicFilter.GetExportFormatShortName(nFilter));
    String sPath(aTempFileName);

    // Write the Graphic to the file now
    XOutBitmap::WriteGraphic(aGraphic, sPath, aFilter, XOUTBMP_USE_NATIVE_IF_POSSIBLE | XOUTBMP_DONT_EXPAND_FILENAME);

    // There is a possiblity that sPath extnesion might have been changed if the
    // provided extension is not writable
    m_aFileName = OUString(sPath);

    //Create a thread

    // Create the data that is needed by the thread later
    osl_createThread(ExternalToolEdit::threadWorker, this);
}
