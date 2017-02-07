/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "PopupRequest.hxx"

using namespace css;

namespace chart
{

PopupRequest::PopupRequest()
    : impl::PopupRequest_Base(m_aMutex)
{
}

PopupRequest::~PopupRequest()
{}

// ____ XRequestCallback ____

void SAL_CALL PopupRequest::addCallback(const uno::Reference<awt::XCallback>& xCallback,
                                        const uno::Any& /*aData*/)
{
    m_xCallback = xCallback;
    uno::Any aAny;
}

// ____ WeakComponentImplHelperBase ____
// is called when dispose() is called at this component
void SAL_CALL PopupRequest::disposing()
{
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
