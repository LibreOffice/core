/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "exoticfileloadexception.hxx"

#include <comphelper/interaction.hxx>
#include <com/sun/star/document/ExoticFileLoadException.hpp>

using namespace com::sun::star;

ExoticFileLoadException::ExoticFileLoadException(const OUString& rURL,
                                                 const OUString& rFilterUIName)
    : m_xAbort(new comphelper::OInteractionAbort)
    , m_xApprove(new comphelper::OInteractionApprove)
    , m_lContinuations{ m_xApprove, m_xAbort }
{
    document::ExoticFileLoadException aReq;
    aReq.URL = rURL;
    aReq.FilterUIName = rFilterUIName;

    m_aRequest <<= aReq;
}

bool ExoticFileLoadException::isApprove() const
{
    comphelper::OInteractionApprove* pBase
        = static_cast<comphelper::OInteractionApprove*>(m_xApprove.get());
    return pBase->wasSelected();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
