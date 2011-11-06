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


#ifndef _SC_DWFUNCTR_HXX
#define _SC_DWFUNCTR_HXX

#include <sfx2/childwin.hxx>
#include <sfx2/dockwin.hxx>
#include <svl/lstner.hxx>
#include <svtools/stdctrl.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include "anyrefdg.hxx"
#include "global.hxx"       // ScAddress
#include "privsplt.hxx"
#include "funcdesc.hxx"

#ifndef LRU_MAX
#define LRU_MAX 10
#endif
/*************************************************************************
|*
|* Ableitung vom SfxChildWindow als "Behaelter" fuer Controller
|*
\************************************************************************/

class ScFunctionChildWindow : public SfxChildWindow
{
 public:
    ScFunctionChildWindow( Window*, sal_uInt16, SfxBindings*,
                            SfxChildWinInfo* );

    SFX_DECL_CHILDWINDOW(ScFunctionChildWindow);
};

/*************************************************************************
|*
|* ScFuncDockWin
|*
\************************************************************************/

class ScFunctionDockWin : public SfxDockingWindow, public SfxListener
{

private:
    Timer               aTimer;
    ScPrivatSplit       aPrivatSplit;
    ListBox             aCatBox;
    ListBox             aFuncList;
    ListBox             aDDFuncList;
    ListBox*            pAllFuncList;

    SfxChildAlignment   eSfxNewAlignment;
    SfxChildAlignment   eSfxOldAlignment;
    ImageButton         aInsertButton;
    FixedText           aFiFuncDesc;
    sal_uInt16              nLeftSlot;
    sal_uInt16              nRightSlot;
    sal_uLong               nMinWidth;
    sal_uLong               nMinHeight;
    Size                aOldSize;
    sal_Bool                bSizeFlag;
    sal_Bool                bInit;
    short               nDockMode;
    Point               aSplitterInitPos;
    const ScFuncDesc*   pFuncDesc;
    sal_uInt16              nArgs;
    String**            pArgArr;


    const ScFuncDesc*   aLRUList[LRU_MAX];

    void            UpdateFunctionList();
    void            UpdateLRUList();
    void            DoEnter(sal_Bool bOk); //@@ ???
    void            SetDescription();
    void            SetLeftRightSize();
    void            SetTopBottonSize();
    void            SetMyWidthLeRi(Size &aNewSize);
    void            SetMyHeightLeRi(Size &aNewSize);
    void            SetMyWidthToBo(Size &aNewSize);
    void            SetMyHeightToBo(Size &aNewSize);
    void            UseSplitterInitPos();

                    DECL_LINK( SetSelectionHdl, void* );
                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK(SetSplitHdl,ScPrivatSplit*);
                    DECL_LINK( TimerHdl, Timer*);

protected:

    virtual sal_Bool    Close();
    virtual void    Resize();
    virtual void    Resizing( Size& rSize );
    virtual void    SetSize();
    virtual void    ToggleFloatingMode();
    virtual void    StateChanged( StateChangedType nStateChange );


    virtual SfxChildAlignment CheckAlignment(SfxChildAlignment,
                                SfxChildAlignment eAlign);

public:
                    ScFunctionDockWin(  SfxBindings* pBindings,
                                        SfxChildWindow *pCW,
                                        Window* pParent,
                                        const ResId& rResId );

                    ~ScFunctionDockWin();

    using SfxDockingWindow::Notify;
    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            SetSlotIDs( sal_uInt16 nLeft, sal_uInt16 nRight )
                        { nLeftSlot = nLeft; nRightSlot = nRight; }

    void            InitLRUList();

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const;
};

#endif


