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
#include <comphelper/processfactory.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase3.hxx>

#include <com/sun/star/frame/XDesktop.hpp>
#include <com/sun/star/sheet/XSpreadsheetDocument.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/beans/PropertyVetoException.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <com/sun/star/document/XTypeDetection.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>
#include <com/sun/star/script/vba/VBAEventId.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAEventProcessor.hpp>
#include <com/sun/star/script/vba/XVBAModuleInfo.hpp>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/script/ModuleType.hpp>

#include <sfx2/objsh.hxx>
#include <tools/urlobj.hxx>

#include "vbaglobals.hxx"
#include "vbaworkbook.hxx"
#include "vbaworkbooks.hxx"
#include <vbahelper/vbahelper.hxx>

#include <boost/unordered_map.hpp>
#include <vector>
#include <osl/file.hxx>
using namespace ::ooo::vba;
using namespace ::com::sun::star;

const sal_Int16 CUSTOM_CHAR = 5;

void setUpDocumentModules( const uno::Reference< sheet::XSpreadsheetDocument >& xDoc )
{
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    ScDocShell* pShell = excel::getDocShell( xModel );
    if ( pShell )
    {
        String aPrjName( RTL_CONSTASCII_USTRINGPARAM( "Standard" ) );
        pShell->GetBasicManager()->SetName( aPrjName );

        /*  Set library container to VBA compatibility mode. This will create
            the VBA Globals object and store it in the Basic manager of the
            document. */
        uno::Reference<script::XLibraryContainer> xLibContainer = pShell->GetBasicContainer();
        uno::Reference<script::vba::XVBACompatibility> xVBACompat( xLibContainer, uno::UNO_QUERY_THROW );
        xVBACompat->setVBACompatibilityMode( sal_True );

        if( xLibContainer.is() )
        {
            if( !xLibContainer->hasByName( aPrjName ) )
                xLibContainer->createLibrary( aPrjName );
            uno::Any aLibAny = xLibContainer->getByName( aPrjName );
            uno::Reference< container::XNameContainer > xLib;
            aLibAny >>= xLib;
            if( xLib.is()  )
            {
                uno::Reference< script::vba::XVBAModuleInfo > xVBAModuleInfo( xLib, uno::UNO_QUERY_THROW );
                uno::Reference< lang::XMultiServiceFactory> xSF( pShell->GetModel(), uno::UNO_QUERY_THROW);
                uno::Reference< container::XNameAccess > xVBACodeNamedObjectAccess( xSF->createInstance("ooo.vba.VBAObjectModuleObjectProvider"), uno::UNO_QUERY_THROW );
                // set up the module info for the workbook and sheets in the nealy created
                // spreadsheet
                ScDocument* pDoc = pShell->GetDocument();
                String sCodeName = pDoc->GetCodeName();
                if ( sCodeName.Len() == 0 )
                {
                    sCodeName = String( RTL_CONSTASCII_USTRINGPARAM("ThisWorkbook") );
                    pDoc->SetCodeName( sCodeName );
                }

                std::vector< OUString > sDocModuleNames;
                sDocModuleNames.push_back( sCodeName );

                uno::Reference<container::XNameAccess > xSheets( xDoc->getSheets(), uno::UNO_QUERY_THROW );
                uno::Sequence< OUString > sSheets( xSheets->getElementNames() );

                for ( sal_Int32 index=0; index < sSheets.getLength() ; ++index )
                {
                    sDocModuleNames.push_back( sSheets[ index ] );
                }

                std::vector<OUString>::iterator it_end = sDocModuleNames.end();

                for ( std::vector<OUString>::iterator it = sDocModuleNames.begin(); it != it_end; ++it )
                {
                    script::ModuleInfo sModuleInfo;

                    sModuleInfo.ModuleObject.set( xVBACodeNamedObjectAccess->getByName( *it ), uno::UNO_QUERY );
                    sModuleInfo.ModuleType = script::ModuleType::DOCUMENT;
                    xVBAModuleInfo->insertModuleInfo( *it, sModuleInfo );
                    if( xLib->hasByName( *it ) )
                        xLib->replaceByName( *it, uno::makeAny( OUString( "Option VBASupport 1\n") ) );
                    else
                        xLib->insertByName( *it, uno::makeAny( OUString( "Option VBASupport 1\n" ) ) );
                }
            }
        }
    }
}

