/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <svx/dialmgr.hxx>
#include <svx/ehdl.hxx>
#include <svx/svxerr.hxx>

SvxErrorContext::SvxErrorContext(sal_uInt16 nCtxIdP, OUString aArgOne, weld::Window* pWin)
    : SfxErrorContext(nCtxIdP, std::move(aArgOne), pWin, RID_SVXERRCTX)
{
}

OUString SvxErrorContext::Translate(TranslateId aId) const { return SvxResId(aId); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
