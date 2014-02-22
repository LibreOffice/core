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

#include "vbahelper/vbadocumentsbase.hxx"

#include <unotools/mediadescriptor.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>
#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/document/MacroExecMode.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>
#include <boost/unordered_map.hpp>
#include <osl/file.hxx>

#include "vbahelper/vbahelper.hxx"
#include "vbahelper/vbaapplicationbase.hxx"
#include "vbahelper/vbadocumentbase.hxx"

using namespace ::ooo::vba;
using namespace ::com::sun::star;

static const char aSpreadsheetDocument[] = "com.sun.star.sheet.SpreadsheetDocument";
static const char aTextDocument[] = "com.sun.star.text.TextDocument";

typedef  boost::unordered_map< OUString,
sal_Int32, OUStringHash,
::std::equal_to< OUString > > NameIndexHash;

typedef std::vector < uno::Reference< frame::XModel > > Documents;

typedef ::cppu::WeakImplHelper1< container::XEnumeration > DocumentsEnumImpl_BASE;




class DocumentsEnumImpl : public DocumentsEnumImpl_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    Documents m_documents;
    Documents::const_iterator m_it;

public:
    DocumentsEnumImpl( const uno::Reference< uno::XComponentContext >& xContext, const Documents& docs ) throw ( uno::RuntimeException ) :  m_xContext( xContext ), m_documents( docs )
    {
        m_it = m_documents.begin();
    }
    DocumentsEnumImpl( const uno::Reference< uno::XComponentContext >& xContext ) throw ( uno::RuntimeException ) :  m_xContext( xContext )
    {
        uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( m_xContext );
        uno::Reference< container::XEnumeration > mxComponents = xDesktop->getComponents()->createEnumeration();
        while( mxComponents->hasMoreElements() )
        {
            uno::Reference< frame::XModel > xNext( mxComponents->nextElement(), uno::UNO_QUERY );
            if ( xNext.is() )
                m_documents.push_back( xNext );
        }
        m_it = m_documents.begin();
    }
    
    virtual ::sal_Bool SAL_CALL hasMoreElements(  ) throw (uno::RuntimeException)
    {
        return m_it != m_documents.end();
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







typedef ::cppu::WeakImplHelper3< container::XEnumerationAccess
    , com::sun::star::container::XIndexAccess
    , com::sun::star::container::XNameAccess
    > DocumentsAccessImpl_BASE;

class DocumentsAccessImpl : public DocumentsAccessImpl_BASE
{
    uno::Reference< uno::XComponentContext > m_xContext;
    Documents m_documents;
    NameIndexHash namesToIndices;
    VbaDocumentsBase::DOCUMENT_TYPE meDocType;
public:
    DocumentsAccessImpl( const uno::Reference< uno::XComponentContext >& xContext, VbaDocumentsBase::DOCUMENT_TYPE eDocType ) throw (uno::RuntimeException) :m_xContext( xContext ), meDocType( eDocType )
    {
        uno::Reference< container::XEnumeration > xEnum = new DocumentsEnumImpl( m_xContext );
        sal_Int32 nIndex=0;
        while( xEnum->hasMoreElements() )
        {
            uno::Reference< lang::XServiceInfo > xServiceInfo( xEnum->nextElement(), uno::UNO_QUERY );
            if ( xServiceInfo.is()
                && (  ( xServiceInfo->supportsService( OUString(aSpreadsheetDocument) ) && meDocType == VbaDocumentsBase::EXCEL_DOCUMENT )
                || ( xServiceInfo->supportsService( OUString(aTextDocument) ) && meDocType == VbaDocumentsBase::WORD_DOCUMENT ) ) )
            {
                uno::Reference< frame::XModel > xModel( xServiceInfo, uno::UNO_QUERY_THROW ); 
                m_documents.push_back( xModel );
                OUString sName = VbaDocumentBase::getNameFromModel( xModel );
                namesToIndices[ sName ] = nIndex++;
            }
        }

    }

    
    virtual uno::Reference< container::XEnumeration > SAL_CALL createEnumeration(  ) throw (uno::RuntimeException)
    {
        return new DocumentsEnumImpl( m_xContext, m_documents );
    }
    
    virtual ::sal_Int32 SAL_CALL getCount(  ) throw (uno::RuntimeException)
    {
        return m_documents.size();
    }
    virtual uno::Any SAL_CALL getByIndex( ::sal_Int32 Index ) throw ( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException)
    {
        if ( Index < 0
            || static_cast< Documents::size_type >(Index) >= m_documents.size() )
            throw lang::IndexOutOfBoundsException();
        return makeAny( m_documents[ Index ] ); 
    }

    
    virtual uno::Type SAL_CALL getElementType(  ) throw (uno::RuntimeException)
    {
        return cppu::UnoType<frame::XModel>::get();
    }

    virtual ::sal_Bool SAL_CALL hasElements(  ) throw (uno::RuntimeException)
    {
        return (!m_documents.empty());
    }

    
    virtual uno::Any SAL_CALL getByName( const OUString& aName ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        NameIndexHash::const_iterator it = namesToIndices.find( aName );
        if ( it == namesToIndices.end() )
            throw container::NoSuchElementException();
        return makeAny( m_documents[ it->second ] );

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

VbaDocumentsBase::VbaDocumentsBase( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext >& xContext, DOCUMENT_TYPE eDocType ) throw (uno::RuntimeException) : VbaDocumentsBase_BASE( xParent, xContext, uno::Reference< container::XIndexAccess >( new DocumentsAccessImpl( xContext, eDocType ) ) ), meDocType( eDocType )
{
}

namespace {

void lclSetupComponent( const uno::Reference< lang::XComponent >& rxComponent, sal_Bool bScreenUpdating, sal_Bool bInteractive )
{
    if( !bScreenUpdating ) try
    {
        uno::Reference< frame::XModel >( rxComponent, uno::UNO_QUERY_THROW )->lockControllers();
    }
    catch( uno::Exception& )
    {
    }

    if( !bInteractive ) try
    {
        uno::Reference< frame::XModel > xModel( rxComponent, uno::UNO_QUERY_THROW );
        uno::Reference< frame::XController > xController( xModel->getCurrentController(), uno::UNO_SET_THROW );
        uno::Reference< frame::XFrame > xFrame( xController->getFrame(), uno::UNO_SET_THROW );
        uno::Reference< awt::XWindow >( xFrame->getContainerWindow(), uno::UNO_SET_THROW )->setEnable( sal_False );
    }
    catch( uno::Exception& )
    {
    }
}

} 

uno::Any VbaDocumentsBase::createDocument() throw (uno::RuntimeException)
{
    
    uno::Reference< XApplicationBase > xApplication( Application(), uno::UNO_QUERY );
    sal_Bool bScreenUpdating = !xApplication.is() || xApplication->getScreenUpdating();
    sal_Bool bInteractive = !xApplication.is() || xApplication->getInteractive();

    uno::Reference< frame::XDesktop2 > xLoader = frame::Desktop::create(mxContext);
    OUString sURL;
    if( meDocType == WORD_DOCUMENT )
        sURL = "private:factory/swriter";
    else if( meDocType == EXCEL_DOCUMENT )
        sURL = "private:factory/scalc";
    else
        throw uno::RuntimeException( "Not implemented" , uno::Reference< uno::XInterface >() );

    
    utl::MediaDescriptor aMediaDesc;
    aMediaDesc[ utl::MediaDescriptor::PROP_MACROEXECUTIONMODE() ] <<= document::MacroExecMode::USE_CONFIG;
    aMediaDesc.setComponentDataEntry( "ApplyFormDesignMode" , uno::Any( false ) );

    
    uno::Reference< lang::XComponent > xComponent = xLoader->loadComponentFromURL(
                                       sURL , "_blank", 0,
                                       aMediaDesc.getAsConstPropertyValueList() );

    
    lclSetupComponent( xComponent, bScreenUpdating, bInteractive );

    return uno::makeAny( xComponent );
}

void VbaDocumentsBase::closeDocuments() throw (uno::RuntimeException)
{


/*
    uno::Reference< lang::XMultiComponentFactory > xSMgr(
        mxContext->getServiceManager(), uno::UNO_QUERY_THROW );
    uno::Reference< frame::XModel > xModel( getCurrentDocument(), uno::UNO_QUERY_THROW );
    OUString url = ".uno:CloseDoc";
    dispatchRequests(xModel,url);
*/
}


uno::Any VbaDocumentsBase::openDocument( const OUString& rFileName, const uno::Any& ReadOnly, const uno::Sequence< beans::PropertyValue >& rProps ) throw (uno::RuntimeException)
{
    
    uno::Reference< XApplicationBase > xApplication( Application(), uno::UNO_QUERY );
    sal_Bool bScreenUpdating = !xApplication.is() || xApplication->getScreenUpdating();
    sal_Bool bInteractive = !xApplication.is() || xApplication->getInteractive();

    
        OUString aURL;
        INetURLObject aObj;
    aObj.SetURL( rFileName );
    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
    if ( bIsURL )
        aURL = rFileName;
    else
        osl::FileBase::getFileURLFromSystemPath( rFileName, aURL );
    uno::Reference< frame::XDesktop2 > xDesktop = frame::Desktop::create( mxContext );

    uno::Sequence< beans::PropertyValue > sProps( rProps );
    sProps.realloc( sProps.getLength() + 1 );
    sProps[ sProps.getLength() - 1 ].Name = "MacroExecutionMode";
    sProps[ sProps.getLength() - 1 ].Value <<= document::MacroExecMode::ALWAYS_EXECUTE_NO_WARN;

    if ( ReadOnly.hasValue()  )
    {
        sal_Bool bIsReadOnly = sal_False; ReadOnly >>= bIsReadOnly;
        if ( bIsReadOnly )
        {
            sProps.realloc( sProps.getLength() + 1 );
            sProps[ sProps.getLength() - 1 ].Name = "ReadOnly";
            sProps[ sProps.getLength() - 1 ].Value <<= true;
        }
    }

    uno::Reference< lang::XComponent > xComponent = xDesktop->loadComponentFromURL( aURL,
        "_default" ,
        frame::FrameSearchFlag::CREATE,
        sProps);

    
    lclSetupComponent( xComponent, bScreenUpdating, bInteractive );

    return uno::makeAny( xComponent );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
