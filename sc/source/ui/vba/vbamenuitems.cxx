/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "vbamenuitems.hxx"
#include "vbamenuitem.hxx"
#include "vbamenu.hxx"
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/office/MsoControlType.hpp>
#include <ooo/vba/XCommandBarControls.hpp>
#include <utility>

using namespace com::sun::star;
using namespace ooo::vba;

typedef ::cppu::WeakImplHelper< container::XEnumeration > MenuEnumeration_BASE;

namespace {

class MenuEnumeration : public MenuEnumeration_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< container::XEnumeration > m_xEnumeration;
public:
    /// @throws uno::RuntimeException
    MenuEnumeration( uno::Reference< XHelperInterface > xParent, uno::Reference< uno::XComponentContext > xContext, uno::Reference< container::XEnumeration > xEnumeration) : m_xParent(std::move( xParent )), m_xContext(std::move( xContext )), m_xEnumeration(std::move( xEnumeration ))
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        return m_xEnumeration->hasMoreElements();
    }
    virtual uno::Any SAL_CALL nextElement() override
    {
        // FIXME: should be add menu
        if( !hasMoreElements() )
            throw container::NoSuchElementException();

        uno::Reference< XCommandBarControl > xCommandBarControl( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        if( xCommandBarControl->getType() == office::MsoControlType::msoControlPopup )
        {
            uno::Reference< excel::XMenu > xMenu( new ScVbaMenu( m_xParent, m_xContext, xCommandBarControl ) );
            return uno::Any( xMenu );
        }
        else if( xCommandBarControl->getType() == office::MsoControlType::msoControlButton )
        {
            uno::Reference< excel::XMenuItem > xMenuItem( new ScVbaMenuItem( m_xParent, m_xContext, xCommandBarControl ) );
            return uno::Any( xMenuItem );
        }
        nextElement();

        return uno::Any();
    }
};

}

ScVbaMenuItems::ScVbaMenuItems( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, uno::Reference< XCommandBarControls > xCommandBarControls ) : MenuItems_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() ), m_xCommandBarControls(std::move( xCommandBarControls ))
{
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaMenuItems::getElementType()
{
    return cppu::UnoType<excel::XMenuItem>::get();
}

uno::Reference< container::XEnumeration >
ScVbaMenuItems::createEnumeration()
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xCommandBarControls, uno::UNO_QUERY_THROW );
    return uno::Reference< container::XEnumeration >( new MenuEnumeration( this, mxContext, xEnumAccess->createEnumeration() ) );
}

uno::Any
ScVbaMenuItems::createCollectionObject( const uno::Any& aSource )
{
    // make no sense
    return aSource;
}

sal_Int32 SAL_CALL
ScVbaMenuItems::getCount()
{
    // FIXME: should check if it is a popup menu
    return m_xCommandBarControls->getCount();
}

// ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaMenuItems::Item( const uno::Any& aIndex, const uno::Any& /*aIndex2*/ )
{
    uno::Reference< XCommandBarControl > xCommandBarControl( m_xCommandBarControls->Item( aIndex, uno::Any() ), uno::UNO_QUERY_THROW );
    if( xCommandBarControl->getType() == office::MsoControlType::msoControlPopup )
        return uno::Any( uno::Reference< excel::XMenu > ( new ScVbaMenu( this, mxContext, xCommandBarControl ) ) );
    else if( xCommandBarControl->getType() == office::MsoControlType::msoControlButton )
        return uno::Any( uno::Reference< excel::XMenuItem > ( new ScVbaMenuItem( this, mxContext, xCommandBarControl ) ) );
    throw uno::RuntimeException();
}

uno::Reference< excel::XMenuItem > SAL_CALL ScVbaMenuItems::Add( const OUString& Caption, const css::uno::Any& OnAction, const css::uno::Any& /*ShortcutKey*/, const css::uno::Any& Before, const css::uno::Any& Restore, const css::uno::Any& /*StatusBar*/, const css::uno::Any& /*HelpFile*/, const css::uno::Any& /*HelpContextID*/ )
{
    uno::Reference< XCommandBarControl > xCommandBarControl = m_xCommandBarControls->Add(
            uno::Any( office::MsoControlType::msoControlButton ),
            uno::Any(), uno::Any(), Before, Restore );
    xCommandBarControl->setCaption( Caption );
    if( OnAction.hasValue() )
    {
        OUString sAction;
        OnAction >>= sAction;
        xCommandBarControl->setOnAction( sAction );
    }
    return uno::Reference< excel::XMenuItem >( new ScVbaMenuItem( this, mxContext, xCommandBarControl ) );
}

// XHelperInterface
OUString
ScVbaMenuItems::getServiceImplName()
{
    return u"ScVbaMenuItems"_ustr;
}

uno::Sequence<OUString>
ScVbaMenuItems::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        u"ooo.vba.excel.MenuItems"_ustr
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
