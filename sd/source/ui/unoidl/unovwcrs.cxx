/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: unovwcrs.cxx,v $
 * $Revision: 1.10 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/view/XScreenCursor.hpp>
#include <sfx2/request.hxx>
#include <vos/mutex.hxx>
#include "View.hxx"
#ifndef SVX_LIGHT
#ifndef SD_DRAW_DOC_SHELL_HXX
#include "DrawDocShell.hxx"
#endif
#endif
#include "ViewShell.hxx"
#include "slideshow.hxx"

#include <cppuhelper/implbase2.hxx>
#include <vcl/svapp.hxx>

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
    DBG_WARNING("not implemented");
    return sal_True;
}

void SdXTextViewCursor::setVisible(sal_Bool ) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

awt::Point SdXTextViewCursor::getPosition(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return awt::Point();
}

void SdXTextViewCursor::collapseToStart(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SdXTextViewCursor::collapseToEnd(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

sal_Bool SdXTextViewCursor::isCollapsed(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return sal_True;

}

sal_Bool SdXTextViewCursor::goLeft(sal_Int16 , sal_Bool ) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return sal_False;
}

sal_Bool SdXTextViewCursor::goRight(sal_Int16, sal_Bool ) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return sal_False;
}

void SdXTextViewCursor::gotoRange(const uno::Reference< text::XTextRange > &, sal_Bool ) throw (::com::sun::star::uno::RuntimeException)
{
    DBG_WARNING("not implemented");
}

void SdXTextViewCursor::gotoStart(sal_Bool ) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SdXTextViewCursor::gotoEnd(sal_Bool ) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}

sal_Bool SdXTextViewCursor::screenDown(void) throw( uno::RuntimeException )
{
    OGuard aGuard(Application::GetSolarMutex());
    return sal_False;
}

sal_Bool SdXTextViewCursor::screenUp(void) throw( uno::RuntimeException )
{
    OGuard aGuard(Application::GetSolarMutex());
    return sal_False;
}

uno::Reference< text::XText >  SdXTextViewCursor::getText(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return uno::Reference< text::XText > ();
}

uno::Reference< text::XTextRange >  SdXTextViewCursor::getStart(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return uno::Reference< text::XTextRange > ();
}

uno::Reference< text::XTextRange >  SdXTextViewCursor::getEnd(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return uno::Reference< text::XTextRange > ();
}

OUString SdXTextViewCursor::getString(void) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
    return OUString();
}

void SdXTextViewCursor::setString(const OUString& ) throw( uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}


