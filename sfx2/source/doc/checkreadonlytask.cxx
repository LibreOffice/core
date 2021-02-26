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
#include "checkreadonlytask.hxx"

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
