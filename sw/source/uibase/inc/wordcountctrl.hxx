/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_WORDCOUNTCTRL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_WORDCOUNTCTRL_HXX

#include <sfx2/stbitem.hxx>

/**
Word count status bar control for Writer.

@remarks This is a simple status bar control of type SfxStringItem, and it has no custom
logic whatsoever. The actual updating of the word count string happens in
SwView::StateStatusLine (see sw/source/ui/uiview/view2.cxx).
*/
class SwWordCountStatusBarControl : public SfxStatusBarControl
{
public:
    SFX_DECL_STATUSBAR_CONTROL();

    SwWordCountStatusBarControl(sal_uInt16 nSlotId, sal_uInt16 nId, StatusBar& rStb);
    virtual ~SwWordCountStatusBarControl();

    virtual void StateChanged( sal_uInt16 nSID, SfxItemState eState,
                               const SfxPoolItem* pState ) override;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
