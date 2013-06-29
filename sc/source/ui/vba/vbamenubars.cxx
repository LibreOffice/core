/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include "vbamenubars.hxx"
#include "vbamenubar.hxx"
#include <ooo/vba/excel/XlSheetType.hpp>

using namespace com::sun::star;
using namespace ooo::vba;


typedef ::cppu::WeakImplHelper1< container::XEnumeration > MenuBarEnumeration_BASE;

class MenuBarEnumeration : public MenuBarEnumeration_BASE
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< container::XEnumeration > m_xEnumeration;
public:
    MenuBarEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration) throw ( uno::RuntimeException ) : m_xParent( xParent ), m_xContext( xContext ), m_xEnumeration( xEnumeration )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements() throw ( uno::RuntimeException )
    {
        return m_xEnumeration->hasMoreElements();
    }
    virtual uno::Any SAL_CALL nextElement() throw ( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
    {
        // FIXME: should be add menubar
        if( hasMoreElements() )
        {
            uno::Reference< XCommandBar > xCommandBar( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
            uno::Reference< excel::XMenuBar > xMenuBar( new ScVbaMenuBar( m_xParent, m_xContext, xCommandBar ) );
            return uno::makeAny( xMenuBar );
        }
        else
            throw container::NoSuchElementException();
    }
};

ScVbaMenuBars::ScVbaMenuBars( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< XCommandBars >& xCommandBars ) throw ( uno::RuntimeException ) : MenuBars_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() ), m_xCommandBars( xCommandBars )
{
}

ScVbaMenuBars::~ScVbaMenuBars()
{
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaMenuBars::getElementType() throw ( uno::RuntimeException )
{
    return excel::XMenuBar::static_type( 0 );
}

uno::Reference< container::XEnumeration >
ScVbaMenuBars::createEnumeration() throw ( uno::RuntimeException )
{
    uno::Reference< container::XEnumerationAccess > xEnumAccess( m_xCommandBars, uno::UNO_QUERY_THROW );
    return uno::Reference< container::XEnumeration >( new MenuBarEnumeration( this, mxContext, xEnumAccess->createEnumeration() ) );
}

uno::Any
ScVbaMenuBars::createCollectionObject( const uno::Any& aSource )
{
    // make no sense
    return aSource;
}

sal_Int32 SAL_CALL
ScVbaMenuBars::getCount() throw(css::uno::RuntimeException)
{
    return m_xCommandBars->getCount();
}

// ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaMenuBars::Item( const uno::Any& aIndex, const uno::Any& /*aIndex2*/ ) throw( uno::RuntimeException )
{
    sal_Int16 nIndex = 0;
    aIndex >>= nIndex;
    if( nIndex == excel::XlSheetType::xlWorksheet )
    {
        uno::Any aSource;
        aSource <<= OUString( "Worksheet Menu Bar" );
        uno::Reference< XCommandBar > xCommandBar( m_xCommandBars->Item( aSource, uno::Any() ), uno::UNO_QUERY_THROW );
        uno::Reference< excel::XMenuBar > xMenuBar( new ScVbaMenuBar( this, mxContext, xCommandBar ) );
        return uno::makeAny( xMenuBar );
    }

    throw uno::RuntimeException("Not implemented", uno::Reference< uno::XInterface >() );
}

// XHelperInterface
OUString
ScVbaMenuBars::getServiceImplName()
{
    return OUString("ScVbaMenuBars");
}

uno::Sequence<OUString>
ScVbaMenuBars::getServiceNames()
{
    static uno::Sequence< OUString > aServiceNames;
    if ( aServiceNames.getLength() == 0 )
    {
        aServiceNames.realloc( 1 );
        aServiceNames[ 0 ] = OUString("ooo.vba.excel.MenuBars" );
    }
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
