/*************************************************************************
 *
 *  $RCSfile: bibcont.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:44 $
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
#include "bibmod.hxx"

#define TOP_WINDOW                          1
#define BOTTOM_WINDOW                       2

class BibDataManager;

class BibWindowContainer: public DockingWindow
{
    private:
        Window*                 pChild;

    protected:
        virtual void            Resize();

    public:
        BibWindowContainer( Window* pParent,WinBits nStyle = 0);
        BibWindowContainer( Window* pParent,Window* pChild, WinBits nStyle = 0 );
        ~BibWindowContainer();

        Window*                 GetChild(){return pChild;}
        void                    SetChild(Window* pWin);
};


class BibBookContainer: public SplitWindow
{
    private:

        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 xTopFrameRef;
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame >                 xBottomFrameRef;

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              xTopPeerRef;
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >              xBottomPeerRef;
        sal_Bool                    bFirstTime;

        DockingWindow*          pTopWin;
        DockingWindow*          pBottomWin;
        BibDataManager*         pDatMan;
        HdlBibModul             pBibMod;
        Timer                   aTimer;

        DECL_LINK( SplitHdl, Timer*);

    protected:

        virtual void            Split();

        void                    CreateTopWin();
        void                    CreateBottomWin();
        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > GetTopComponentInterface( sal_Bool bCreate = sal_True );
        void                    SetTopComponentInterface( ::com::sun::star::awt::XWindowPeer* pIFace );

        ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > GetBottomComponentInterface( sal_Bool bCreate = sal_True );
        void                    SetBottomComponentInterface( ::com::sun::star::awt::XWindowPeer* pIFace );

//      VCLXWindow*             GetTopWindowPeer();
//      void                    SetTopWindowPeer( VCLXWindow* pPeer );

//      VCLXWindow*             GetBottomWindowPeer();
//      void                    SetBottomWindowPeer( VCLXWindow* pPeer );

    public:

        BibBookContainer(Window* pParent,BibDataManager*, WinBits nStyle = 0 );
        ~BibBookContainer();

        DockingWindow*          GetTopWin() {return pTopWin;}
        DockingWindow*          GetBottomWin() {return pBottomWin;}


        void                    createTopFrame(const rtl::OUString & rURL );

        void                    createBottomFrame(const rtl::OUString & rURL );

        void                    createTopFrame(Window* pWin);

        void                    createBottomFrame(Window* pWin);

};


#endif
