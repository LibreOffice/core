/*************************************************************************
 *
 *  $RCSfile: dwfunctr.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:58 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SC_DWFUNCTR_HXX
#define _SC_DWFUNCTR_HXX

#ifndef _SFX_CHILDWIN_HXX //autogen
#include <sfx2/childwin.hxx>
#endif

#ifndef _SFXDOCKWIN_HXX //autogen
#include <sfx2/dockwin.hxx>
#endif

#ifndef _SFXLSTNER_HXX //autogen
#include <svtools/lstner.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif

#ifndef SC_ANYREFDG_HXX
#include "anyrefdg.hxx"
#endif

#ifndef SC_FUNCUTL_HXX
#include "funcutl.hxx"
#endif

#ifndef SC_SCGLOB_HXX
#include "global.hxx"       // ScAddress
#endif

#ifndef SC_PRIVSPLT_HXX
#include "privsplt.hxx"
#endif

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
    ScFunctionChildWindow( Window*, USHORT, SfxBindings*,
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
    USHORT              nLeftSlot;
    USHORT              nRightSlot;
    ULONG               nMinWidth;
    ULONG               nMinHeight;
    Size                aOldSize;
    BOOL                bSizeFlag;
    BOOL                bInit;
    short               nDockMode;
    Point               aSplitterInitPos;
    ScFuncDesc*         pFuncDesc;
    USHORT              nArgs;
    String**            pArgArr;


    ScFuncDesc*     aLRUList[LRU_MAX];

    void            UpdateFunctionList();
    void            UpdateLRUList();
    void            DoEnter(BOOL bOk); //@@ ???
    void            SetDescription();
    void            SetLeftRightSize();
    void            SetTopBottonSize();
    void            SetMyWidthLeRi(Size &aNewSize);
    void            SetMyHeightLeRi(Size &aNewSize);
    void            SetMyWidthToBo(Size &aNewSize);
    void            SetMyHeightToBo(Size &aNewSize);

                    DECL_LINK( SetSelectionHdl, void* );
                    DECL_LINK( SelHdl, ListBox* );
                    DECL_LINK(SetSplitHdl,ScPrivatSplit*);
                    DECL_LINK( TimerHdl, Timer*);

protected:

    virtual BOOL    Close();
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

    virtual void    SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                            const SfxHint& rHint, const TypeId& rHintType );

    void            SetSlotIDs( USHORT nLeft, USHORT nRight )
                        { nLeftSlot = nLeft; nRightSlot = nRight; }

    void            InitLRUList();

    void            Initialize (SfxChildWinInfo* pInfo);
    virtual void    FillInfo(SfxChildWinInfo&) const;
};

#endif


