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

#include "oox/ole/vbaproject.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/script/ModuleType.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <com/sun/star/script/vba/XVBACompatibility.hpp>
#include <com/sun/star/script/vba/XVBAMacroResolver.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <comphelper/configurationhelper.hxx>
#include <comphelper/string.hxx>
#include <osl/diagnose.h>
#include <rtl/tencinfo.h>
#include <rtl/ustrbuf.h>
#include "oox/helper/binaryinputstream.hxx"
#include "oox/helper/containerhelper.hxx"
#include "oox/helper/propertyset.hxx"
#include "oox/helper/textinputstream.hxx"
#include "oox/ole/olestorage.hxx"
#include "oox/ole/vbacontrol.hxx"
#include "oox/ole/vbahelper.hxx"
#include "oox/ole/vbainputstream.hxx"
#include "oox/ole/vbamodule.hxx"
#include "oox/token/properties.hxx"

namespace oox {
namespace ole {

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::embed;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::script::vba;
using namespace ::com::sun::star::uno;

using ::comphelper::ConfigurationHelper;

namespace {

bool lclReadConfigItem( const Reference< XInterface >& rxConfigAccess, const OUString& rItemName )
{
    // some applications do not support all configuration items, assume 'false' in this case
    try
    {
        Any aItem = ConfigurationHelper::readRelativeKey( rxConfigAccess, "Filter/Import/VBA", rItemName );
        return aItem.has< bool >() && aItem.get< bool >();
    }
    catch(const Exception& )
    {
    }
    return false;
}

} // namespace

VbaFilterConfig::VbaFilterConfig( const Reference< XComponentContext >& rxContext, const OUString& rConfigCompName )
{
    OSL_ENSURE( rxContext.is(), "VbaFilterConfig::VbaFilterConfig - missing component context" );
    if( rxContext.is() ) try
    {
        OSL_ENSURE( !rConfigCompName.isEmpty(), "VbaFilterConfig::VbaFilterConfig - invalid configuration component name" );
        OUString aConfigPackage = "org.openoffice.Office." + rConfigCompName;
        mxConfigAccess = ConfigurationHelper::openConfig( rxContext, aConfigPackage, comphelper::EConfigurationModes::ReadOnly );
    }
    catch(const Exception& )
    {
    }
    OSL_ENSURE( mxConfigAccess.is(), "VbaFilterConfig::VbaFilterConfig - cannot open configuration" );
}

VbaFilterConfig::~VbaFilterConfig()
{
}

bool VbaFilterConfig::isImportVba() const
{
    return lclReadConfigItem( mxConfigAccess, "Load" );
}

bool VbaFilterConfig::isImportVbaExecutable() const
{
    return lclReadConfigItem( mxConfigAccess, "Executable" );
}

bool VbaFilterConfig::isExportVba() const
{
    return lclReadConfigItem( mxConfigAccess, "Save" );
}

VbaMacroAttacherBase::VbaMacroAttacherBase( const OUString& rMacroName ) :
    maMacroName( rMacroName )
{
    OSL_ENSURE( !maMacroName.isEmpty(), "VbaMacroAttacherBase::VbaMacroAttacherBase - empty macro name" );
}

VbaMacroAttacherBase::~VbaMacroAttacherBase()
{
}

void VbaMacroAttacherBase::resolveAndAttachMacro( const Reference< XVBAMacroResolver >& rxResolver )
{
    try
    {
        attachMacro( rxResolver->resolveVBAMacroToScriptURL( maMacroName ) );
    }
    catch(const Exception& )
    {
    }
}

VbaProject::VbaProject( const Reference< XComponentContext >& rxContext,
        const Reference< XModel >& rxDocModel, const OUString& rConfigCompName ) :
    VbaFilterConfig( rxContext, rConfigCompName ),
    mxContext( rxContext ),
    mxDocModel( rxDocModel ),
    maPrjName( "Standard" )
{
    OSL_ENSURE( mxContext.is(), "VbaProject::VbaProject - missing component context" );
    OSL_ENSURE( mxDocModel.is(), "VbaProject::VbaProject - missing document model" );
}

VbaProject::~VbaProject()
{
}

bool VbaProject::importVbaProject( StorageBase& rVbaPrjStrg )
{
   // create GraphicHelper
   Reference< css::frame::XFrame > xFrame;
   if ( mxDocModel.is() )
   {
       Reference< css::frame::XController > xController =  mxDocModel->getCurrentController();
       xFrame =  xController.is() ? xController->getFrame() : nullptr;
   }
   StorageRef noStorage;
   // if the GraphicHelper tries to use noStorage it will of course crash
   // but.. this shouldn't happen as there is no reason for GraphicHelper
   // to do that when importing VBA projects
   GraphicHelper grfHlp( mxContext, xFrame, noStorage );
   importVbaProject( rVbaPrjStrg, grfHlp );
   // return true if something has been imported
   return hasModules() || hasDialogs();
}

void VbaProject::importVbaProject( StorageBase& rVbaPrjStrg, const GraphicHelper& rGraphicHelper )
{
    if( rVbaPrjStrg.isStorage() )
    {
        // load the code modules and forms
        if( isImportVba() )
            importVba( rVbaPrjStrg, rGraphicHelper, true/*bDefaultColorBgr*/ );
        // copy entire storage into model
        if( isExportVba() )
            copyStorage( rVbaPrjStrg );
    }
}

void VbaProject::registerMacroAttacher( const VbaMacroAttacherRef& rxAttacher )
{
    OSL_ENSURE( rxAttacher.get(), "VbaProject::registerMacroAttacher - unexpected empty reference" );
    maMacroAttachers.push_back( rxAttacher );
}

bool VbaProject::hasModules() const
{
    return mxBasicLib.is() && mxBasicLib->hasElements();
}

bool VbaProject::hasDialogs() const
{
    return mxDialogLib.is() && mxDialogLib->hasElements();
}

// protected ------------------------------------------------------------------

void VbaProject::addDummyModule( const OUString& rName, sal_Int32 nType )
{
    OSL_ENSURE( !rName.isEmpty(), "VbaProject::addDummyModule - missing module name" );
    maDummyModules[ rName ] = nType;
}

void VbaProject::prepareImport()
{
}

// private --------------------------------------------------------------------

Reference< XLibraryContainer > VbaProject::getLibraryContainer( sal_Int32 nPropId )
{
    PropertySet aDocProp( mxDocModel );
    Reference< XLibraryContainer > xLibContainer( aDocProp.getAnyProperty( nPropId ), UNO_QUERY );
    return xLibContainer;
}

Reference< XNameContainer > VbaProject::openLibrary( sal_Int32 nPropId, bool bCreateMissing )
{
    Reference< XNameContainer > xLibrary;
    try
    {
        Reference< XLibraryContainer > xLibContainer( getLibraryContainer( nPropId ), UNO_SET_THROW );
        if( bCreateMissing && !xLibContainer->hasByName( maPrjName ) )
            xLibContainer->createLibrary( maPrjName );
        xLibrary.set( xLibContainer->getByName( maPrjName ), UNO_QUERY_THROW );
    }
    catch(const Exception& )
    {
    }
    OSL_ENSURE( !bCreateMissing || xLibrary.is(), "VbaProject::openLibrary - cannot create library" );
    return xLibrary;
}

Reference< XNameContainer > const & VbaProject::createBasicLibrary()
{
    if( !mxBasicLib.is() )
        mxBasicLib = openLibrary( PROP_BasicLibraries, true );
    return mxBasicLib;
}

Reference< XNameContainer > const & VbaProject::createDialogLibrary()
{
    if( !mxDialogLib.is() )
        mxDialogLib = openLibrary( PROP_DialogLibraries, true );
    return mxDialogLib;
}

void VbaProject::importVba( StorageBase& rVbaPrjStrg, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr )
{
    readVbaModules( rVbaPrjStrg );
    importModulesAndForms(rVbaPrjStrg, rGraphicHelper, bDefaultColorBgr );
}

void VbaProject::readVbaModules( StorageBase& rVbaPrjStrg )
{
    StorageRef xVbaStrg = rVbaPrjStrg.openSubStorage( "VBA", false );
    OSL_ENSURE( xVbaStrg.get(), "VbaProject::readVbaModules - cannot open 'VBA' substorage" );
    if( !xVbaStrg )
        return;

    /*  Read the 'VBA/dir' stream which contains general settings of the VBA
        project such as the text encoding used throughout several streams, and
        a list of all code modules.
     */
    BinaryXInputStream aInStrm( xVbaStrg->openInputStream( "dir" ), true );
    // VbaInputStream implements decompression
    VbaInputStream aDirStrm( aInStrm );
    OSL_ENSURE( !aDirStrm.isEof(), "VbaProject::importVba - cannot open 'dir' stream" );
    if( aDirStrm.isEof() )
        return;

    // virtual call, derived classes may do some preparations
    prepareImport();

    // read all records of the directory
    rtl_TextEncoding eTextEnc = RTL_TEXTENCODING_MS_1252;
    sal_uInt16 nModuleCount = 0;
    bool bExecutable = isImportVbaExecutable();

    sal_uInt16 nRecId = 0;
    StreamDataSequence aRecData;
    while( VbaHelper::readDirRecord( nRecId, aRecData, aDirStrm ) && (nRecId != VBA_ID_PROJECTEND) )
    {
        // create record stream object from imported record data
        SequenceInputStream aRecStrm( aRecData );
        sal_Int32 nRecSize = aRecData.getLength();
        switch( nRecId )
        {
            case VBA_ID_PROJECTCODEPAGE:
            {
                OSL_ENSURE( nRecSize == 2, "VbaProject::importVba - invalid record size" );
                OSL_ENSURE( maModules.empty(), "VbaProject::importVba - unexpected PROJECTCODEPAGE record" );
                rtl_TextEncoding eNewTextEnc = rtl_getTextEncodingFromWindowsCodePage( aRecStrm.readuInt16() );
                OSL_ENSURE( eNewTextEnc != RTL_TEXTENCODING_DONTKNOW, "VbaProject::importVba - unknown text encoding" );
                if( eNewTextEnc != RTL_TEXTENCODING_DONTKNOW )
                    eTextEnc = eNewTextEnc;
            }
            break;
            case VBA_ID_PROJECTNAME:
            {
                OUString aPrjName = aRecStrm.readCharArrayUC( nRecSize, eTextEnc );
                OSL_ENSURE( !aPrjName.isEmpty(), "VbaProject::importVba - invalid project name" );
                if( !aPrjName.isEmpty() )
                    maPrjName = aPrjName;
            }
            break;
            case VBA_ID_PROJECTMODULES:
                OSL_ENSURE( nRecSize == 2, "VbaProject::importVba - invalid record size" );
                OSL_ENSURE( maModules.empty(), "VbaProject::importVba - unexpected PROJECTMODULES record" );
                nModuleCount = aRecStrm.readuInt16();
            break;
            case VBA_ID_MODULENAME:
            {
                OUString aName = aRecStrm.readCharArrayUC( nRecSize, eTextEnc );
                OSL_ENSURE( !aName.isEmpty(), "VbaProject::importVba - invalid module name" );
                OSL_ENSURE( !maModules.has( aName ), "VbaProject::importVba - multiple modules with the same name" );
                VbaModuleMap::mapped_type& rxModule = maModules[ aName ];
                rxModule.reset( new VbaModule( mxContext, mxDocModel, aName, eTextEnc, bExecutable ) );
                // read all remaining records until the MODULEEND record
                rxModule->importDirRecords( aDirStrm );
                OSL_ENSURE( !maModulesByStrm.has( rxModule->getStreamName() ), "VbaProject::importVba - multiple modules with the same stream name" );
                maModulesByStrm[ rxModule->getStreamName() ] = rxModule;
            }
            break;
        }
    }
    SAL_WARN_IF( nModuleCount != maModules.size(), "oox", "VbaProject::importVba - invalid module count" );

    /*  The directory does not contain the real type of the modules, it
        distinguishes only between 'procedural' and 'document' (the latter
        includes class and form modules). Now, the exact type of all modules
        will be read from the 'PROJECT' stream. It consists of text lines in
        'key=value' format which list the code modules by type.

        -   The line 'document=<modulename>/&HXXXXXXXX' declares document
            modules. These are attached to the Word document (usually called
            'ThisDocument'), the Excel workbook (usually called
            'ThisWorkbook'), or single Excel worksheets or chartsheets (usually
            called 'SheetX' or 'ChartX', X being a decimal number). Of course,
            users may rename all these modules. The slash character separates
            an automation server version number (hexadecimal 'XXXXXXXX') from
            the module name.
        -   The line 'Module=<modulename>' declares common procedural code
            modules.
        -   The line 'Class=<modulename>' declares a class module.
        -   The line 'BaseClass=<modulename>' declares a code module attached
            to a user form with the same name.
     */
    BinaryXInputStream aPrjStrm( rVbaPrjStrg.openInputStream( "PROJECT" ), true );
    OSL_ENSURE( !aPrjStrm.isEof(), "VbaProject::importVba - cannot open 'PROJECT' stream" );
    // do not exit if this stream does not exist, but proceed to load the modules below
    if( !aPrjStrm.isEof() )
    {
        TextInputStream aPrjTextStrm( mxContext, aPrjStrm, eTextEnc );
        OUString aKey, aValue;
        bool bExitLoop = false;
        while( !bExitLoop && !aPrjTextStrm.isEof() )
        {
            // read a text line from the stream
            OUString aLine = aPrjTextStrm.readLine().trim();
            sal_Int32 nLineLen = aLine.getLength();
            // exit if a subsection starts (section name is given in brackets)
            bExitLoop = (nLineLen >= 2) && (aLine[ 0 ] == '[') && (aLine[ nLineLen - 1 ] == ']');
            if( !bExitLoop && VbaHelper::extractKeyValue( aKey, aValue, aLine ) )
            {
                sal_Int32 nType = ModuleType::UNKNOWN;
                if( aKey.equalsIgnoreAsciiCase( "Document" ) )
                {
                    nType = ModuleType::DOCUMENT;
                    // strip automation server version from module names
                    sal_Int32 nSlashPos = aValue.indexOf( '/' );
                    if( nSlashPos >= 0 )
                        aValue = aValue.copy( 0, nSlashPos );
                }
                else if( aKey.equalsIgnoreAsciiCase( "Module" ) )
                    nType = ModuleType::NORMAL;
                else if( aKey.equalsIgnoreAsciiCase( "Class" ) )
                    nType = ModuleType::CLASS;
                else if( aKey.equalsIgnoreAsciiCase( "BaseClass" ) )
                    nType = ModuleType::FORM;

                if( (nType != ModuleType::UNKNOWN) && !aValue.isEmpty() )
                {
                    OSL_ENSURE( maModules.has( aValue ), "VbaProject::importVba - module not found" );
                    if( VbaModule* pModule = maModules.get( aValue ).get() )
                        pModule->setType( nType );
                }
            }
        }
    }
    if( !maModules.empty() ) try
    {
        /*  Set library container to VBA compatibility mode. This will create
            the VBA Globals object and store it in the Basic manager of the
            document. */
        try
        {
            Reference< XVBACompatibility > xVBACompat( getLibraryContainer( PROP_BasicLibraries ), UNO_QUERY_THROW );
            xVBACompat->setVBACompatibilityMode( true );
            xVBACompat->setProjectName( maPrjName );

        }
        catch(const Exception& )
        {
        }
    }
    catch(const Exception& )
    {
    }
}

void VbaProject::importModulesAndForms( StorageBase& rVbaPrjStrg, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr )
{
    StorageRef xVbaStrg = rVbaPrjStrg.openSubStorage( "VBA", false );
    OSL_ENSURE( xVbaStrg.get(), "VbaProject::importModulesAndForms - cannot open 'VBA' substorage" );
    if( !xVbaStrg )
        return;
    rtl_TextEncoding eTextEnc = RTL_TEXTENCODING_MS_1252;
    bool bExecutable = isImportVbaExecutable();

    // create empty dummy modules
    VbaModuleMap aDummyModules;
    for( DummyModuleMap::iterator aIt = maDummyModules.begin(), aEnd = maDummyModules.end(); aIt != aEnd; ++aIt )
    {
        OSL_ENSURE( !maModules.has( aIt->first ) && !aDummyModules.has( aIt->first ), "VbaProject::importVba - multiple modules with the same name" );
        VbaModuleMap::mapped_type& rxModule = aDummyModules[ aIt->first ];
        rxModule.reset( new VbaModule( mxContext, mxDocModel, aIt->first, eTextEnc, bExecutable ) );
        rxModule->setType( aIt->second );
    }

    /*  Now it is time to load the source code. All modules will be inserted
        into the Basic library of the document specified by the 'maPrjName'
        member. Do not create the Basic library, if there are no modules
        specified. */
    if( !maModules.empty() || !aDummyModules.empty() ) try
    {
        // get the model factory and the basic library
        Reference< XMultiServiceFactory > xModelFactory( mxDocModel, UNO_QUERY_THROW );
        Reference< XNameContainer > xBasicLib( createBasicLibrary(), UNO_SET_THROW );

        // try to get access to document objects related to code modules
        Reference< XNameAccess > xDocObjectNA;
        try
        {
            xDocObjectNA.set( xModelFactory->createInstance( "ooo.vba.VBAObjectModuleObjectProvider" ), UNO_QUERY );
        }
        catch(const Exception& )
        {
            // not all documents support this
        }

        if( xBasicLib.is() )
        {
            // #TODO cater for mxOleOverridesSink, like I used to before
            // call Basic source code import for each module, std::[c]ref enforces pass-by-ref
            maModules.forEachMem( &VbaModule::createAndImportModule,
                ::std::ref( *xVbaStrg ), ::std::cref( xBasicLib ),
                ::std::cref( xDocObjectNA ) );

            // create empty dummy modules
            aDummyModules.forEachMem( &VbaModule::createEmptyModule,
                ::std::cref( xBasicLib ), ::std::cref( xDocObjectNA ) );
        }
    }
    catch(const Exception& )
    {
    }

    /*  Load the forms. The file format specification requires that a module
        must exist for every form. We are a bit more tolerant and scan the
        project storage for all form substorages. This may 'repair' broken VBA
        storages that misses to mention a module for an existing form. */
    ::std::vector< OUString > aElements;
    rVbaPrjStrg.getElementNames( aElements );
    for( ::std::vector< OUString >::iterator aIt = aElements.begin(), aEnd = aElements.end(); aIt != aEnd; ++aIt )
    {
        // try to open the element as storage
        if( *aIt != "VBA" )
        {
            StorageRef xSubStrg = rVbaPrjStrg.openSubStorage( *aIt, false );
            if( xSubStrg.get() ) try
            {
                // resolve module name from storage name (which equals the module stream name)
                VbaModule* pModule = maModulesByStrm.get( *aIt ).get();
                OSL_ENSURE( pModule && (pModule->getType() == ModuleType::FORM),
                    "VbaProject::importVba - form substorage without form module" );
                OUString aModuleName;
                if( pModule )
                    aModuleName = pModule->getName();

                // create and import the form
                Reference< XNameContainer > xDialogLib( createDialogLibrary(), UNO_SET_THROW );
                VbaUserForm aForm( mxContext, mxDocModel, rGraphicHelper, bDefaultColorBgr );
                aForm.importForm( xDialogLib, *xSubStrg, aModuleName, eTextEnc );
            }
            catch(const Exception& )
            {
            }
        }
    }

    // attach macros to registered objects
    attachMacros();
}

void VbaProject::attachMacros()
{
    if( !maMacroAttachers.empty() && mxContext.is() ) try
    {
        Reference< XMultiComponentFactory > xFactory( mxContext->getServiceManager(), UNO_SET_THROW );
        Sequence< Any > aArgs( 2 );
        aArgs[ 0 ] <<= mxDocModel;
        aArgs[ 1 ] <<= maPrjName;
        Reference< XVBAMacroResolver > xResolver( xFactory->createInstanceWithArgumentsAndContext(
            "com.sun.star.script.vba.VBAMacroResolver", aArgs, mxContext ), UNO_QUERY_THROW );
        maMacroAttachers.forEachMem( &VbaMacroAttacherBase::resolveAndAttachMacro, ::std::cref( xResolver ) );
    }
    catch(const Exception& )
    {
    }
}

void VbaProject::copyStorage( StorageBase& rVbaPrjStrg )
{
    if( mxContext.is() ) try
    {
        Reference< XStorageBasedDocument > xStorageBasedDoc( mxDocModel, UNO_QUERY_THROW );
        Reference< XStorage > xDocStorage( xStorageBasedDoc->getDocumentStorage(), UNO_QUERY_THROW );
        {
            const sal_Int32 nOpenMode = ElementModes::SEEKABLE | ElementModes::WRITE | ElementModes::TRUNCATE;
            Reference< XStream > xDocStream( xDocStorage->openStreamElement( "_MS_VBA_Macros", nOpenMode ), UNO_SET_THROW );
            OleStorage aDestStorage( mxContext, xDocStream, false );
            rVbaPrjStrg.copyStorageToStorage( aDestStorage );
            aDestStorage.commit();
        }
        Reference< XTransactedObject >( xDocStorage, UNO_QUERY_THROW )->commit();
    }
    catch(const Exception& )
    {
    }
}

} // namespace ole
} // namespace oox

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
