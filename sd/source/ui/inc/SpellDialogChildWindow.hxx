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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SPELLDIALOGCHILDWINDOW_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SPELLDIALOGCHILDWINDOW_HXX

#include <svx/SpellDialogChildWindow.hxx>


namespace sd {

class Outliner;

/** This derivation of the ::svx::SpellDialogChildWindow base class
    provides Draw and Impress specific implementations of
    GetNextWrongSentence() and ApplyChangedSentence().
*/
class SpellDialogChildWindow
    : public ::svx::SpellDialogChildWindow
{
public:
    SpellDialogChildWindow (
        ::Window* pParent,
        sal_uInt16 nId,
        SfxBindings* pBindings,
        SfxChildWinInfo* pInfo);
    virtual ~SpellDialogChildWindow (void);

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    virtual void InvalidateSpellDialog (void);

    SFX_DECL_CHILDWINDOW_WITHID(SpellDialogChildWindow);

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual ::svx::SpellPortions GetNextWrongSentence( bool bRecheck ) SAL_OVERRIDE;

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void ApplyChangedSentence(const ::svx::SpellPortions& rChanged, bool bRecheck) SAL_OVERRIDE;
    virtual void GetFocus (void) SAL_OVERRIDE;
    virtual void LoseFocus (void) SAL_OVERRIDE;

private:
    /** This outliner is used to do the main work of iterating over a
        document and finding sentences with spelling errors.
    */
    Outliner* mpSdOutliner;

    /** When this flag is <TRUE/> then eventually we have to destroy
        the outliner in mpSdOutliner.
    */
    bool mbOwnOutliner;

    /** Provide an outliner in the mpSdOutliner data member.  When the
        view shell has changed since the last call this include the
        deletion/release of formerly created/obtained one prior to
        construction/obtaining of a new one.
    */
    void ProvideOutliner (void);
};

} // end of namespace ::sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
