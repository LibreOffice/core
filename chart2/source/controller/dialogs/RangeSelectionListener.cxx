/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <RangeSelectionListener.hxx>
#include <ChartModel.hxx>
#include <vcl/weld.hxx>
#include <utility>

using namespace ::com::sun::star;

namespace chart
{

RangeSelectionListener::RangeSelectionListener(
    RangeSelectionListenerParent & rParent,
    OUString aInitialRange,
    const rtl::Reference<::chart::ChartModel>& xModelToLockController ) :
        m_rParent( rParent ),
        m_aRange(std::move( aInitialRange )),
        m_aControllerLockGuard( xModelToLockController )
{}

RangeSelectionListener::~RangeSelectionListener()
{}

// ____ XRangeSelectionListener ____
void SAL_CALL RangeSelectionListener::done( const sheet::RangeSelectionEvent& aEvent )
{
    m_aRange = aEvent.RangeDescriptor;
    m_rParent.listeningFinished( m_aRange );
}

void SAL_CALL RangeSelectionListener::aborted( const sheet::RangeSelectionEvent& /*aEvent*/ )
{
    m_rParent.listeningFinished( m_aRange );
}

// ____ XEventListener ____
void SAL_CALL RangeSelectionListener::disposing( const lang::EventObject& /*Source*/ )
{
    m_rParent.disposingRangeSelection();
}

void RangeSelectionListenerParent::enableRangeChoosing(bool bEnable, weld::DialogController* pDialog)
{
    if (!pDialog)
        return;
    weld::Dialog* pDlg = pDialog->getDialog();
    // tdf#158753 save the current page when hiding the wizard,
    // and restore it on showing the wizard to workaround that
    // that GtkAssistant resets to page 0 on hide+show
    weld::Assistant* pAss = dynamic_cast<weld::Assistant*>(pDlg);
    if (pAss && bEnable)
        m_sRestorePageIdent = pAss->get_current_page_ident();
    pDlg->set_modal(!bEnable);
    pDlg->set_visible(!bEnable);
    if (pAss && !bEnable)
        pAss->set_current_page(m_sRestorePageIdent);
}

} //  namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
