/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef SW_VBA_VIEW_HXX
#define SW_VBA_VIEW_HXX

#include <ooo/vba/word/XView.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/text/XTextViewCursor.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XView > SwVbaView_BASE;

class SwVbaView : public SwVbaView_BASE
{
private:
    css::uno::Reference< css::frame::XModel > mxModel;
    css::uno::Reference< css::text::XTextViewCursor > mxViewCursor;
    css::uno::Reference< css::beans::XPropertySet > mxViewSettings;

    css::uno::Reference< css::text::XTextRange > getHFTextRange( sal_Int32 nType ) throw (css::uno::RuntimeException);
    css::uno::Reference< css::text::XTextRange > getFirstObjectPosition( const css::uno::Reference< css::text::XText >& xText ) throw (css::uno::RuntimeException);

public:
    SwVbaView( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext,
        const css::uno::Reference< css::frame::XModel >& rModel ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaView();

   // XView
    virtual ::sal_Int32 SAL_CALL getSeekView() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSeekView( ::sal_Int32 _seekview ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getSplitSpecial() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setSplitSpecial( ::sal_Int32 _splitspecial ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getTableGridLines() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTableGridLines( ::sal_Bool _tablegridlines ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getType() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setType( ::sal_Int32 _type ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_VIEW_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