static uno::Any
getWorkbook( uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< sheet::XSpreadsheetDocument > &xDoc, const uno::Reference< XHelperInterface >& xParent )
{
    // FIXME: fine as long as ScVbaWorkbook is stateless ...
    uno::Reference< frame::XModel > xModel( xDoc, uno::UNO_QUERY );
    if( !xModel.is() )
        return uno::Any();

    uno::Reference< excel::XWorkbook > xWb( getVBADocument( xModel ), uno::UNO_QUERY );
    if ( xWb.is() )
    {
        OSL_TRACE(" *** Returning Module uno Object *** ");
        return uno::Any( xWb );
    }

    ScVbaWorkbook *pWb = new ScVbaWorkbook( xParent, xContext, xModel );
    return uno::Any( uno::Reference< excel::XWorkbook > (pWb) );
}

class WorkBookEnumImpl : public EnumerationHelperImpl
{
    uno::Any m_aApplication;
public:
    WorkBookEnumImpl( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< uno::XComponentContext >& xContext, const uno::Reference< container::XEnumeration >& xEnumeration, const uno::Any& aApplication ) throw ( uno::RuntimeException ) : EnumerationHelperImpl( xParent, xContext, xEnumeration ), m_aApplication( aApplication ) {}

    virtual uno::Any SAL_CALL nextElement(  ) throw (container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException)
    {
        uno::Reference< sheet::XSpreadsheetDocument > xDoc( m_xEnumeration->nextElement(), uno::UNO_QUERY_THROW );
        return getWorkbook( m_xContext, xDoc, m_xParent );
    }

};

ScVbaWorkbooks::ScVbaWorkbooks( const uno::Reference< XHelperInterface >& xParent, const uno::Reference< css::uno::XComponentContext >& xContext ) : ScVbaWorkbooks_BASE( xParent, xContext, VbaDocumentsBase::EXCEL_DOCUMENT )
{
}
// XEnumerationAccess
uno::Type
ScVbaWorkbooks::getElementType() throw (uno::RuntimeException)
{
    return excel::XWorkbook::static_type(0);
}
uno::Reference< container::XEnumeration >
ScVbaWorkbooks::createEnumeration() throw (uno::RuntimeException)
{
    // #FIXME its possible the WorkBookEnumImpl here doens't reflect
    // the state of this object ( although it should ) would be
    // safer to create an enumeration based on this objects state
    // rather than one effectively based of the desktop component
    uno::Reference< container::XEnumerationAccess > xEnumerationAccess( m_xIndexAccess, uno::UNO_QUERY_THROW );
    return new WorkBookEnumImpl( mxParent, mxContext, xEnumerationAccess->createEnumeration(), Application() );
}

uno::Any
ScVbaWorkbooks::createCollectionObject( const css::uno::Any& aSource )
{
    uno::Reference< sheet::XSpreadsheetDocument > xDoc( aSource, uno::UNO_QUERY_THROW );
    return getWorkbook( mxContext, xDoc, mxParent );
}


