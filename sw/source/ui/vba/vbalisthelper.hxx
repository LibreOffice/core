/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile$
 *
 *  $Revision$
 *
 *  last change: $Author$ $Date$
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
    rtl::OUString msStyleName;

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

    sal_Int32 getGalleryType() { return mnGalleryType; }
    css::uno::Reference< css::container::XIndexReplace > getNumberingRules() { return mxNumberingRules; }
    css::uno::Any getPropertyValueWithNameAndLevel( sal_Int32 nLevel, const rtl::OUString& sName ) throw( css::uno::RuntimeException );
    void setPropertyValueWithNameAndLevel( sal_Int32 nLevel, const rtl::OUString& sName, const css::uno::Any& aValue ) throw( css::uno::RuntimeException );

};

#endif//SW_VBA_LISTHELPER_HXX
