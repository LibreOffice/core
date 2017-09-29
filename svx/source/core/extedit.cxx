/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/extedit.hxx>

#include <vcl/svapp.hxx>
#include <vcl/graph.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xoutbmp.hxx>
#include <svx/graphichelper.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdograf.hxx>
#include <svx/fmview.hxx>
#include <svtools/grfmgr.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <salhelper/thread.hxx>
#include <osl/file.hxx>
#include <osl/thread.hxx>
#include <osl/process.h>
#include <osl/time.h>
#include <svtools/filechangedchecker.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/logging.hxx>

#include <memory>

#include <com/sun/star/system/SystemShellExecute.hpp>
#include <com/sun/star/system/SystemShellExecuteFlags.hpp>

using namespace css::uno;
using namespace css::system;

ExternalToolEdit::ExternalToolEdit()
{
}

ExternalToolEdit::~ExternalToolEdit()
{
}

void ExternalToolEdit::HandleCloseEvent(ExternalToolEdit* pData)
{
    Graphic newGraphic;

    //import the temp file image stream into the newGraphic
    std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(pData->m_aFileName, StreamMode::READ));
    if(pStream)
    {
        GraphicConverter::Import(*pStream, newGraphic);

        // Now update the Graphic in the shell by re-reading from the newGraphic
        pData->Update( newGraphic );
    }
}

void ExternalToolEdit::StartListeningEvent()
{
    //Start an event listener implemented via VCL timeout
    assert(!m_pChecker.get());
    m_pChecker.reset(new FileChangedChecker(
            m_aFileName, [this] () { return HandleCloseEvent(this); }));
}

// self-destructing thread to make shell execute async
class ExternalToolEditThread
    : public ::salhelper::Thread
{
private:
    OUString const m_aFileName;

    virtual void execute() override;

public:
    explicit ExternalToolEditThread(OUString const& rFileName)
        : ::salhelper::Thread("ExternalToolEdit")
        , m_aFileName(rFileName)
    {}
};

void ExternalToolEditThread::execute()
{
    try
    {
        Reference<XSystemShellExecute> const xSystemShellExecute(
            SystemShellExecute::create( ::comphelper::getProcessComponentContext()));
        xSystemShellExecute->execute(m_aFileName, OUString(),
                SystemShellExecuteFlags::URIS_ONLY);
    }
    catch (Exception const& e)
    {
        SAL_WARN("svx", "ExternalToolEditThread: " << e);
    }
}

void ExternalToolEdit::Edit(GraphicObject const*const pGraphicObject)
{
    //Get the graphic from the GraphicObject
    const Graphic& aGraphic = pGraphicObject->GetGraphic();

    //get the Preferred File Extension for this graphic
    OUString fExtension;
    GraphicHelper::GetPreferredExtension(fExtension, aGraphic);

    //Create the temp File
    OUString aTempFileBase;
    OUString aTempFileName;

    osl::FileBase::RC rc =
        osl::FileBase::createTempFile(nullptr, nullptr, &aTempFileBase);
    if (osl::FileBase::E_None != rc)
    {
        SAL_WARN("svx", "ExternalToolEdit::Edit: cannot create temp file");
        return;
    }

    // Move it to a file name with image extension properly set
    aTempFileName = aTempFileBase + "." + fExtension;
    // FIXME: this is pretty stupid, need a better osl temp file API
    rc = osl::File::move(aTempFileBase, aTempFileName);
    if (osl::FileBase::E_None != rc)
    {
        SAL_WARN("svx", "ExternalToolEdit::Edit: cannot move temp file");
        return;
    }

    //Write Graphic to the Temp File
    GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
    sal_uInt16 nFilter(rGraphicFilter.GetExportFormatNumberForShortName(fExtension));

    OUString aFilter(rGraphicFilter.GetExportFormatShortName(nFilter));

    // Write the Graphic to the file now
    XOutBitmap::WriteGraphic(aGraphic, aTempFileName, aFilter, XOutFlags::UseNativeIfPossible | XOutFlags::DontExpandFilename);

    // There is a possibility that sPath extension might have been changed if the
    // provided extension is not writable
    m_aFileName = aTempFileName;

    //Create a thread

    rtl::Reference<ExternalToolEditThread> const pThread(
            new ExternalToolEditThread(m_aFileName));
    pThread->launch();

    StartListeningEvent();
}

SdrExternalToolEdit::SdrExternalToolEdit(
        FmFormView *const pView, SdrObject *const pObj)
    : m_pView(pView)
    , m_pObj(pObj)
{
    assert(m_pObj && m_pView);
    StartListening(*m_pObj->GetModel());
}


void SdrExternalToolEdit::Notify(SfxBroadcaster & rBC, SfxHint const& rHint)
{
    SdrHint const*const pSdrHint(dynamic_cast<SdrHint const*>(&rHint));
    if (pSdrHint
        && (SdrHintKind::ModelCleared == pSdrHint->GetKind()
            || (pSdrHint->GetObject() == m_pObj
                && SdrHintKind::ObjectRemoved == pSdrHint->GetKind())))
    {
        m_pView = nullptr;
        m_pObj = nullptr;
        m_pChecker.reset(); // avoid modifying deleted object
        EndListening(rBC);
    }
}

void SdrExternalToolEdit::Update(Graphic & rGraphic)
{
    assert(m_pObj && m_pView); // timer should be deleted by Notify() too
    SdrPageView *const pPageView = m_pView->GetSdrPageView();
    if (pPageView)
    {
        SdrGrafObj *const pNewObj(static_cast<SdrGrafObj*>(m_pObj->Clone()));
        assert(pNewObj);
        OUString const description =
            m_pView->GetDescriptionOfMarkedObjects() + " External Edit";
        m_pView->BegUndo(description);
        pNewObj->SetGraphicObject(rGraphic);
        // set to new object before ReplaceObjectAtView() so that Notify() will
        // not delete the running timer and crash
        SdrObject *const pOldObj = m_pObj;
        m_pObj = pNewObj;
        m_pView->ReplaceObjectAtView(pOldObj, *pPageView, pNewObj);
        m_pView->EndUndo();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
