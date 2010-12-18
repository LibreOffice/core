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
#ifndef SW_VBA_LISTLEVEL_HXX
#define SW_VBA_LISTLEVEL_HXX

#include <ooo/vba/word/XListLevel.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/text/XTextDocument.hpp>
#include "vbalisthelper.hxx"


typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XListLevel > SwVbaListLevel_BASE;

class SwVbaListLevel : public SwVbaListLevel_BASE
{
private:
    SwVbaListHelperRef pListHelper;
    sal_Int32 mnLevel;

public:
    SwVbaListLevel( const css::uno::Reference< ooo::vba::XHelperInterface >& rParent, const css::uno::Reference< css::uno::XComponentContext >& rContext, SwVbaListHelperRef pHelper, sal_Int32 nLevel ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaListLevel();

    // Attributes
    virtual ::sal_Int32 SAL_CALL getAlignment() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAlignment( ::sal_Int32 _alignment ) throw (css::uno::RuntimeException);
    virtual css::uno::Reference< ::ooo::vba::word::XFont > SAL_CALL getFont() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setFont( const css::uno::Reference< ::ooo::vba::word::XFont >& _font ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getIndex() throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getLinkedStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setLinkedStyle( const ::rtl::OUString& _linkedstyle ) throw (css::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getNumberFormat() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNumberFormat( const ::rtl::OUString& _numberformat ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getNumberPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNumberPosition( float _numberposition ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getNumberStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setNumberStyle( ::sal_Int32 _numberstyle ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getResetOnHigher() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setResetOnHigher( ::sal_Int32 _resetonhigher ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getStartAt() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setStartAt( ::sal_Int32 _startat ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getTabPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTabPosition( float _tabposition ) throw (css::uno::RuntimeException);
    virtual float SAL_CALL getTextPosition() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTextPosition( float _textposition ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getTrailingCharacter() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setTrailingCharacter( ::sal_Int32 _trailingcharacter ) throw (css::uno::RuntimeException);

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_LISTLEVEL_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
