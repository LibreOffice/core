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
#ifndef SW_VBA_OPTIONS_HXX
#define SW_VBA_OPTIONS_HXX

#include <ooo/vba/word/XOptions.hpp>
#include <vbahelper/vbahelperinterface.hxx>
#include <cppuhelper/implbase1.hxx>
#include <vbahelper/vbapropvalue.hxx>
#include <comphelper/processfactory.hxx>

typedef InheritedHelperInterfaceImpl1< ooo::vba::word::XOptions > SwVbaOptions_BASE;

class SwVbaOptions : public SwVbaOptions_BASE,
                    public PropListener
{
private:
    rtl::OUString msDefaultFilePath;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxFactory;
public:
    SwVbaOptions( css::uno::Reference< css::uno::XComponentContext >& m_xContext ) throw ( css::uno::RuntimeException );
    virtual ~SwVbaOptions();

    // Attributes
    virtual ::sal_Int32 SAL_CALL getDefaultBorderLineStyle() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDefaultBorderLineStyle( ::sal_Int32 _defaultborderlinestyle ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getDefaultBorderLineWidth() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDefaultBorderLineWidth( ::sal_Int32 _defaultborderlinewidth ) throw (css::uno::RuntimeException);
    virtual ::sal_Int32 SAL_CALL getDefaultBorderColorIndex() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setDefaultBorderColorIndex( ::sal_Int32 _defaultbordercolorindex ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getReplaceSelection() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setReplaceSelection( ::sal_Bool _replaceselection ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getMapPaperSize() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setMapPaperSize( ::sal_Bool _mappapersize ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatAsYouTypeApplyHeadings() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatAsYouTypeApplyHeadings( ::sal_Bool _autoformatasyoutypeapplyheadings ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatAsYouTypeApplyBulletedLists() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatAsYouTypeApplyBulletedLists( ::sal_Bool _autoformatasyoutypeapplybulletedlists ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatAsYouTypeApplyNumberedLists() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatAsYouTypeApplyNumberedLists( ::sal_Bool _autoformatasyoutypeapplynumberedlists ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatAsYouTypeFormatListItemBeginning() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatAsYouTypeFormatListItemBeginning( ::sal_Bool _autoformatasyoutypeformatlistitembeginning ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatAsYouTypeDefineStyles() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatAsYouTypeDefineStyles( ::sal_Bool _autoformatasyoutypedefinestyles ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatApplyHeadings() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatApplyHeadings( ::sal_Bool _autoformatapplyheadings ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatApplyLists() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatApplyLists( ::sal_Bool _autoformatapplylists ) throw (css::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL getAutoFormatApplyBulletedLists() throw (css::uno::RuntimeException);
    virtual void SAL_CALL setAutoFormatApplyBulletedLists( ::sal_Bool _autoformatapplybulletedlists ) throw (css::uno::RuntimeException);

    // Methods
    virtual css::uno::Any SAL_CALL DefaultFilePath( sal_Int32 _path ) throw ( css::uno::RuntimeException );

    //PropListener
    virtual void setValueEvent( const css::uno::Any& value );
    virtual css::uno::Any getValueEvent();

    // XHelperInterface
    virtual rtl::OUString& getServiceImplName();
    virtual css::uno::Sequence<rtl::OUString> getServiceNames();
};
#endif /* SW_VBA_OPTIONS_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
