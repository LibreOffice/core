/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Novell Inc.
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s): <pflin@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef SW_VBA_LISTHELPER_HXX
#define SW_VBA_LISTHELPER_HXX

#include <vbahelper/vbahelperinterface.hxx>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <boost/shared_ptr.hpp>


class SwVbaListHelper;
typedef ::boost::shared_ptr< SwVbaListHelper > SwVbaListHelperRef;

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

#endif//SW_VBA_LISTHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
