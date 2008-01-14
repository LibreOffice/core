/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: bibcont.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:38:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef ADDRCONT_HXX
#define ADDRCONT_HXX

#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif

#ifndef _SV_SPLITWIN_HXX
#include <vcl/splitwin.hxx>
#endif

#ifndef _SV_TIMER_HXX //autogen wg. Timer
#include <vcl/timer.hxx>
#endif
#ifndef _BIBSHORTCUTHANDLER_HXX
#include "bibshortcuthandler.hxx"
#endif

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
        BibWindowContainer( Window* pParent, WinBits nStyle = WB_3DLOOK );
        BibWindowContainer( Window* pParent, BibShortCutHandler* pChild, WinBits nStyle = WB_3DLOOK);
        ~BibWindowContainer();

        inline Window*          GetChild();

        virtual void            GetFocus();

        virtual BOOL            HandleShortCutKey( const KeyEvent& rKeyEvent ); // returns true, if key was handled

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
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                                GetTopComponentInterface( sal_Bool bCreate = sal_True );
        void                    SetTopComponentInterface( ::com::sun::star::awt::XWindowPeer* pIFace );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > GetBottomComponentInterface( sal_Bool bCreate = sal_True );
        void                    SetBottomComponentInterface( ::com::sun::star::awt::XWindowPeer* pIFace );

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
