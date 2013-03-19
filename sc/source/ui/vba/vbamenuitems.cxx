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
 * Contributor(s): Pei Feng Lin <pflin@novell.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#include "vbamenuitems.hxx"
#include "vbamenuitem.hxx"
#include "vbamenu.hxx"
#include <ooo/vba/office/MsoControlType.hpp>

using namespace com::sun::star;
using namespace ooo::vba;


typedef ::cppu::WeakImplHelper1< container::XEnumeration > MenuEnumeration_BASE;

class MenuEnumeration : public MenuEnumeration_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< container::XEnumeration > m_xEnumeration;
public:
    MenuEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration) throw ( uno::RuntimeException ) : m_xParent( xParent ), m_xContext( xContext ), m_xEnumeration( xEnumeration )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements() throw ( uno::RuntimeException )
    {
        return m_xEnumeration->hasMoreElements();
    }
    virtual uno::Any SAL_CALL nextElement() throw ( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
    {
        // FIXME: should be add menu
        if( hasMoreElements() )
        {
            uno::Reference< XCommandBarControl > xCommandBarControl( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
            if( xCommandBarControl->getType() == office::MsoControlType::msoControlPopup )
            {
                uno::Reference< excel::XMenu > xMenu( new ScVbaMenu( m_xParent, m_xContext, xCommandBarControl ) );
                return uno::makeAny( xMenu );
            }
            else if( xCommandBarControl->getType() == office::MsoControlType::msoControlButton )
            {
                uno::Reference< excel::XMenuItem > xMenuItem( new ScVbaMenuItem( m_xParent, m_xContext, xCommandBarControl ) );
                return uno::makeAny( xMenuItem );
            }
            nextElement();
        }
        else
            throw container::NoSuchElementException();
        return uno::Any();
    }
};

ScVbaMenuItems::ScVbaMenuItems( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< XCommandBarControls >& xCommandBarControls ) throw ( uno::RuntimeException ) : MenuItems_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() ), m_xCommandBarControls( xCommandBarControls )
{
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaMenuItems::getElementType() throw ( uno::RuntimeException )
{
    return excel::XMenuItem::static_type( 0 );
}

uno::Reference< container::XEnumeration >
ScVbaMenuItems::createEnumeration() throw ( uno::RuntimeException )
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
ScVbaMenuItems::getCount() throw(css::uno::RuntimeException)
{
    // FIXME: should check if it is a popup menu
    return m_xCommandBarControls->getCount();
}

// ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaMenuItems::Item( const uno::Any& aIndex, const uno::Any& /*aIndex2*/ ) throw( uno::RuntimeException )
{
    uno::Reference< XCommandBarControl > xCommandBarControl( m_xCommandBarControls->Item( aIndex, uno::Any() ), uno::UNO_QUERY_THROW );
    if( xCommandBarControl->getType() == office::MsoControlType::msoControlPopup )
        return uno::makeAny( uno::Reference< excel::XMenu > ( new ScVbaMenu( this, mxContext, xCommandBarControl ) ) );
    else if( xCommandBarControl->getType() == office::MsoControlType::msoControlButton )
        return uno::makeAny( uno::Reference< excel::XMenuItem > ( new ScVbaMenuItem( this, mxContext, xCommandBarControl ) ) );
    throw uno::RuntimeException();
}

uno::Reference< excel::XMenuItem > SAL_CALL ScVbaMenuItems::Add( const rtl::OUString& Caption, const css::uno::Any& OnAction, const css::uno::Any& /*ShortcutKey*/, const css::uno::Any& Before, const css::uno::Any& Restore, const css::uno::Any& /*StatusBar*/, const css::uno::Any& /*HelpFile*/, const css::uno::Any& /*HelpContextID*/ ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    sal_Int32 nType = office::MsoControlType::msoControlButton;
    uno::Reference< XCommandBarControl > xCommandBarControl = m_xCommandBarControls->Add( uno::makeAny( nType ), uno::Any(), uno::Any(), Before, Restore );
    xCommandBarControl->setCaption( Caption );
    if( OnAction.hasValue() )
    {
        rtl::OUString sAction;
        OnAction >>= sAction;
        xCommandBarControl->setOnAction( sAction );
    }
    return uno::Reference< excel::XMenuItem >( new ScVbaMenuItem( this, mxContext, xCommandBarControl ) );
}

// XHelperInterface
rtl::OUString
ScVbaMenuItems::getServiceImplName()
{
    return rtl::OUString("ScVbaMenuItems");
}

uno::Sequence<rtl::OUString>
ScVbaMenuItems::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString("ooo.vba.excel.MenuItems" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
