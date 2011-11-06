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



#ifndef ADDRCONT_HXX
#define ADDRCONT_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <vcl/splitwin.hxx>
#include <vcl/timer.hxx>
#include "bibshortcuthandler.hxx"

#include "bibmod.hxx"

#define TOP_WINDOW                          1
#define BOTTOM_WINDOW                       2

class BibDataManager;

class BibWindowContainer : public BibWindow     //Window
{
    private:
        // !BibShortCutHandler is also always a Window!
        BibShortCutHandler*     pChild;

    protected:
        virtual void            Resize();

    public:
        BibWindowContainer( Window* pParent, BibShortCutHandler* pChild, WinBits nStyle = WB_3DLOOK);
        ~BibWindowContainer();

        inline Window*          GetChild();

        virtual void            GetFocus();

        virtual sal_Bool            HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled

        using Window::GetChild;
};

inline Window* BibWindowContainer::GetChild()
{
    return pChild? pChild->GetWindow() : NULL;
}


class BibBookContainer: public BibSplitWindow
{
    private:

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             xTopFrameRef;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >             xBottomFrameRef;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              xTopPeerRef;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              xBottomPeerRef;

        BibDataManager*         pDatMan;
        BibWindowContainer*     pTopWin;
        BibWindowContainer*     pBottomWin;
        sal_Bool                    bFirstTime;
        HdlBibModul             pBibMod;
        Timer                   aTimer;

        DECL_LINK( SplitHdl, Timer*);

    protected:

        virtual void            Split();

        virtual long            PreNotify( NotifyEvent& rNEvt );

    public:

        BibBookContainer(Window* pParent,BibDataManager*, WinBits nStyle = WB_3DLOOK );
        ~BibBookContainer();

        inline BibWindow*       GetTopWin() {return pTopWin;}
        inline BibWindow*       GetBottomWin() {return pBottomWin;}

        // !BibShortCutHandler is also always a Window!
        void                    createTopFrame( BibShortCutHandler* pWin );

        void                    createBottomFrame( BibShortCutHandler* pWin );

        virtual void            GetFocus();

        virtual sal_Bool        HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled
};

#endif
