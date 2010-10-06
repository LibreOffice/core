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
#include "vbamenus.hxx"
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
            nextElement();
        }
        else
            throw container::NoSuchElementException();
        return uno::Any();
    }
};

ScVbaMenus::ScVbaMenus( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< XCommandBarControls >& xCommandBarControls ) throw ( uno::RuntimeException ) : Menus_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() ), m_xCommandBarControls( xCommandBarControls )
{
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaMenus::getElementType() throw ( uno::RuntimeException )
{
    return excel::XMenu::static_type( 0 );
}

uno::Reference< container::XEnumeration >
ScVbaMenus::createEnumeration() throw ( uno::RuntimeException )
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xCommandBarControls, uno::UNO_QUERY_THROW );
    return uno::Reference< container::XEnumeration >( new MenuEnumeration( this, mxContext, xEnumAccess->createEnumeration() ) );
}

uno::Any
ScVbaMenus::createCollectionObject( const uno::Any& aSource )
{
    // make no sense
    return aSource;
}

sal_Int32 SAL_CALL
ScVbaMenus::getCount() throw(css::uno::RuntimeException)
{
    // FIXME: should check if it is a popup menu
    return m_xCommandBarControls->getCount();
}

// ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaMenus::Item( const uno::Any& aIndex, const uno::Any& /*aIndex2*/ ) throw( uno::RuntimeException )
{
    uno::Reference< XCommandBarControl > xCommandBarControl( m_xCommandBarControls->Item( aIndex, uno::Any() ), uno::UNO_QUERY_THROW );
    if( xCommandBarControl->getType() != office::MsoControlType::msoControlPopup )
        throw uno::RuntimeException();
    return uno::makeAny( uno::Reference< excel::XMenu > ( new ScVbaMenu( this, mxContext, xCommandBarControl ) ) );
}

uno::Reference< excel::XMenu > SAL_CALL ScVbaMenus::Add( const rtl::OUString& Caption, const css::uno::Any& Before, const css::uno::Any& Restore ) throw (css::script::BasicErrorException, css::uno::RuntimeException)
{
    sal_Int32 nType = office::MsoControlType::msoControlPopup;
    uno::Reference< XCommandBarControl > xCommandBarControl = m_xCommandBarControls->Add( uno::makeAny( nType ), uno::Any(), uno::Any(), Before, Restore );
    xCommandBarControl->setCaption( Caption );
    return uno::Reference< excel::XMenu >( new ScVbaMenu( this, mxContext, xCommandBarControl ) );
}

// XHelperInterface
rtl::OUString&
ScVbaMenus::getServiceImplName()
{
    static rtl::OUString sImplName( RTL_CONSTASCII_USTRINGPARAM("ScVbaMenus") );
    return sImplName;
}
uno::Sequence<rtl::OUString>
ScVbaMenus::getServiceNames()
{
    static uno::Sequence< rtl::OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("ooo.vba.excel.Menus" ) );
    }
    return aServiceNames;
}

