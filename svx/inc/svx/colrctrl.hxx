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


#ifndef _SVX_COLRCTRL_HXX
#define _SVX_COLRCTRL_HXX

#include <sfx2/dockwin.hxx>
#include <sfx2/childwin.hxx>
#include <svx/SvxColorValueSet.hxx>
#include <svtools/transfer.hxx>
#include <svl/lstner.hxx>
#include "svx/svxdllapi.h"

class XColorList;
class SvData;

/*************************************************************************
|*
|* SvxColorValueSet_docking
|*
\************************************************************************/

class SvxColorValueSet_docking : public SvxColorValueSet, public DragSourceHelper
{
private:
    using SvxColorValueSet::StartDrag;

    bool            mbLeftButton;
    Point           aDragPosPixel;

protected:

    void            DoDrag();

    // ValueSet
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    Command(const CommandEvent& rCEvt );

    // DragSourceHelper
    virtual void    StartDrag( sal_Int8 nAction, const Point& rPtPixel );

                    DECL_STATIC_LINK(SvxColorValueSet_docking, ExecDragHdl, void*);

public:
                    SvxColorValueSet_docking( Window* pParent, WinBits nWinStyle = WB_ITEMBORDER );
                    SvxColorValueSet_docking( Window* pParent, const ResId& rResId );

    bool IsLeftButton() const { return mbLeftButton; }
};

/*************************************************************************
|*
|* SvxColorDockingWindow
|*
\************************************************************************/

class SvxColorDockingWindow : public SfxDockingWindow, public SfxListener
{
 friend class SvxColorChildWindow;

private:
    XColorList*     pColorTable;
    SvxColorValueSet_docking    aColorSet;
    sal_uInt16              nLeftSlot;
    sal_uInt16              nRightSlot;
    sal_uInt16              nCols;
    sal_uInt16              nLines;
    long                nCount;
    Size                aItemSize;

//#if 0 // _SOLAR__PRIVATE
    void                FillValueSet();
    void                SetSize();
                        DECL_LINK( SelectHdl, void * );

    /** This function is called when the window gets the focus.  It grabs
        the focus to the color value set so that it can be controlled with
        the keyboard.
    */
    virtual void GetFocus (void);
//#endif

protected:
    virtual sal_Bool    Close();
    virtual void    Resize();
    virtual void    Resizing( Size& rSize );

public:
                    SvxColorDockingWindow( SfxBindings* pBindings,
                                           SfxChildWindow *pCW,
                                           Window* pParent,
                                           const ResId& rResId );
                    ~SvxColorDockingWindow();

    virtual void    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    void            SetSlotIDs( sal_uInt16 nLeft, sal_uInt16 nRight )
                        { nLeftSlot = nLeft; nRightSlot = nRight; }

    virtual long    Notify( NotifyEvent& rNEvt );
};

#endif

