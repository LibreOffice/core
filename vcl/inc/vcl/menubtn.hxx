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



#ifndef _SV_MENUBTN_HXX
#define _SV_MENUBTN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/button.hxx>

class Timer;
class PopupMenu;

// --------------------
// - MenuButton-Types -
// --------------------

#define MENUBUTTON_MENUMODE_TIMED       ((sal_uInt16)0x0001)

// --------------
// - MenuButton -
// --------------

class VCL_DLLPUBLIC MenuButton : public PushButton
{
private:
    Rectangle       maFocusRect;
    Timer*          mpMenuTimer;
    PopupMenu*      mpOwnMenu;
    PopupMenu*      mpMenu;
    sal_uInt16          mnCurItemId;
    sal_uInt16          mnMenuMode;
    Link            maActivateHdl;
    Link            maSelectHdl;

    SAL_DLLPRIVATE void    ImplInitMenuButtonData();
    SAL_DLLPRIVATE void    ImplExecuteMenu();
    DECL_DLLPRIVATE_LINK(  ImplMenuTimeoutHdl, Timer* );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE         MenuButton( const MenuButton & );
    SAL_DLLPRIVATE         MenuButton& operator=( const MenuButton & );

protected:
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE void    ImplLoadRes( const ResId& rResId );

public:
    explicit        MenuButton( Window* pParent, WinBits nStyle = 0 );
    explicit        MenuButton( Window* pParent, const ResId& );
    virtual         ~MenuButton();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );

    virtual void    Activate();
    virtual void    Select();

    void            SetMenuMode( sal_uInt16 nMode );
    sal_uInt16          GetMenuMode() const { return mnMenuMode; }

    void            SetPopupMenu( PopupMenu* pNewMenu );
    PopupMenu*      GetPopupMenu() const { return mpMenu; }

    sal_uInt16          GetCurItemId() const { return mnCurItemId; }

    void            SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&     GetActivateHdl() const              { return maActivateHdl; }
    void            SetSelectHdl( const Link& rLink )   { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const                { return maSelectHdl; }
};

#endif  // _SV_MENUBTN_HXX

