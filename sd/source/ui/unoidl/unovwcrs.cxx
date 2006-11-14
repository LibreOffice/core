/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unovwcrs.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 14:39:34 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#ifndef _COM_SUN_STAR_TEXT_XTEXTVIEWCURSOR_HPP_
#include <com/sun/star/text/XTextViewCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSCREENCURSOR_HPP_
#include <com/sun/star/view/XScreenCursor.hpp>
#endif

#ifndef _SFXREQUEST_HXX
#include <sfx2/request.hxx>
#endif

#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif

#ifndef SD_VIEW_HXX
#include "View.hxx"
#endif
#ifndef SVX_LIGHT
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#endif
#ifndef SD_VIEW_SHELL_HXX
#include "ViewShell.hxx"
#endif
#ifndef _SD_SLIDESHOW_HXX
#include "slideshow.hxx"
#endif

#include <cppuhelper/implbase2.hxx>

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif

using namespace ::vos;
using namespace ::rtl;
using namespace ::com::sun::star;

class SdXTextViewCursor
    : public ::cppu::WeakImplHelper2<
    text::XTextViewCursor,
    view::XScreenCursor >
{
public:
    SdXTextViewCursor(::sd::View* pVw) throw();
    virtual ~SdXTextViewCursor() throw();

    //XTextViewCursor
    virtual sal_Bool SAL_CALL isVisible(void) throw( uno::RuntimeException );
    virtual void SAL_CALL setVisible(sal_Bool bVisible) throw( uno::RuntimeException );
    virtual awt::Point SAL_CALL getPosition(void) throw( uno::RuntimeException );

    //XTextCursor
    virtual void SAL_CALL collapseToStart(void) throw( uno::RuntimeException );
    virtual void SAL_CALL collapseToEnd(void) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL isCollapsed(void) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL goLeft(sal_Int16 nCount, sal_Bool Expand) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL goRight(sal_Int16 nCount, sal_Bool Expand) throw( uno::RuntimeException );
    virtual void SAL_CALL gotoStart(sal_Bool Expand) throw( uno::RuntimeException );
    virtual void SAL_CALL gotoEnd(sal_Bool Expand) throw( uno::RuntimeException );
    virtual void SAL_CALL gotoRange(const uno::Reference< text::XTextRange > & rRange, sal_Bool bExpand ) throw (::com::sun::star::uno::RuntimeException);

    //XTextRange
    virtual uno::Reference< text::XText >  SAL_CALL getText(void) throw( uno::RuntimeException );
    virtual uno::Reference< text::XTextRange >  SAL_CALL getStart(void) throw( uno::RuntimeException );
    virtual uno::Reference< text::XTextRange >  SAL_CALL getEnd(void) throw( uno::RuntimeException );
    virtual OUString SAL_CALL getString(void) throw( uno::RuntimeException );
    virtual void SAL_CALL setString(const OUString& aString) throw( uno::RuntimeException );

    //XScreenCursor
    virtual sal_Bool SAL_CALL screenDown(void) throw( uno::RuntimeException );
    virtual sal_Bool SAL_CALL screenUp(void) throw( uno::RuntimeException );

    void    Invalidate()    { mpView = 0; }

private:
    ::sd::View* mpView;
};


text::XTextViewCursor* CreateSdXTextViewCursor(::sd::View* mpView )
{
    return new SdXTextViewCursor( mpView );
}

SdXTextViewCursor::SdXTextViewCursor(::sd::View* pSdView ) throw()
    :   mpView(pSdView)
{

}

SdXTextViewCursor::~SdXTextViewCursor() throw()
{
}

sal_Bool SdXTextViewCursor::isVisible(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return sal_True;
}

void SdXTextViewCursor::setVisible(sal_Bool bVisible) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

awt::Point SdXTextViewCursor::getPosition(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return awt::Point();
}

void SdXTextViewCursor::collapseToStart(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

void SdXTextViewCursor::collapseToEnd(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

sal_Bool SdXTextViewCursor::isCollapsed(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return sal_True;

}

sal_Bool SdXTextViewCursor::goLeft(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return sal_False;
}

sal_Bool SdXTextViewCursor::goRight(sal_Int16 nCount, sal_Bool bExpand) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return sal_False;
}

void SdXTextViewCursor::gotoRange(const uno::Reference< text::XTextRange > & xRange, sal_Bool bExpand) throw (::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING("not implemented")
}

void SdXTextViewCursor::gotoStart(sal_Bool bExpand) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

void SdXTextViewCursor::gotoEnd(sal_Bool bExpand) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}

sal_Bool SdXTextViewCursor::screenDown(void) throw( uno::RuntimeException )
{
    OGuard aGuard(Application::GetSolarMutex());
    sal_Bool bRet = sal_False;

/*

    if( mpView && mpView->GetDocSh() )
    {
        ::sd::ViewShell* pViewSh = mpView->GetDocSh()->GetViewShell();
        if( pViewSh )
        {
            ::sd::FuSlideShow* pShow = pViewSh->GetSlideShow();
            if( pShow )
            {
                pShow->KeyInput( KeyEvent( 32, KeyCode( KEY_SPACE ) ) );
                return sal_True;
            }
        }
    }
*/
    return sal_False;
}

sal_Bool SdXTextViewCursor::screenUp(void) throw( uno::RuntimeException )
{
    OGuard aGuard(Application::GetSolarMutex());
/*
    sal_Bool bRet = sal_False;

    if( mpView && mpView->GetDocSh() )
    {
        ::sd::ViewShell* pViewSh = mpView->GetDocSh()->GetViewShell();
        if( pViewSh )
        {
            ::sd::FuSlideShow* pShow = pViewSh->GetSlideShow();
            if( pShow )
            {
                pShow->KeyInput( KeyEvent( 32, KeyCode( KEY_BACKSPACE ) ) );
                return sal_True;
            }
        }
    }
*/
    return sal_False;
}

uno::Reference< text::XText >  SdXTextViewCursor::getText(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return uno::Reference< text::XText > ();
}

uno::Reference< text::XTextRange >  SdXTextViewCursor::getStart(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return uno::Reference< text::XTextRange > ();
}

uno::Reference< text::XTextRange >  SdXTextViewCursor::getEnd(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return uno::Reference< text::XTextRange > ();
}

OUString SdXTextViewCursor::getString(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
    return OUString();
}

void SdXTextViewCursor::setString(const OUString& aString) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented")
}


