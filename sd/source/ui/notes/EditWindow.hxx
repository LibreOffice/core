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



#ifndef SD_EDIT_WINDOW_HXX
#define SD_EDIT_WINDOW_HXX

#include <vcl/window.hxx>
#include <svtools/transfer.hxx>
#include <vcl/timer.hxx>
#include <editeng/editdata.hxx>
#include <svtools/colorcfg.hxx>

class EditEngine;
class EditStatus;
class EditView;
class Menu;
class ScrollBar;
class ScrollBarBox;
class SfxItemPool;
class Timer;


namespace sd { namespace notes {

class EditWindow
    : public Window,
      public DropTargetHelper
{
public:
    EditWindow (Window* pParentWindow, SfxItemPool* pItemPool);
    ~EditWindow (void);

    void InsertText (const String &rText);

    using Window::GetText;
private:
    EditView* mpEditView;
    EditEngine* mpEditEngine;
    SfxItemPool* mpEditEngineItemPool;
    ScrollBar* mpHorizontalScrollBar;
    ScrollBar* mpVerticalScrollBar;
    ScrollBarBox* mpScrollBox;
    Timer maModifyTimer;
    Timer maCursorMoveTimer;
    ESelection maOldSelection;

    virtual void KeyInput(const KeyEvent& rKEvt);
    virtual void Command(const CommandEvent& rCEvt);
    DECL_LINK(MenuSelectHdl, Menu *);

    virtual void DataChanged( const DataChangedEvent& );
    virtual void Resize();
    virtual void MouseMove(const MouseEvent &rEvt);
    virtual void MouseButtonUp(const MouseEvent &rEvt);
    virtual void MouseButtonDown(const MouseEvent &rEvt);

    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );
    virtual void Paint(const Rectangle& rRect);

    DECL_LINK(EditStatusHdl ,EditStatus *);
    DECL_LINK(ScrollHdl, ScrollBar *);

    void        CreateEditView();

    Rectangle   AdjustScrollBars();
    void        SetScrollBarRanges();
    void        InitScrollBars();

    //    SmDocShell *    GetDoc();
    //    SmViewShell *   GetView();
    EditView* GetEditView (void);
    EditEngine* GetEditEngine (void);
    EditEngine* CreateEditEngine (void);

    // Window
    virtual void        SetText(const XubString &rText);
    virtual XubString   GetText();
    virtual void        GetFocus();
    virtual void        LoseFocus();

    ESelection          GetSelection() const;
    void                SetSelection(const ESelection &rSel);

    bool                IsEmpty() const;
    bool                IsSelected() const;
    bool                IsAllSelected() const;

    void                Cut();
    void                Copy();
    void                Paste();
    void                Delete();
    void                SelectAll();
    void                MarkError(const Point &rPos);
    void                SelNextMark();
    void                SelPrevMark();

    bool                HasMark(const String &rText) const;

    void ApplyColorConfigValues( const svtools::ColorConfig &rColorCfg );
};

} } // end of namespace ::sd::notes

#endif

