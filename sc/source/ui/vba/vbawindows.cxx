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
#include <comphelper/sequence.hxx>
#include <o3tl/safeint.hxx>
#include <rtl/ref.hxx>

#include "vbawindow.hxx"
#include "vbaworkbook.hxx"

#include <unordered_map>
#include <utility>

#include <osl/file.hxx>
#include <ooo/vba/excel/XApplication.hpp>

using namespace ::com::sun::star;
using namespace ::ooo::vba;

typedef  std::unordered_map< OUString,
sal_Int32 > NameIndexHash;

static uno::Reference< XHelperInterface > lcl_createWorkbookHIParent( const uno::Reference< frame::XModel >& xModel, const uno::Reference< uno::XComponentContext >& xContext, const uno::Any& aApplication )
{
    return new ScVbaWorkbook( uno::Reference< XHelperInterface >( aApplication, uno::UNO_QUERY_THROW ), xContext,  xModel );
}

static uno::Any ComponentToWindow( const uno::Any& aSource, const uno::Reference< uno::XComponentContext > & xContext, const uno::Any& aApplication )
{
    uno::Reference< frame::XModel > xModel( aSource, uno::UNO_QUERY_THROW );
    // !! TODO !! iterate over all controllers
    uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
    uno::Reference< excel::XWindow > xWin( new ScVbaWindow( lcl_createWorkbookHIParent( xModel, xContext, aApplication ), xContext, xModel, xController ) );
    return uno::Any( xWin );
}

typedef std::vector < uno::Reference< sheet::XSpreadsheetDocument > > Components;

namespace {

// #TODO more or less the same as class in workwindows ( code sharing needed )
class WindowComponentEnumImpl : public EnumerationHelper_BASE
{
protected:
    uno::Reference< uno::XComponentContext > m_xContext;
    Components m_components;
    Components::const_iterator m_it;

public:
    /// @throws uno::RuntimeException
    WindowComponentEnumImpl( uno::Reference< uno::XComponentContext > xContext, Components&& components )
        :  m_xContext(std::move( xContext )), m_components( std::move(components) )
    {
        m_it = m_components.begin();
    }

    /// @throws uno::RuntimeException
    explicit WindowComponentEnumImpl( uno::Reference< uno::XComponentContext > xContext ) :  m_xContext(std::move( xContext ))
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create(m_xContext);
        uno::Reference< container::XEnumeration > xComponents = xDesktop->getComponents()->createEnumeration();
        while( xComponents->hasMoreElements() )
        {
            uno::Reference< sheet::XSpreadsheetDocument > xNext( xComponents->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
                m_components.push_back( xNext );
        }
        m_it = m_components.begin();
    }
    // XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override
    {
        return m_it != m_components.end();
    }

    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        if ( !hasMoreElements() )
        {
            throw container::NoSuchElementException();
        }
        return css::uno::Any( *(m_it++) );
    }
};

class WindowEnumImpl : public  WindowComponentEnumImpl
{
    uno::Any m_aApplication;
public:
    WindowEnumImpl( const uno::Reference< uno::XComponentContext >& xContext,  uno::Any  aApplication ): WindowComponentEnumImpl( xContext ), m_aApplication(std::move( aApplication )) {}
    virtual uno::Any SAL_CALL nextElement(  ) override
    {
        return ComponentToWindow( WindowComponentEnumImpl::nextElement(), m_xContext, m_aApplication );
    }
};

}

typedef ::cppu::WeakImplHelper< container::XEnumerationAccess
    , css::container::XIndexAccess
    , css::container::XNameAccess
    > WindowsAccessImpl_BASE;

