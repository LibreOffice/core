/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: PresenterTextView.hxx,v $
 *
 * $Revision: 1.3 $
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

#ifndef SD_PRESENTER_TEXT_VIEW_HXX
#define SD_PRESENTER_TEXT_VIEW_HXX

#include "tools/PropertySet.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/implbase1.hxx>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>

namespace css = ::com::sun::star;

namespace sd { namespace presenter {

namespace {
    typedef ::cppu::ImplInheritanceHelper1 <
        tools::PropertySet,
        css::lang::XInitialization
    > PresenterTextViewInterfaceBase;
}


/** Render text into bitmaps.  An edit engine is used to render the text.
    This service is used by the presenter screen to render the notes view.
*/
class PresenterTextView
    : private ::boost::noncopyable,
      public PresenterTextViewInterfaceBase
{
public:
    explicit PresenterTextView (const css::uno::Reference<css::uno::XComponentContext>& rxContext);
    virtual ~PresenterTextView (void);

    // XInitialization

    virtual void SAL_CALL initialize (const css::uno::Sequence<css::uno::Any>& rArguments)
        throw (css::uno::Exception, css::uno::RuntimeException);


protected:
    virtual void SAL_CALL disposing (void);

    virtual css::uno::Any GetPropertyValue (
        const ::rtl::OUString& rsPropertyName);
    virtual css::uno::Any SetPropertyValue (
        const ::rtl::OUString& rsPropertyName,
        const css::uno::Any& rValue);

private:
    class Implementation;
    ::boost::scoped_ptr<Implementation> mpImplementation;

    /** This method throws a DisposedException when the object has already been
        disposed.
    */
    void ThrowIfDisposed (void) throw (css::lang::DisposedException);
};

} } // end of namespace ::sd::presenter

#endif
