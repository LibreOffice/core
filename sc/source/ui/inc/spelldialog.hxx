/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef SC_SPELLDIALOG_HXX
#define SC_SPELLDIALOG_HXX

#include <memory>
#include <svx/SpellDialogChildWindow.hxx>

// ============================================================================

class ScConversionEngineBase;
class ScSelectionState;
class ScTabViewShell;
class ScViewData;
class ScDocShell;
class ScDocument;

/** Specialized spell check dialog child window for Calc.

    This derivation of the ::svx::SpellDialogChildWindow base class provides
    Calc specific implementations of the virtual functions GetNextWrongSentence()
    and ApplyChangedSentence().
 */
class ScSpellDialogChildWindow : public ::svx::SpellDialogChildWindow
{
public:
    SFX_DECL_CHILDWINDOW( ScSpellDialogChildWindow );

    explicit            ScSpellDialogChildWindow( Window* pParent, sal_uInt16 nId,
                            SfxBindings* pBindings, SfxChildWinInfo* pInfo );
    virtual             ~ScSpellDialogChildWindow();

    /** This method makes the one from the base class public so that
        it can be called from the view shell when one is created.
    */
    virtual void        InvalidateSpellDialog();

protected:
    /** Iterate over the sentences in all text shapes and stop at the
        next sentence with spelling errors. While doing so the view
        mode may be changed and text shapes are set into edit mode.
    */
    virtual ::svx::SpellPortions GetNextWrongSentence( bool bRecheck );

    /** This method is responsible for merging corrections made in the
        spelling dialog back into the document.
    */
    virtual void        ApplyChangedSentence( const ::svx::SpellPortions& rChanged, bool bRecheck );
    virtual void        GetFocus();
    virtual void        LoseFocus();

private:
    void                Reset();
    void                Init();

    bool                IsSelectionChanged();

private:
    typedef ::std::auto_ptr< ScConversionEngineBase >   ScConvEnginePtr;
    typedef ::std::auto_ptr< ScDocument >               ScDocumentPtr;
    typedef ::std::auto_ptr< ScSelectionState >         ScSelectionStatePtr;

    ScConvEnginePtr     mxEngine;
    ScDocumentPtr       mxUndoDoc;
    ScDocumentPtr       mxRedoDoc;
    ScSelectionStatePtr mxOldSel;           /// Old selection state for comparison.
    ScTabViewShell*     mpViewShell;
    ScViewData*         mpViewData;
    ScDocShell*         mpDocShell;
    ScDocument*         mpDoc;
    bool                mbNeedNextObj;
    bool                mbOldIdleDisabled;
};

// ============================================================================

#endif

