/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <memory>
#include <type_traits>

#include <sfx2/bindings.hxx>
#include <sfx2/request.hxx>
#include <vcl/abstdlg.hxx>

namespace sfx2
{
// Helper that executes the dialog according to the call mode defined in the passed request:
// synchronously or asynchronously. User-provided callback endDialogFn(sal_Int32, SfxRequest&)
// is called, and the result of the dialog execution is passed to it, along with the request, to
// set the necessary return values. After endDialogFn exits, the dialog is disposed. Intended
// for use in request handlers.
template <class Fn>
requires(std::is_invocable_v<Fn, sal_Int32, SfxRequest&>) void ExecDialogPerRequestAndDispose(
    VclPtr<VclAbstractDialog> dlg, SfxRequest& req, Fn endDialogFn)
{
    if (req.GetCallMode() & SfxCallMode::SYNCHRON)
    {
        endDialogFn(dlg->Execute(), req);
        dlg->disposeOnce();
    }
    else
    {
        dlg->StartExecuteAsync(
            [ dlg, endDialogFn, pRequest = std::make_shared<SfxRequest>(req) ](sal_Int32 result) {
                endDialogFn(result, *pRequest);
                dlg->disposeOnce();
            });
        req.Ignore();
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
