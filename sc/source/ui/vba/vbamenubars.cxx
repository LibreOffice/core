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
#include <cppuhelper/implbase.hxx>
#include <ooo/vba/excel/XlSheetType.hpp>
#include <ooo/vba/XCommandBars.hpp>

using namespace com::sun::star;
using namespace ooo::vba;

class MenuBarEnumeration : public ::cppu::WeakImplHelper< container::XEnumeration >
{
    uno::Reference< XHelperInterface > m_xParent;
    uno::Reference< uno::XComponentContext > m_xContext;
    uno::Reference< container::XEnumeration > m_xEnumeration;
public:
    /// @throws uno::RuntimeException
    MenuBarEnumeration( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration) : m_xParent( xParent ), m_xContext( xContext ), m_xEnumeration( xEnumeration )
    {
    }
    virtual sal_Bool SAL_CALL hasMoreElements() override
    {
        return m_xEnumeration->hasMoreElements();
    }
    virtual uno::Any SAL_CALL nextElement() override
    {
        // FIXME: should be add menubar
        if( !hasMoreElements() )
            throw container::NoSuchElementException();

        uno::Reference< XCommandBar > xCommandBar( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        uno::Reference< excel::XMenuBar > xMenuBar( new ScVbaMenuBar( m_xParent, m_xContext, xCommandBar ) );
        return uno::makeAny( xMenuBar );
    }
};

ScVbaMenuBars::ScVbaMenuBars( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< XCommandBars >& xCommandBars ) : MenuBars_BASE( xParent, xContext, uno::Reference< container::XIndexAccess>() ), m_xCommandBars( xCommandBars )
{
}

ScVbaMenuBars::~ScVbaMenuBars()
{
}

// XEnumerationAccess
uno::Type SAL_CALL
ScVbaMenuBars::getElementType()
{
    return cppu::UnoType<excel::XMenuBar>::get();
}

uno::Reference< container::XEnumeration >
ScVbaMenuBars::createEnumeration()
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
ScVbaMenuBars::getCount()
{
    return m_xCommandBars->getCount();
}

// ScVbaCollectionBaseImpl
uno::Any SAL_CALL
ScVbaMenuBars::Item( const uno::Any& aIndex, const uno::Any& /*aIndex2*/ )
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

    throw uno::RuntimeException("Not implemented" );
}

// XHelperInterface
OUString
ScVbaMenuBars::getServiceImplName()
{
    return "ScVbaMenuBars";
}

uno::Sequence<OUString>
ScVbaMenuBars::getServiceNames()
{
    static uno::Sequence< OUString > const aServiceNames
    {
        "ooo.vba.excel.MenuBars"
    };
    return aServiceNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
