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

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XTextDocument.hpp>

class SwVbaListHelper;
typedef std::shared_ptr< SwVbaListHelper > SwVbaListHelperRef;

class SwVbaListHelper
{
private:
    css::uno::Reference< css::text::XTextDocument > mxTextDocument;
    css::uno::Reference< css::container::XIndexReplace > mxNumberingRules;
    css::uno::Reference< css::container::XNameContainer > mxStyleFamily;
    css::uno::Reference< css::beans::XPropertySet > mxStyleProps;
    sal_Int32 mnGalleryType;
    sal_Int32 mnTemplateType;
    OUString msStyleName;

    void Init() throw( css::uno::RuntimeException );
    void CreateListTemplate() throw( css::uno::RuntimeException );
    void CreateBulletListTemplate() throw( css::uno::RuntimeException );
    void CreateNumberListTemplate() throw( css::uno::RuntimeException );
    void CreateOutlineNumberListTemplate() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType1() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType2() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType3() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType4() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType5() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType6() throw( css::uno::RuntimeException );
    void CreateOutlineNumberForType7() throw( css::uno::RuntimeException );

public:
    SwVbaListHelper( const css::uno::Reference< css::text::XTextDocument >& xTextDoc, sal_Int32 nGalleryType, sal_Int32 nTemplateType ) throw( css::uno::RuntimeException );

    sal_Int32 getGalleryType() const { return mnGalleryType; }
    css::uno::Reference< css::container::XIndexReplace > getNumberingRules() const { return mxNumberingRules; }
    css::uno::Any getPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName ) throw( css::uno::RuntimeException );
    void setPropertyValueWithNameAndLevel( sal_Int32 nLevel, const OUString& sName, const css::uno::Any& aValue ) throw( css::uno::RuntimeException );

};

#endif // INCLUDED_SW_SOURCE_UI_VBA_VBALISTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
