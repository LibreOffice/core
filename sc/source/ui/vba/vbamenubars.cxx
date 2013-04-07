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

    throw uno::RuntimeException( OUString("Not implemented"), uno::Reference< uno::XInterface >() );
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
