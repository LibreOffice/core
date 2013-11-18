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



#ifndef _SIDEBARTXTCONTROL_HXX
#define _SIDEBARTXTCONTROL_HXX

#include <vcl/ctrl.hxx>

class OutlinerView;
class SwView;
class SwPostItMgr;
struct SpellCallbackInfo;

namespace sw { namespace sidebarwindows {

class SwSidebarWin;

class SidebarTxtControl : public Control
{
    private:
        SwSidebarWin& mrSidebarWin;
        SwView& mrDocView;
        SwPostItMgr& mrPostItMgr;

        bool mbMouseOver;

    protected:
        virtual void    Paint( const Rectangle& rRect);
        virtual void    KeyInput( const KeyEvent& rKeyEvt );
        virtual void    MouseMove( const MouseEvent& rMEvt );
        virtual void    MouseButtonDown( const MouseEvent& rMEvt );
        virtual void    MouseButtonUp( const MouseEvent& rMEvt );
        virtual void    Command( const CommandEvent& rCEvt );
        virtual void    LoseFocus();
        virtual void    RequestHelp(const HelpEvent &rEvt);
        virtual XubString GetSurroundingText() const;
        virtual Selection GetSurroundingTextSelection() const;

        DECL_LINK( Select, Menu* );

    public:
        SidebarTxtControl( SwSidebarWin& rSidebarWin,
                           WinBits nBits,
                           SwView& rDocView,
                           SwPostItMgr& rPostItMgr );
        ~SidebarTxtControl();

        virtual void GetFocus();

        OutlinerView* GetTextView() const;

        DECL_LINK( OnlineSpellCallback, SpellCallbackInfo*);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
};

} } // end of namespace sw::sidebarwindows

#endif
