/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshow.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:09:25 $
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

#ifndef _SD_SLIDESHOW_HXX
#define _SD_SLIDESHOW_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#include <memory>

namespace com { namespace sun { namespace star {

    namespace drawing {
        class XDrawPage;
    }
    namespace animations {
        class XAnimationNode;
    }
} } }

class SdDrawDocument;
class KeyEvent;
class HelpEvent;
class MouseEvent;
class Size;
class CommandEvent;
class Rectangle;
class Window;
class SfxRequest;

// TODO: Remove
#define PAGE_NO_END         65535
#define PAGE_NO_SOFTEND     (PAGE_NO_END - 1)
#define PAGE_NO_PAUSE       (PAGE_NO_SOFTEND - 1)
#define PAGE_NO_FIRSTDEF    PAGE_NO_PAUSE // immer mit anpassen

/* Definition of Slideshow class */

namespace sd
{

class SlideshowImpl;
class ShowWindow;
class Window;
class View;
class ViewShell;
struct PresentationSettings;
class EffectSequenceHelper;

enum AnimationMode
{
    ANIMATIONMODE_SHOW,
    ANIMATIONMODE_VIEW,
    ANIMATIONMODE_PREVIEW
};

class Slideshow
{
public:
    Slideshow( ViewShell* pViewSh, ::sd::View* pView, SdDrawDocument* pDoc,
        ::Window* pParentWindow );
    ~Slideshow();

    // actions
    bool startShow( PresentationSettings* pPresSettings = 0 );  // a.k.a. FuSlideShow::StartShow()
    bool startPreview(
        const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& xDrawPage,
        const ::com::sun::star::uno::Reference< ::com::sun::star::animations::XAnimationNode >& xAnimationNode,
        ::Window* pParent = 0 );


    void stopShow();                                        // a.k.a. FuSlideShow::Terminate()
    void jumpToPageNumber( sal_Int32 nPage );               // a.k.a. FuSlideShow::JumpToPage()
    void jumpToPageIndex( sal_Int32 nIndex );
    void jumpToBookmark( const String& sBookmark );         // a.k.a. FuSlideShow::JumpToBookmark()
    void dispose();                                         // a.k.a. FuSlideShow::Destroy()

    /** sets or clears the pause state of the running slideshow.
        !!!! This should only be called by the SdShowWindow !!!!*/
    bool pause( bool bPause );

    // settings
    void setRehearseTimings( bool bRehearseTimings );
    bool isFullScreen();                                // a.k.a. FuSlideShow::IsFullScreen()
    bool isTerminated();                                // a.k.a. FuSlideShow::IsTerminated();
    bool isAlwaysOnTop();                               // a.k.a. FuSlideShow::IsAlwaysOnTop();
    ShowWindow* getShowWindow();                        // a.k.a. FuSlideShow::GetShowWindow()
    int getAnimationMode();                             // a.k.a. FuSlideShow::GetAnimationMode()
    sal_Int32 getCurrentPageNumber();                   // a.k.a. FuSlideShow::GetCurrentPage()
    sal_Int32 getCurrentPageIndex();
    sal_Int32 getFirstPageNumber();
    sal_Int32 getLastPageNumber();
    bool isEndless();
    bool isDrawingPossible();

    // methods
    void setWindow( sd::Window* pWindow );              // a.k.a. FuSlideShow::SetWindow();

    // events
    void paint( const Rectangle& rRect );
    void resize( const Size &rSize );                   // a.k.a. FuSlideShow::Resize()
    void activate();                                    // a.k.a. FuSlideShow::Activate();
    void deactivate();                                  // a.k.a. FuSlideShow::Deactivate();

    bool requestHelp(const HelpEvent& rHEvt);           // a.k.a. FuSlideShow::requestHelp();
    bool keyInput(const KeyEvent& rKEvt);               // a.k.a. FuSlideShow::KeyInput();
    void mouseButtonDown(const MouseEvent& rMEvt);      // a.k.a. FuSlideShow::MouseButtonDown();
    void mouseMove(const MouseEvent& rMEvt);            // a.k.a. FuSlideShow::MouseMove();
    void mouseButtonUp(const MouseEvent& rMEvt);        // a.k.a. FuSlideShow::MouseButtonUp();

    void command(const CommandEvent& rCEvt);            // a.k.a. FuSlideShow::Command();

    void receiveRequest(SfxRequest& rReq);

private:
    // default: disabled copy/assignment
    Slideshow(const Slideshow&);
    Slideshow& operator=( const Slideshow& );

    sd::SlideshowImpl* mpImpl;
};

}

#endif /* _SD_SLIDESHOW_HXX */