uno::Any SAL_CALL
ScVbaWorkbooks::Add( const uno::Any& Template ) throw (uno::RuntimeException)
{
    uno::Reference< sheet::XSpreadsheetDocument > xSpreadDoc;
    sal_Int32 nWorkbookType = 0;
    OUString aTemplateFileName;
    if( Template >>= nWorkbookType )
    {
        // nWorkbookType is a constant from XlWBATemplate (added in Excel 2007)
        // TODO: create chart-sheet if supported by Calc

        xSpreadDoc.set( createDocument(), uno::UNO_QUERY_THROW );
        // create a document with one sheet only
        uno::Reference< sheet::XSpreadsheets > xSheets( xSpreadDoc->getSheets(), uno::UNO_SET_THROW );
        uno::Reference< container::XIndexAccess > xSheetsIA( xSheets, uno::UNO_QUERY_THROW );
        while( xSheetsIA->getCount() > 1 )
        {
            uno::Reference< container::XNamed > xSheetName( xSheetsIA->getByIndex( xSheetsIA->getCount() - 1 ), uno::UNO_QUERY_THROW );
            xSheets->removeByName( xSheetName->getName() );
        }
    }
    else if( Template >>= aTemplateFileName )
    {
        // TODO: create document from template
        xSpreadDoc.set( createDocument(), uno::UNO_QUERY_THROW );
    }
    else if( !Template.hasValue() )
    {
        // regular spreadsheet document with configured number of sheets
        xSpreadDoc.set( createDocument(), uno::UNO_QUERY_THROW );
    }
    else
    {
        // illegal argument
        throw uno::RuntimeException();
    }

    // need to set up the document modules ( and vba mode ) here
    excel::setUpDocumentModules( xSpreadDoc );
    if( xSpreadDoc.is() )
        return getWorkbook( mxContext, xSpreadDoc, mxParent );
    return uno::Any();
}

void SAL_CALL
ScVbaWorkbooks::Close() throw (uno::RuntimeException)
{
    closeDocuments();
}

bool
ScVbaWorkbooks::isTextFile( const OUString& sType )
{
    // will return true if the file is
    // a) a variant of a text file
    // b) a csv file
    // c) unknown
    // returning true basically means treat this like a csv file
    const static OUString txtType("generic_Text");
    return sType.equals( txtType ) || sType.isEmpty();
}

bool
ScVbaWorkbooks::isSpreadSheetFile( const OUString& sType )
{
    // include calc_QPro etc. ? ( not for the moment anyway )
    if ( sType.indexOf( "calc_MS" ) == 0
    || sType.indexOf( "calc8" ) == 0
    || sType.indexOf( "calc_StarOffice" ) == 0 )
        return true;
    return false;
}

OUString
ScVbaWorkbooks::getFileFilterType( const OUString& rFileName )
{
    uno::Reference< document::XTypeDetection > xTypeDetect( mxContext->getServiceManager()->createInstanceWithContext("com.sun.star.document.TypeDetection", mxContext), uno::UNO_QUERY_THROW );
    uno::Sequence< beans::PropertyValue > aMediaDesc(1);
    aMediaDesc[ 0 ].Name = OUString("URL" );
    aMediaDesc[ 0 ].Value <<= rFileName;
    OUString sType = xTypeDetect->queryTypeByDescriptor( aMediaDesc, sal_True );
    return sType;
}

