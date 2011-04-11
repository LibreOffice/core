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

#include "precompiled_sw.hxx"
#include <extedit.hxx>
#include <view.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <wrtsh.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <osl/process.h>
#include <osl/time.h>
#include <svtools/filter.hxx>
#include <svtools/filechangedchecker.hxx>
#include <svx/xoutbmp.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <basesh.hxx>
#include "romenu.hxx"
#include "com/sun/star/system/XSystemShellExecute.hpp"
#include "com/sun/star/system/SystemShellExecuteFlags.hpp"
#include <comphelper/processfactory.hxx>

#include <boost/bind.hpp>

using namespace ::com::sun::star;

void HandleCloseEvent(const String& rURL, SwWrtShell *rSh)
{
    //create a new Graphic
    Graphic newGraphic;

    //import the temp file image stream into the newGraphic
    SvStream*  pStream = utl::UcbStreamHelper::CreateStream(rURL, STREAM_READ);
    if(pStream)
    {
        GraphicConverter::Import(*pStream, newGraphic);

        // Now update the Graphic in the shell by re-reading from the newGraphic
        // TODO: Make this action Undoable !
        rSh->ReRead(aEmptyStr, aEmptyStr, (const Graphic*) &newGraphic);

        delete(pStream);
    }
}

IMPL_LINK (ExternalProcessClass_Impl, StartListeningEvent, void*, pEvent)
{
    //Start an event listener implemented via VCL timeout
    Data *pData = ( Data* )pEvent;
    String aURL( pData->fileName );

    new FileChangedChecker(
        pData->fileName,
        ::boost::bind(&HandleCloseEvent,aURL,pData->rSh));

    return 0;
}

void pWorker(void *pThreadData)
{
    Data *pData = (Data*)(pThreadData);
    rtl_uString *aFileName = new rtl_uString();
    rtl_uString_newFromAscii (
            &aFileName, rtl::OUStringToOString(
                pData->fileName, RTL_TEXTENCODING_UTF8).getStr());

    // Make an asynchronous call to listen to the event of temporary image file
    // getting changed
    Application::PostUserEvent(
            LINK(NULL, ExternalProcessClass_Impl, StartListeningEvent), pThreadData);

    uno::Reference< com::sun::star::system::XSystemShellExecute > xSystemShellExecute(
            ::comphelper::getProcessServiceFactory()->createInstance(
                DEFINE_CONST_UNICODE("com.sun.star.system.SystemShellExecute") ), uno::UNO_QUERY_THROW );
    xSystemShellExecute->execute( pData->fileName, rtl::OUString(),  com::sun::star::system::SystemShellExecuteFlags::DEFAULTS );
}

void EditWithExternalTool(GraphicObject *pGraphicObject, SwWrtShell *rSh)
{
    //Get the graphic from the GraphicObject
    const Graphic pGraphic = pGraphicObject->GetGraphic();

    //get the Preferred File Extension for this graphic
    String fExt;
    GetPreferedExtension(fExt, pGraphic);

    //Create the temp File
    rtl::OUString tempFileBase, tempFileName;
    oslFileHandle pHandle;
    osl::FileBase::createTempFile(0, &pHandle, &tempFileBase);

    // Move it to a file name with image extension properly set
    tempFileName = tempFileBase + rtl::OUString(String('.')) +
        rtl::OUString(fExt);
    osl::File::move(tempFileBase, tempFileName);

    //Write Graphic to the Temp File
    GraphicFilter& rGF = *GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilter(rGF.GetExportFormatNumber(fExt));
    String aFilter(rGF.GetExportFormatShortName(nFilter));
    String sPath(tempFileName.getStr());

    // Write the Graphic to the file now
    XOutBitmap::WriteGraphic(pGraphic, sPath, aFilter,
            XOUTBMP_USE_NATIVE_IF_POSSIBLE|XOUTBMP_DONT_EXPAND_FILENAME);

    // There is a possiblity that sPath extnesion might have been changed if the
    // provided extension is not writable
    tempFileName = rtl::OUString(sPath);

    //Create a thread
    rtl_uString *fileName = new rtl_uString();
    rtl_uString_newFromAscii(
            &fileName, rtl::OUStringToOString(tempFileName,
                RTL_TEXTENCODING_UTF8).getStr());

    // Create the data that is needed by the thread later
    Data *pThreadData = new Data();
    pThreadData->pGraphicObject = pGraphicObject;
    pThreadData->fileName = fileName;
    pThreadData->rSh = rSh ;

    osl_createThread(pWorker, pThreadData);

}
