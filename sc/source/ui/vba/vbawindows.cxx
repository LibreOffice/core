/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#include "vbawindows.hxx"


#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <cppuhelper/implbase.hxx>

#include "vbawindow.hxx"
#include "vbaglobals.hxx"

#include <unordered_map>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

typedef  std::unordered_map< OUString,
sal_Int32, OUStringHash,
std::equal_to< OUString > > NameIndexHash;

static uno::Reference< XHelperInterface > lcl_createWorkbookHIParent( const uno::Reference< frame::XModel >& xModel, const uno::Reference< uno::XComponentContext >& xContext, const uno::Any& aApplication )
{
    return new ScVbaWorkbook( uno::Reference< XHelperInterface >( aApplication, uno::UNO_QUERY_THROW ), xContext,  xModel );
}

uno::Any ComponentToWindow( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext, const uno::Any& aApplication )
{
    uno::Reference< frame::XModel > xModel( aSource, uno::UNO_QUERY_THROW );
    // !! TODO !! iterate over all controllers
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    uno::Reference< excel::XWindow > xWin( new ScVbaWindow( lcl_createWorkbookHIParent( xModel, xContext, aApplication ), xContext, xModel, xController ) );
    return uno::makeAny( xWin );
}

typedef std::vector < uno::Reference< sheet::XSpreadsheetDocument > > Components;
// #TODO more or less the same as class in workwindows ( code sharing needed )
class WindowComponentEnumImpl : public EnumerationHelper_BASE
{
protected:
    uno::Reference< uno::XComponentContext > m_xContext;
    Components m_components;
    Components::const_iterator m_it;

public:
    WindowComponentEnumImpl( const uno::Reference< uno::XComponentContext >& xContext, const Components& components ) throw ( uno::RuntimeException ) :  m_xContext( xContext ), m_components( components )
    {
        m_it = m_components.begin();
    }

    WindowComponentEnumImpl( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException ) :  m_xContext( xContext )
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xContext);
        uno::Reference< container::XEnumeration > mxComponents = xDesktop->getComponents()->createEnumeration();
        while( mxComponents->hasMoreElements() )
        {
            uno::Reference< sheet::XSpreadsheetDocument > xNext( mxComponents->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
                m_components.push_back( xNext );
        }
        m_it = m_components.begin();
    }
    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return m_it != m_components.end();
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( !hasMoreElements() )
        {
            throw container::NoSuchElementException();
        }
        return makeAny( *(m_it++) );
    }
};

class WindowEnumImpl : public  WindowComponentEnumImpl
{
    uno::Any m_aApplication;
public:
    WindowEnumImpl( const uno::Reference< uno::XComponentContext >& xContext,  const uno::Any& aApplication ): WindowComponentEnumImpl( xContext ), m_aApplication( aApplication ) {}
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        return ComponentToWindow( WindowComponentEnumImpl::nextElement(), m_xContext, m_aApplication );
    }
};

typedef ::cppu::WeakImplHelper< container::XEnumerationAccess
    , com::sun::star::container::XIndexAccess
    , com::sun::star::container::XNameAccess
    > WindowsAccessImpl_BASE;

class WindowsAccessImpl : public WindowsAccessImpl_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    Components m_windows;
    NameIndexHash namesToIndices;
public:
    WindowsAccessImpl( const uno::Reference< uno::XComponentContext >& xContext ):m_xContext( xContext )
    {
        uno::Reference< container::XEnumeration > xEnum = new WindowComponentEnumImpl( m_xContext );
        sal_Int32 nIndex=0;
        while( xEnum->hasMoreElements() )
        {
            uno::Reference< sheet::XSpreadsheetDocument > xNext( xEnum->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
            {
                m_windows.push_back( xNext );
                uno::Reference< frame::XModel > xModel( xNext, uno::UNO_QUERY_THROW ); // that the spreadsheetdocument is a xmodel is a given
                // !! TODO !! iterate over all controllers
                uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
                uno::Reference< XHelperInterface > xTemp;  // temporary needed for g++ 3.3.5
                uno::Reference< ScVbaWindow > window( new ScVbaWindow( xTemp, m_xContext, xModel, xController ) );
                OUString sCaption;
                window->getCaption() >>= sCaption;
                namesToIndices[ sCaption ] = nIndex++;
            }
        }

    }

    //XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException, std::exception) override
    {
        return new WindowComponentEnumImpl( m_xContext, m_windows );
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException, std::exception) override
    {
        return m_windows.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw ( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        if ( Index < 0
            || static_cast< Components::size_type >( Index ) >= m_windows.size() )
            throw lang::IndexOutOfBoundsException();
        return makeAny( m_windows[ Index ] ); // returns xspreadsheetdoc
    }

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException, std::exception) override
    {
        return cppu::UnoType<sheet::XSpreadsheetDocument>::get();
    }

    virtual sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException, std::exception) override
    {
        return ( !m_windows.empty() );
    }

    //XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException, std::exception) override
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return makeAny( m_windows[ it->second ] );

    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException, std::exception) override
    {
        uno::Sequence< OUString > names( namesToIndices.size() );
        OUString* pString = names.getArray();
        NameIndexHash::const_iterator it = namesToIndices.begin();
        NameIndexHash::const_iterator it_end = namesToIndices.end();
        for ( ; it != it_end; ++it, ++pString )
            *pString = it->first;
        return names;
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (uno::RuntimeException, std::exception) override
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        return (it != namesToIndices.end());
    }

};

ScVbaWindows::ScVbaWindows( const uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : ScVbaWindows_BASE( xParent, xContext, uno::Reference< container::XIndexAccess > ( new WindowsAccessImpl( xContext ) ) )
{
}
uno::Reference< container::XEnumeration >
ScVbaWindows::createEnumeration() throw (uno::RuntimeException)
{
    return new WindowEnumImpl( mxContext, Application() );
}

uno::Any
ScVbaWindows::createCollectionObject( const css::uno::Any& aSource )
{
    return ComponentToWindow( aSource,  mxContext, Application() );
}

uno::Type
ScVbaWindows::getElementType() throw (uno::RuntimeException)
{
    return cppu::UnoType<excel::XWindows>::get();
}

void SAL_CALL
ScVbaWindows::Arrange( ::sal_Int32 /*ArrangeStyle*/, const uno::Any& /*ActiveWorkbook*/, const uno::Any& /*SyncHorizontal*/, const uno::Any& /*SyncVertical*/ ) throw (uno::RuntimeException, std::exception)
{
    //#TODO #FIXME see what can be done for an implementation here
}

OUString
ScVbaWindows::getServiceImplName()
{
    return OUString("ScVbaWindows");
}

css::uno::Sequence<OUString>
ScVbaWindows::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = "ooo.vba.excel.Windows";
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
