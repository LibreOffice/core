/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBALISTHELPER_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBALISTHELPER_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

#include <memory>

class SwVbaListHelper;
typedef std::shared_ptr< SwVbaListHelper > SwVbaListHelperRef;

class SwVbaListHelper
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::container::XIndexReplace > mxNumberingRules;
    css::uno::Reference< css::container::XNameContainer > mxStyleFamily;
    css::uno::Reference< css::beans::XPropertySet > mxStyleProps;
    sal_Int32 const mnGalleryType;
    sal_Int32 const mnTemplateType;
    OUString msStyleName;

    /// @throws css::uno::RuntimeException
    void Init();
    /// @throws css::uno::RuntimeException
    void CreateListTemplate();
    /// @throws css::uno::RuntimeException
    void CreateBulletListTemplate();
    /// @throws css::uno::RuntimeException
    void CreateNumberListTemplate();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberListTemplate();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType1();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType2();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType3();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType4();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType5();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType6();
    /// @throws css::uno::RuntimeException
    void CreateOutlineNumberForType7();

public:
    /// @throws css::uno::RuntimeException
    SwVbaListHelper( const css::uno::Reference< css::text::XTextDocument >& xTextDoc, sal_Int32 nGalleryType, sal_Int32 nTemplateType );

    sal_Int32 getGalleryType() const { return mnGalleryType; }
    const css::uno::Reference< css::container::XIndexReplace >& getNumberingRules() const { return mxNumberingRules; }
    /// @throws css::uno::RuntimeException
    css::uno::Any getPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName );
    /// @throws css::uno::RuntimeException
    void setPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName, const css::uno::Any& aValue );

};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBALISTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