namespace {

class WindowsAccessImpl : public WindowsAccessImpl_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    Components m_windows;
    NameIndexHash namesToIndices;
public:
    explicit WindowsAccessImpl( uno::Reference< uno::XComponentContext > xContext ):m_xContext(std::move( xContext ))
    {
        css::uno::Reference<css::container::XNameAccess> xNameAccess(m_xContext,
                                                                     css::uno::UNO_QUERY_THROW);
        const auto aAppplication = xNameAccess->getByName(u"Application"_ustr);

        uno::Reference< container::XEnumeration > xEnum = new WindowComponentEnumImpl( m_xContext );
        sal_Int32 nIndex=0;
        while( xEnum->hasMoreElements() )
        {
            uno::Reference< sheet::XSpreadsheetDocument > xNext( xEnum->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
            {
                m_windows.push_back( xNext );
                uno::Reference< frame::XModel > xModel( xNext, uno::UNO_QUERY_THROW ); // that the spreadsheetdocument is a xmodel is a given

                // tdf#126457 - add workbook name to window titles
                rtl::Reference<ScVbaWorkbook> workbook(new ScVbaWorkbook(
                    uno::Reference<XHelperInterface>(aAppplication, uno::UNO_QUERY_THROW),
                    m_xContext, xModel));
                const OUString aWorkBookName(workbook->getName());
                if (!hasByName(aWorkBookName))
                    namesToIndices[aWorkBookName] = nIndex;

                // tdf#126457 - add file url to window titles
                OUString sName;
                ::osl::File::getSystemPathFromFileURL(xModel->getURL(), sName);
                if (!hasByName(sName))
                    namesToIndices[sName] = nIndex;

                // !! TODO !! iterate over all controllers
                uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
                uno::Reference< XHelperInterface > xTemp;  // temporary needed for g++ 3.3.5
                rtl::Reference< ScVbaWindow > window( new ScVbaWindow( xTemp, m_xContext, xModel, xController ) );
                OUString sCaption;
                window->getCaption() >>= sCaption;
                namesToIndices[ sCaption ] = nIndex++;
            }
        }

    }

    //XEnumerationAccess
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) override
    {
        return new WindowComponentEnumImpl( m_xContext, std::vector(m_windows) );
    }
    // XIndexAccess
    virtual ::sal_Int32 SAL_CALL getCount(  ) override
    {
        return m_windows.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) override
    {
        if ( Index < 0
            || o3tl::make_unsigned( Index ) >= m_windows.size() )
            throw lang::IndexOutOfBoundsException();
        return css::uno::Any( m_windows[ Index ] ); // returns xspreadsheetdoc
    }

    //XElementAccess
    virtual uno::Type SAL_CALL getElementType(  ) override
    {
        return cppu::UnoType<sheet::XSpreadsheetDocument>::get();
    }

    virtual sal_Bool SAL_CALL hasElements(  ) override
    {
        return ( !m_windows.empty() );
    }

    //XNameAccess
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) override
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return css::uno::Any( m_windows[ it->second ] );

    }

    virtual uno::Sequence< OUString > SAL_CALL getElementNames(  ) override
    {
        return comphelper::mapKeysToSequence( namesToIndices );
    }

    virtual sal_Bool SAL_CALL hasByName( const OUString& aName ) override
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        return (it != namesToIndices.end());
    }

};

}

ScVbaWindows::ScVbaWindows( const uno::Reference< ov::XHelperInterface >& xParent, const css::uno::Reference< css::uno::XComponentContext >& xContext ) : ScVbaWindows_BASE( xParent, xContext, uno::Reference< container::XIndexAccess > ( new WindowsAccessImpl( xContext ) ) )
{
}
uno::Reference< container::XEnumeration >
ScVbaWindows::createEnumeration()
{
    return new WindowEnumImpl( mxContext, Application() );
}

uno::Any
ScVbaWindows::createCollectionObject( const css::uno::Any& aSource )
{
    return ComponentToWindow( aSource,  mxContext, Application() );
}

uno::Type
ScVbaWindows::getElementType()
{
    return cppu::UnoType<excel::XWindows>::get();
}

void SAL_CALL
ScVbaWindows::Arrange( ::sal_Int32 /*ArrangeStyle*/, const uno::Any& /*ActiveWorkbook*/, const uno::Any& /*SyncHorizontal*/, const uno::Any& /*SyncVertical*/ )
{
    //#TODO #FIXME see what can be done for an implementation here
}

OUString
ScVbaWindows::getServiceImplName()
{
    return u"ScVbaWindows"_ustr;
}

css::uno::Sequence<OUString>
ScVbaWindows::getServiceNames()
{
    static uno::Sequence< OUString > const sNames
    {
        u"ooo.vba.excel.Windows"_ustr
    };
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
