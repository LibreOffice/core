/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/docfile.hxx>
#include <osl/file.hxx>
#include <vcl/svapp.hxx>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/uno/Reference.h>
#include <ucbhelper/interactionrequest.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/documentlockfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <com/sun/star/document/ReloadEditableRequest.hpp>
#include <sfx2/viewfrm.hxx>
#include <com/sun/star/io/WrongFormatException.hpp>
#include "checkreadonlytask.hxx"

using namespace ::com::sun::star;

/** callback function, which is triggered by worker thread after successfully checking if the file
     is editable. Sent from <Application::PostUserEvent(..)>
     Note: This method has to be run in the main thread.
*/
IMPL_STATIC_LINK(SfxMedium, ShowReloadEditableDialog, void*, p, void)
{
    SfxMedium* pMed = static_cast<SfxMedium*>(p);
    if (pMed == nullptr)
        return;
    pMed->SetCheckEditableWorkerDone();
    uno::Reference<task::XInteractionHandler> xHandler = pMed->GetInteractionHandler();
    if (xHandler.is())
    {
        OUString aDocumentURL
            = pMed->GetURLObject().GetLastName(INetURLObject::DecodeMechanism::WithCharset);
        ::rtl::Reference<::ucbhelper::InteractionRequest> xInteractionRequestImpl;
        xInteractionRequestImpl
            = new ::ucbhelper::InteractionRequest(uno::makeAny(document::ReloadEditableRequest(
                OUString(), uno::Reference<uno::XInterface>(), aDocumentURL)));
        if (xInteractionRequestImpl != nullptr)
        {
            sal_Int32 nContinuations = 2;
            uno::Sequence<uno::Reference<task::XInteractionContinuation>> aContinuations(
                nContinuations);
            aContinuations[0] = new ::ucbhelper::InteractionAbort(xInteractionRequestImpl.get());
            aContinuations[1] = new ::ucbhelper::InteractionApprove(xInteractionRequestImpl.get());
            xInteractionRequestImpl->setContinuations(aContinuations);
            xHandler->handle(xInteractionRequestImpl);
            ::rtl::Reference<::ucbhelper::InteractionContinuation> xSelected
                = xInteractionRequestImpl->getSelection();
            if (uno::Reference<task::XInteractionApprove>(xSelected.get(), uno::UNO_QUERY).is())
            {
                for (SfxViewFrame* pFrame = SfxViewFrame::GetFirst(); pFrame;
                     pFrame = SfxViewFrame::GetNext(*pFrame))
                {
                    if (pFrame->GetObjectShell()->GetMedium() == pMed)
                    {
                        // special case to ensure view isn't set to read-only in
                        // SfxViewFrame::ExecReload_Impl after reloading
                        pMed->SetOriginallyReadOnly(false);
                        pFrame->GetDispatcher()->Execute(SID_RELOAD);
                        break;
                    }
                }
            }
        }
    }
}

bool SfxMedium::CheckCanGetLockfile() const
{
    bool bCanReload = false;
    ::svt::DocumentLockFile aLockFile(GetName());
    LockFileEntry aData;
    osl::DirectoryItem rItem;
    auto nError1 = osl::DirectoryItem::get(aLockFile.GetURL(), rItem);
    if (nError1 == osl::FileBase::E_None)
    {
        try
        {
            aData = aLockFile.GetLockData();
        }
        catch (const io::WrongFormatException&)
        {
            // we get empty or corrupt data
            return false;
        }
        catch (const uno::Exception&)
        {
            // locked from other app
            return false;
        }
        LockFileEntry aOwnData = svt::LockFileCommon::GenerateOwnEntry();
        bool bOwnLock
            = aOwnData[LockFileComponent::SYSUSERNAME] == aData[LockFileComponent::SYSUSERNAME];
        if (bOwnLock
            && aOwnData[LockFileComponent::LOCALHOST] == aData[LockFileComponent::LOCALHOST]
            && aOwnData[LockFileComponent::USERURL] == aData[LockFileComponent::USERURL])
        {
            // this is own lock from the same installation, it could remain because of crash
            bCanReload = true;
        }
    }
    else if (nError1 == osl::FileBase::E_NOENT) // file doesn't exist
    {
        bCanReload = true;
    }
    return bCanReload;
}

CheckReadOnlyTask::CheckReadOnlyTask(SfxMedium* pMed,
                                     const std::shared_ptr<std::recursive_mutex>& pMutex,
                                     const std::shared_ptr<std::condition_variable_any>& pCond,
                                     const std::shared_ptr<bool>& pIsDestructed,
                                     const std::shared_ptr<comphelper::ThreadTaskTag>& pTag)
    : ThreadTask(pTag)
    , _pMed(pMed)
    , _pMutex(pMutex)
    , _pMyCond(pCond)
    , _pIsDestructed(pIsDestructed)
{
}

// worker thread method
void CheckReadOnlyTask::doWork()
{
    if (_pMed == nullptr || _pMyCond == nullptr || _pMutex == nullptr || _pIsDestructed == nullptr)
        return;

    std::unique_lock<std::recursive_mutex> lock(*_pMutex);

    if (*_pIsDestructed || _pMed->GetWorkerReloadEvent() != nullptr
        || _pMed->GetCheckEditableWorkerRunning())
        return;

    _pMed->SetCheckEditableWorkerRunning(true);

    OUString aDocumentURL
        = _pMed->GetURLObject().GetMainURL(INetURLObject::DecodeMechanism::WithCharset);

    while (true)
    {
        switch (_pMyCond->wait_for(lock, std::chrono::seconds(60)))
        {
            case std::cv_status::timeout:
                break;
            default: // signalled or error
                _pMed->SetCheckEditableWorkerRunning(false);
                return;
        }

        // prevent multiple reload events from occurring
        if (*_pIsDestructed || _pMed->GetWorkerReloadEvent() != nullptr
            || !_pMed->GetCheckEditableWorkerRunning())
        {
            _pMed->SetCheckEditableWorkerRunning(false);
            return;
        }

        // close the file handle so we can open it below and check for write access
        _pMed->UnlockFile(true);

        osl::File aFile(aDocumentURL);
        if (aFile.open(osl_File_OpenFlag_Write) != osl::FileBase::E_None)
            continue;

        if (!_pMed->CheckCanGetLockfile())
            continue;

        _pMed->SetCheckEditableWorkerRunning(false);

        if (aFile.close() != osl::FileBase::E_None)
            return;

        // we can load, ask user
        ImplSVEvent* pEvent
            = Application::PostUserEvent(LINK(nullptr, SfxMedium, ShowReloadEditableDialog), _pMed);
        _pMed->SetWorkerReloadEvent(pEvent);

        return;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