// #TODO# #FIXME# can any of the unused params below be used?
uno::Any SAL_CALL
ScVbaWorkbooks::Open( const OUString& rFileName, const uno::Any& /*UpdateLinks*/, const uno::Any& ReadOnly, const uno::Any& Format, const uno::Any& /*Password*/, const uno::Any& /*WriteResPassword*/, const uno::Any& /*IgnoreReadOnlyRecommended*/, const uno::Any& /*Origin*/, const uno::Any& Delimiter, const uno::Any& /*Editable*/, const uno::Any& /*Notify*/, const uno::Any& /*Converter*/, const uno::Any& /*AddToMru*/ ) throw (uno::RuntimeException)
{
    // we need to detect if this is a URL, if not then assume its a file path
    OUString aURL;
    INetURLObject aObj;
    aObj.SetURL( rFileName );
    bool bIsURL = aObj.GetProtocol() != INET_PROT_NOT_VALID;
    if ( bIsURL )
        aURL = rFileName;
    else
        osl::FileBase::getFileURLFromSystemPath( rFileName, aURL );

    uno::Sequence< beans::PropertyValue > sProps(0);

    OUString sType = getFileFilterType( aURL );
    // A text file means it needs to be processed as a csv file
    if ( isTextFile( sType ) )
    {
        sal_Int32 nIndex = 0;
        // Values for format
        // 1 Tabs
        // 2 Commas
        // 3 Spaces
        // 4 Semicolons
        // 5 Nothing
        // 6 Custom character (see the Delimiter argument
        // no format means use the current delimiter
        sProps.realloc( 3 );
        sProps[ nIndex ].Name = OUString("FilterOptions" );
        sal_Int16 delims[] = { 0 /*default not used*/, 9/*tab*/, 44/*comma*/, 32/*space*/, 59/*semicolon*/ };
        static OUString sRestOfFormat(",34,0,1" );

        OUString sFormat;
        sal_Int16 nFormat = 0; // default indicator


        if ( Format.hasValue() )
        {
            Format >>= nFormat; // val of nFormat overwritten if extracted
            // validate param
            if ( nFormat < 1 || nFormat > 6 )
                throw uno::RuntimeException("Illegal value for Format", uno::Reference< uno::XInterface >() );
        }

        sal_Int16 nDelim = getCurrentDelim();

        if (  nFormat > 0 && nFormat < CUSTOM_CHAR )
        {
            nDelim =  delims[ nFormat ];
        }
        else if ( nFormat > CUSTOM_CHAR )
        {
            // Need to check Delimiter param
            if ( !Delimiter.hasValue() )
                throw uno::RuntimeException("Expected value for Delimiter", uno::Reference< uno::XInterface >() );
            OUString sStr;
            Delimiter >>= sStr;
            String aUniStr( sStr );
            if ( aUniStr.Len() )
                nDelim = aUniStr.GetChar(0);
            else
                throw uno::RuntimeException("Incorrect value for Delimiter", uno::Reference< uno::XInterface >() );
        }

        getCurrentDelim() = nDelim; //set new current

        sFormat = OUString::number( nDelim ) + sRestOfFormat;
        sProps[ nIndex++ ].Value <<= sFormat;
        sProps[ nIndex ].Name = OUString("FilterName");
        sProps[ nIndex++ ].Value <<= OUString( "Text - txt - csv (StarCalc)" );
        // Ensure WORKAROUND_CSV_TXT_BUG_i60158 gets called in typedetection.cxx so
        // csv is forced for deep detected 'writerxxx' types
        sProps[ nIndex ].Name = OUString("DocumentService");
        sProps[ nIndex ].Value <<= OUString("com.sun.star.sheet.SpreadsheetDocument");
    }
    else if ( !isSpreadSheetFile( sType ) )
        throw uno::RuntimeException("Bad Format", uno::Reference< uno::XInterface >() );

    uno::Reference <sheet::XSpreadsheetDocument> xSpreadDoc( openDocument( rFileName, ReadOnly, sProps ), uno::UNO_QUERY_THROW );
    uno::Any aRet = getWorkbook( mxContext, xSpreadDoc, mxParent );
    uno::Reference< excel::XWorkbook > xWBook( aRet, uno::UNO_QUERY );
    if ( xWBook.is() )
        xWBook->Activate();
    return aRet;
}

OUString
ScVbaWorkbooks::getServiceImplName()
{
    return OUString("ScVbaWorkbooks");
}

css::uno::Sequence<OUString>
ScVbaWorkbooks::getServiceNames()
{
    static uno::Sequence< OUString > sNames;
    if ( sNames.getLength() == 0 )
    {
        sNames.realloc( 1 );
        sNames[0] = OUString("ooo.vba.excel.Workbooks");
    }
    return sNames;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
