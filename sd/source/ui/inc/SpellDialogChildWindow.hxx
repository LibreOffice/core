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

#pragma once

#include <svx/SpellDialogChildWindow.hxx>
#include <svl/lstner.hxx>

class SdOutliner;

namespace sd
{
/** This derivation of the svx::SpellDialogChildWindow base class
    provides Draw and Impress specific implementations of
    GetNextWrongSentence() and ApplyChangedSentence().
*/
class SpellDialogChildWindow : public svx::SpellDialogChildWindow, public SfxListener
{
public:
    SpellDialogChildWindow(vcl::Window* pParent, sal_uInt16 nId, SfxBindings* pBindings,
                           SfxChildWinInfo* pInfo);
    virtual ~SpellDialogChildWindow() override;

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    void InvalidateSpellDialog();

    // SfxListener
    virtual void Notify(SfxBroadcaster& rBC, const SfxHint& rHint) override;

    SFX_DECL_CHILDWINDOW_WITHID(SpellDialogChildWindow);

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual svx::SpellPortions GetNextWrongSentence(bool bRecheck) override;

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void ApplyChangedSentence(const svx::SpellPortions& rChanged, bool bRecheck) override;
    virtual void GetFocus() override;
    virtual void LoseFocus() override;

private:
    /** This outliner is used to do the main work of iterating over a
        document and finding sentences with spelling errors.
    */
    SdOutliner* mpSdOutliner;

    /** When this flag is <TRUE/> then eventually we have to destroy
        the outliner in mpSdOutliner.
    */
    bool mbOwnOutliner;

    /** Provide an outliner in the mpSdOutliner data member.  When the
        view shell has changed since the last call this include the
        deletion/release of formerly created/obtained one prior to
        construction/obtaining of a new one.
    */
    void ProvideOutliner();

    void EndSpellingAndClearOutliner();
};

} // end of namespace ::sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
