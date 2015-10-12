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

#ifndef INCLUDED_SC_SOURCE_UI_INC_SPELLDIALOG_HXX
#define INCLUDED_SC_SOURCE_UI_INC_SPELLDIALOG_HXX

#include <memory>
#include <svx/SpellDialogChildWindow.hxx>

class ScConversionEngineBase;
class ScSelectionState;
class ScTabViewShell;
class ScViewData;
class ScRangeList;
class ScDocShell;
class ScDocument;

/** Specialized spell check dialog child window for Calc.

    This derivation of the svx::SpellDialogChildWindow base class provides
    Calc specific implementations of the virtual functions GetNextWrongSentence()
    and ApplyChangedSentence().
 */
class ScSpellDialogChildWindow : public svx::SpellDialogChildWindow
{
public:
    SFX_DECL_CHILDWINDOW_WITHID( ScSpellDialogChildWindow );

    explicit            ScSpellDialogChildWindow( vcl::Window* pParent, sal_uInt16 nId,
                            SfxBindings* pBindings, SfxChildWinInfo* pInfo );
    virtual             ~ScSpellDialogChildWindow();

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    void        InvalidateSpellDialog();

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual svx::SpellPortions GetNextWrongSentence( bool bRecheck ) override;

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void        ApplyChangedSentence( const svx::SpellPortions& rChanged, bool bRecheck ) override;
    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;

private:
    void                Reset();
    void                Init();

    bool                IsSelectionChanged();

private:
    typedef ::std::unique_ptr< ScConversionEngineBase >   ScConvEnginePtr;
    typedef ::std::unique_ptr< ScDocument >               ScDocumentPtr;
    typedef ::std::unique_ptr< ScSelectionState >         ScSelectionStatePtr;
    typedef ::std::unique_ptr< ScRangeList >              ScRangeListPtr;

    ScConvEnginePtr     mxEngine;
    ScDocumentPtr       mxUndoDoc;
    ScDocumentPtr       mxRedoDoc;
    ScSelectionStatePtr mxOldSel;           /// For cursor position in selection
    ScRangeListPtr      mxOldRangeList;     /// Original selection range for comparison.
    ScTabViewShell*     mpViewShell;
    ScViewData*         mpViewData;
    ScDocShell*         mpDocShell;
    ScDocument*         mpDoc;
    bool                mbNeedNextObj;
    bool                mbOldIdleEnabled;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
