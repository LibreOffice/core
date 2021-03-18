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

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
