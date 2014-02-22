/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include "vbawindows.hxx"

#include <boost/unordered_map.hpp>

#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <cppuhelper/implbase3.hxx>

#include "vbawindow.hxx"
#include "vbaglobals.hxx"

using namespace ::com::sun::star;
using namespace ::ooo::vba;

typedef  boost::unordered_map< OUString,
sal_Int32, OUStringHash,
::std::equal_to< OUString > > NameIndexHash;


static uno::Reference< XHelperInterface > lcl_createWorkbookHIParent( const uno::Reference< frame::XModel >& xModel, const uno::Reference< uno::XComponentContext >& xContext, const uno::Any& aApplication )
{
    return new ScVbaWorkbook( uno::Reference< XHelperInterface >( aApplication, uno::UNO_QUERY_THROW ), xContext,  xModel );
}

uno::Any ComponentToWindow( const uno::Any& aSource, uno::Reference< uno::XComponentContext > & xContext, const uno::Any& aApplication )
{
    uno::Reference< frame::XModel > xModel( aSource, uno::UNO_QUERY_THROW );
    
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    uno::Reference< excel::XWindow > xWin( new ScVbaWindow( lcl_createWorkbookHIParent( xModel, xContext, aApplication ), xContext, xModel, xController ) );
    return uno::makeAny( xWin );
}

typedef std::vector < uno::Reference< sheet::XSpreadsheetDocument > > Components;

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
    
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return m_it != m_components.end();
    }

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
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
    WindowEnumImpl(const uno::Reference< uno::XComponentContext >& xContext, const Components& components, const uno::Any& aApplication ):WindowComponentEnumImpl( xContext, components ), m_aApplication( aApplication ){}
    WindowEnumImpl( const uno::Reference< uno::XComponentContext >& xContext,  const uno::Any& aApplication ): WindowComponentEnumImpl( xContext ), m_aApplication( aApplication ) {}
    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        return ComponentToWindow( WindowComponentEnumImpl::nextElement(), m_xContext, m_aApplication );
    }
};

typedef ::cppu::WeakImplHelper3< container::XEnumerationAccess
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
                uno::Reference< frame::XModel > xModel( xNext, uno::UNO_QUERY_THROW ); 
                
                uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
                uno::Reference< XHelperInterface > xTemp;  
                ScVbaWindow window( xTemp, m_xContext, xModel, xController );
                OUString sCaption;
                window.getCaption() >>= sCaption;
                namesToIndices[ sCaption ] = nIndex++;
            }
        }

    }

    
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new WindowComponentEnumImpl( m_xContext, m_windows );
    }
    
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return m_windows.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw ( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0
            || static_cast< Components::size_type >( Index ) >= m_windows.size() )
            throw lang::IndexOutOfBoundsException();
        return makeAny( m_windows[ Index ] ); 
    }

    
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return cppu::UnoType<sheet::XSpreadsheetDocument>::get();
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return ( !m_windows.empty() );
    }

    
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return makeAny( m_windows[ it->second ] );

    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) throw (uno::RuntimeException)
    {
        uno::Sequence< OUString > names( namesToIndices.size() );
        OUString* pString = names.getArray();
        NameIndexHash::const_iterator it = namesToIndices.begin();
        NameIndexHash::const_iterator it_end = namesToIndices.end();
        for ( ; it != it_end; ++it, ++pString )
            *pString = it->first;
        return names;
    }

    virtual ::sal_Bool SAL_CALL hasByName( const OUString& aName ) throw (uno::RuntimeException)
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
ScVbaWindows::Arrange( ::sal_Int32 /*ArrangeStyle*/, const uno::Any& /*ActiveWorkbook*/, const uno::Any& /*SyncHorizontal*/, const uno::Any& /*SyncVertical*/ ) throw (uno::RuntimeException)
{
    
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
