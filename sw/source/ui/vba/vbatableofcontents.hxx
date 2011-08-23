/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile:
 * $Revision:
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
#ifndef SW_VBA_TABLEOFCONTENTS_HXX
#define SW_VBA_TABLEOFCONTENTS_HXX

#include <ooo/vba/word/XTableOfContents.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XDocumentIndex.hpp>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XTableOfContents > SwVbaTableOfContents_BASE;

class SwVbaTableOfContents : public SwVbaTableOfContents_BASE
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::text::XDocumentIndex > mxDocumentIndex;
    css::uno::Reference< css::beans::XPropertySet > mxTocProps;

public:
    SwVbaTableOfContents( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, const css::uno::Reference< css::text::XTextDocument >& xDoc, const css::uno::Reference< css::text::XDocumentIndex >& xDocumentIndex ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaTableOfContents();

    // Attributes
    virtual ::sal_Int32 SAL_CALL getLowerHeadingLevel() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLowerHeadingLevel( ::sal_Int32 _lowerheadinglevel ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTabLeader() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTabLeader( ::sal_Int32 _tableader ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getUseFields() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUseFields( ::sal_Bool _useFields ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getUseOutlineLevels() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setUseOutlineLevels( ::sal_Bool _useOutlineLevels ) throw (css::uno::RuntimeException);

    // Methods
    virtual void SAL_CALL Delete(  ) throw (css::uno::RuntimeException);
    virtual void SAL_CALL Update(  ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_TABLEOFCONTENTS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
