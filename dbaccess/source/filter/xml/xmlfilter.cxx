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

#include <config_features.h>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#if HAVE_FEATURE_JAVA
#include <jvmaccess/virtualmachine.hxx>
#endif
#include "xmlfilter.hxx"
#include "flt_reghelper.hxx"
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlscripti.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/nmspmap.hxx>
#include <com/sun/star/xml/sax/InputSource.hpp>
#include <com/sun/star/xml/sax/Parser.hpp>
#include <xmloff/ProgressBarHelper.hxx>
#include <sfx2/docfile.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XNamingService.hpp>
#include "xmlDatabase.hxx"
#include "xmlEnums.hxx"
#include "xmlstrings.hrc"
#include <xmloff/DocumentSettingsContext.hxx>
#include "xmlStyleImport.hxx"
#include <xmloff/xmluconv.hxx>
#include "xmlHelper.hxx"
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/lang/WrappedTargetRuntimeException.hpp>
#include <osl/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <unotools/moduleoptions.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <comphelper/processfactory.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <osl/thread.hxx>
#include <connectivity/CommonTools.hxx>
#include <connectivity/DriversConfig.hxx>
#include "dsntypes.hxx"
#include <rtl/strbuf.hxx>

using namespace ::com::sun::star;

extern "C" void SAL_CALL createRegistryInfo_ODBFilter( )
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::ODBFilter > aAutoRegistration;
}
//--------------------------------------------------------------------------
namespace dbaxml
{
    namespace
    {
        class FastLoader : public ::osl::Thread
        {
        public:
            typedef enum { E_JAVA, E_CALC } StartType;
            FastLoader(uno::Reference< uno::XComponentContext > const & _xContext,StartType _eType)
                :m_xContext(_xContext)
                ,m_eWhat(_eType)
            {}

        protected:
            virtual ~FastLoader(){}

            /// Working method which should be overridden.
            virtual void SAL_CALL run();
            virtual void SAL_CALL onTerminated();
        private:
            uno::Reference< uno::XComponentContext > m_xContext;
            StartType m_eWhat;
        };

        void SAL_CALL FastLoader::run()
        {
            if ( m_eWhat == E_JAVA )
            {
#if HAVE_FEATURE_JAVA
                static bool s_bFirstTime = true;
                if ( s_bFirstTime )
                {
                    s_bFirstTime = false;
                    try
                    {
                        ::rtl::Reference< jvmaccess::VirtualMachine > xJVM = ::connectivity::getJavaVM(m_xContext);
                    }
                    catch (const uno::Exception&)
                    {
                        OSL_ASSERT(0);
                    }
                }
#endif
            }
            else if ( m_eWhat == E_CALC )
            {
                static bool s_bFirstTime = true;
                if ( s_bFirstTime )
                {
                    s_bFirstTime = false;
                    try
                    {
                        uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create( m_xContext );
                        const OUString sTarget("_blank");
                        sal_Int32 nFrameSearchFlag = frame::FrameSearchFlag::TASKS | frame::FrameSearchFlag::CREATE;
                        uno::Reference< frame::XFrame> xFrame = xDesktop->findFrame(sTarget,nFrameSearchFlag);
                        uno::Reference<frame::XComponentLoader> xFrameLoad(xFrame,uno::UNO_QUERY);

                        if ( xFrameLoad.is() )
                        {
                            uno::Sequence < beans::PropertyValue > aArgs( 3);
                            sal_Int32 nLen = 0;
                            aArgs[nLen].Name = OUString("AsTemplate");
                            aArgs[nLen++].Value <<= sal_False;

                            aArgs[nLen].Name = OUString("ReadOnly");
                            aArgs[nLen++].Value <<= sal_True;

                            aArgs[nLen].Name = OUString("Hidden");
                            aArgs[nLen++].Value <<= sal_True;

                            ::comphelper::MimeConfigurationHelper aHelper( m_xContext );
                            SvtModuleOptions aModuleOptions;
                            uno::Reference< frame::XModel > xModel(xFrameLoad->loadComponentFromURL(
                                aModuleOptions.GetFactoryEmptyDocumentURL( aModuleOptions.ClassifyFactoryByServiceName( aHelper.GetDocServiceNameFromMediaType(MIMETYPE_OASIS_OPENDOCUMENT_SPREADSHEET) )),
                                OUString(), // empty frame name
                                0,
                                aArgs
                                ),uno::UNO_QUERY);
                            ::comphelper::disposeComponent(xModel);
                        }
                    }
                    catch (const uno::Exception&)
                    {
                        OSL_ASSERT(0);
                    }
                }
            }
        }
        void SAL_CALL FastLoader::onTerminated()
        {
            delete this;
        }

        class DatasourceURLListener : public ::cppu::WeakImplHelper1< beans::XPropertyChangeListener >
        {
            uno::Reference< uno::XComponentContext > m_xContext;
            ::dbaccess::ODsnTypeCollection m_aTypeCollection;
            DatasourceURLListener(const DatasourceURLListener&);
            void operator =(const DatasourceURLListener&);
        protected:
            virtual ~DatasourceURLListener(){}
        public:
            DatasourceURLListener(uno::Reference< uno::XComponentContext > const & _xContext) : m_xContext(_xContext), m_aTypeCollection(_xContext){}
            // XPropertyChangeListener
            virtual void SAL_CALL propertyChange( const beans::PropertyChangeEvent& _rEvent ) throw (uno::RuntimeException)
            {
                OUString sURL;
                _rEvent.NewValue >>= sURL;
                FastLoader* pCreatorThread = NULL;

                if ( m_aTypeCollection.needsJVM(sURL) )
                {
#if HAVE_FEATURE_JAVA
                    pCreatorThread = new FastLoader(m_xContext, FastLoader::E_JAVA);
#endif
                }
                else if ( sURL.matchIgnoreAsciiCaseAsciiL("sdbc:calc:",10,0) )
                {
                    pCreatorThread = new FastLoader(m_xContext, FastLoader::E_CALC);
                }
                if ( pCreatorThread )
                {
                    pCreatorThread->createSuspended();
                    pCreatorThread->setPriority(osl_Thread_PriorityBelowNormal);
                    pCreatorThread->resume();
                }
            }
            // XEventListener
            virtual void SAL_CALL disposing( const lang::EventObject& /*_rSource*/ ) throw (uno::RuntimeException)
            {
            }
        };
    }
    sal_Char const sXML_np__db[] = "_db";
    sal_Char const sXML_np___db[] = "__db";

    using namespace ::com::sun::star::util;
    /// read a component (file + filter version)
sal_Int32 ReadThroughComponent(
    const uno::Reference<XInputStream>& xInputStream,
    const uno::Reference<XComponent>& xModelComponent,
    const uno::Reference<XComponentContext> & rxContext,
    const uno::Reference< XDocumentHandler >& _xFilter )
{
    OSL_ENSURE(xInputStream.is(), "input stream missing");
    OSL_ENSURE(xModelComponent.is(), "document missing");
    OSL_ENSURE(rxContext.is(), "factory missing");

   SAL_INFO("dbaccess", "dbaxml ReadThroughComponent" );

    // prepare ParserInputSrouce
    InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get parser
    uno::Reference< XParser > xParser = Parser::create(rxContext);
    SAL_INFO("dbaccess", "parser created" );

    // get filter
    OSL_ENSURE( _xFilter.is(), "Can't instantiate filter component." );
    if( !_xFilter.is() )
        return 1;

    // connect parser and filter
    xParser->setDocumentHandler( _xFilter );

    // connect model and filter
    uno::Reference < XImporter > xImporter( _xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );



    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );
    }
    catch (const SAXParseException& r)
    {
#if OSL_DEBUG_LEVEL > 1
        SAL_WARN("dbaccess", "SAX parse exception catched while importing:\n" << r.Message << r.LineNumber << "," << r.ColumnNumber);
#else
        (void)r;
#endif
        return 1;
    }
    catch (const SAXException&)
    {
        return 1;
    }
    catch (const packages::zip::ZipIOException&)
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch (const Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }

    // success!
    return 0;
}

/// read a component (storage version)
sal_Int32 ReadThroughComponent(
    uno::Reference< embed::XStorage > xStorage,
    const uno::Reference<XComponent>& xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    const uno::Reference<XComponentContext> & rxContext,
    const uno::Reference< XDocumentHandler >& _xFilter)
{
    OSL_ENSURE( xStorage.is(), "Need storage!");
    OSL_ENSURE(NULL != pStreamName, "Please, please, give me a name!");

    if ( xStorage.is() )
    {
        uno::Reference< io::XStream > xDocStream;
        sal_Bool bEncrypted = sal_False;

        try
        {
            // open stream (and set parser input)
        OUString sStreamName = OUString::createFromAscii(pStreamName);
            if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
            {
                // stream name not found! Then try the compatibility name.
                // if no stream can be opened, return immediately with OK signal

                // do we even have an alternative name?
                if ( NULL == pCompatibilityStreamName )
                    return 0;

                // if so, does the stream exist?
                sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
                if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
                    return 0;
            }

            // get input stream
            xDocStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );

            uno::Reference< beans::XPropertySet > xProps( xDocStream, uno::UNO_QUERY_THROW );
            uno::Any aAny = xProps->getPropertyValue("Encrypted");
            aAny >>= bEncrypted;
        }
        catch (const packages::WrongPasswordException&)
        {
            return ERRCODE_SFX_WRONGPASSWORD;
        }
        catch (const uno::Exception&)
        {
            return 1; // TODO/LATER: error handling
        }

#ifdef TIMELOG
        // if we do profiling, we want to know the stream
       SAL_INFO("dbaccess", "dbaxml ReadThroughComponent : parsing \"" << pStreamName << "\"" );
#endif

        uno::Reference< XInputStream > xInputStream = xDocStream->getInputStream();
        // read from the stream
        return ReadThroughComponent( xInputStream
                                    ,xModelComponent
                                    ,rxContext
                                    ,_xFilter );
    }

    // TODO/LATER: better error handling
    return 1;
}

// -------------
// - ODBFilter -
// -------------
DBG_NAME(ODBFilter)

ODBFilter::ODBFilter( const uno::Reference< XComponentContext >& _rxContext )
    :SvXMLImport(_rxContext)
    ,m_bNewFormat(false)
{
    DBG_CTOR(ODBFilter,NULL);

    GetMM100UnitConverter().SetCoreMeasureUnit(util::MeasureUnit::MM_10TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(util::MeasureUnit::CM);
    GetNamespaceMap().Add( OUString ( sXML_np__db ),
                        GetXMLToken(XML_N_DB),
                        XML_NAMESPACE_DB );

    GetNamespaceMap().Add( OUString ( sXML_np___db ),
                        GetXMLToken(XML_N_DB_OASIS),
                        XML_NAMESPACE_DB );
}

// -----------------------------------------------------------------------------

ODBFilter::~ODBFilter() throw()
{

    DBG_DTOR(ODBFilter,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(ODBFilter, "com.sun.star.comp.sdb.DBFilter")
IMPLEMENT_SERVICE_INFO_SUPPORTS(ODBFilter)
IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(ODBFilter, "com.sun.star.document.ImportFilter")

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL ODBFilter::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
{
    return static_cast< XServiceInfo* >(new ODBFilter( comphelper::getComponentContext(_rxORB)));
}

// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODBFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    uno::Reference< ::com::sun::star::awt::XWindow > xWindow;
    {
        SolarMutexGuard aGuard;
        Window*     pFocusWindow = Application::GetFocusWindow();
        xWindow = VCLUnoHelper::GetInterface( pFocusWindow );
        if( pFocusWindow )
            pFocusWindow->EnterWait();
    }
    sal_Bool    bRet = sal_False;

    if ( GetModel().is() )
        bRet = implImport( rDescriptor );

    if ( xWindow.is() )
    {
        SolarMutexGuard aGuard;
        Window* pFocusWindow = VCLUnoHelper::GetWindow( xWindow );
        if ( pFocusWindow )
            pFocusWindow->LeaveWait();
    }


    return bRet;
}
// -----------------------------------------------------------------------------
sal_Bool ODBFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    OUString sFileName;
    ::comphelper::NamedValueCollection aMediaDescriptor( rDescriptor );
    if ( aMediaDescriptor.has( "URL" ) )
        sFileName = aMediaDescriptor.getOrDefault( "URL", OUString() );
    if ( sFileName.isEmpty() && aMediaDescriptor.has( "FileName" ) )
        sFileName = aMediaDescriptor.getOrDefault( "FileName", sFileName );

    OSL_ENSURE( !sFileName.isEmpty(), "ODBFilter::implImport: no URL given!" );
    sal_Bool bRet = !sFileName.isEmpty();

    if ( bRet )
    {
        uno::Reference<XComponent> xCom(GetModel(),UNO_QUERY);

        SfxMediumRef pMedium = new SfxMedium(
                sFileName, ( STREAM_READ | STREAM_NOCREATE ) );
        uno::Reference< embed::XStorage > xStorage;
        try
        {
            xStorage.set( pMedium->GetStorage( sal_False ), UNO_QUERY_THROW );
        }
        catch (const Exception&)
        {
            Any aError = ::cppu::getCaughtException();
            if  ( aError.isExtractableTo( ::cppu::UnoType< RuntimeException >::get() ) )
                throw;
            throw lang::WrappedTargetRuntimeException( OUString(), *this, aError );
        }

        uno::Reference<sdb::XOfficeDatabaseDocument> xOfficeDoc(GetModel(),UNO_QUERY_THROW);
        m_xDataSource.set(xOfficeDoc->getDataSource(),UNO_QUERY_THROW);
        uno::Reference<beans::XPropertyChangeListener> xListener = new DatasourceURLListener( GetComponentContext());
        m_xDataSource->addPropertyChangeListener(PROPERTY_URL,xListener);
        uno::Reference< XNumberFormatsSupplier > xNum(m_xDataSource->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
        SetNumberFormatsSupplier(xNum);

        uno::Reference<XComponent> xModel(GetModel(),UNO_QUERY);
        sal_Int32 nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"settings.xml"
                                    ,"Settings.xml"
                                    ,GetComponentContext()
                                    ,this
                                    );

        if ( nRet == 0 )
            nRet = ReadThroughComponent( xStorage
                                    ,xModel
                                    ,"content.xml"
                                    ,"Content.xml"
                                    ,GetComponentContext()
                                    ,this
                                    );

        bRet = nRet == 0;

        if ( bRet )
        {
            uno::Reference< XModifiable > xModi(GetModel(),UNO_QUERY);
            if ( xModi.is() )
                xModi->setModified(sal_False);
        }
        else
        {
            switch( nRet )
            {
                case ERRCODE_IO_BROKENPACKAGE:
                    // TODO/LATER: no way to transport the error outside from the filter!
                    break;
                default:
                {
                    // TODO/LATER: this is completely wrong! Filter code should never call ErrorHandler directly! But for now this is the only way!
                    ErrorHandler::HandleError( nRet );
                    if( nRet & ERRCODE_WARNING_MASK )
                        bRet = sal_True;
                }
            }
        }
    }

    return bRet;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateContext( sal_uInt16 nPrefix,
                                      const OUString& rLocalName,
                                      const uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = 0;

    const SvXMLTokenMap& rTokenMap = GetDocElemTokenMap();
    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_DOC_SETTINGS:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new XMLDocumentSettingsContext( *this, nPrefix, rLocalName,xAttrList );
            break;
        case XML_TOK_DOC_DATABASE:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDatabase( *this, nPrefix, rLocalName );
            break;
        case XML_TOK_DOC_STYLES:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateStylesContext(nPrefix, rLocalName, xAttrList, sal_False);
            break;
        case XML_TOK_DOC_AUTOSTYLES:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateStylesContext(nPrefix, rLocalName, xAttrList, sal_True);
            break;
        case XML_TOK_DOC_SCRIPT:
            pContext = CreateScriptContext( rLocalName );
            break;
    }

    if ( !pContext )
        pContext = SvXMLImport::CreateContext( nPrefix, rLocalName, xAttrList );

    return pContext;
}
// -----------------------------------------------------------------------------
void ODBFilter::SetViewSettings(const Sequence<PropertyValue>& aViewProps)
{
    const PropertyValue *pIter = aViewProps.getConstArray();
    const PropertyValue *pEnd = pIter + aViewProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name == "Queries" )
        {
            fillPropertyMap(pIter->Value,m_aQuerySettings);
        }
        else if ( pIter->Name == "Tables" )
        {
            fillPropertyMap(pIter->Value,m_aTablesSettings);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBFilter::SetConfigurationSettings(const Sequence<PropertyValue>& aConfigProps)
{
    const PropertyValue *pIter = aConfigProps.getConstArray();
    const PropertyValue *pEnd = pIter + aConfigProps.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        if ( pIter->Name == "layout-settings" )
        {
            Sequence<PropertyValue> aWindows;
            pIter->Value >>= aWindows;
            uno::Reference<XPropertySet> xProp(getDataSource());
            if ( xProp.is() )
                xProp->setPropertyValue(PROPERTY_LAYOUTINFORMATION,makeAny(aWindows));
        }
    }
}
// -----------------------------------------------------------------------------
void ODBFilter::fillPropertyMap(const Any& _rValue,TPropertyNameMap& _rMap)
{
    Sequence<PropertyValue> aWindows;
    _rValue >>= aWindows;
    const PropertyValue *pIter = aWindows.getConstArray();
    const PropertyValue *pEnd = pIter + aWindows.getLength();
    for (; pIter != pEnd; ++pIter)
    {
        Sequence<PropertyValue> aValue;
        pIter->Value >>= aValue;
        _rMap.insert(TPropertyNameMap::value_type(pIter->Name,aValue));
    }

}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetDocElemTokenMap() const
{
    if ( !m_pDocElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS    },
            { XML_NAMESPACE_OOO,    XML_SETTINGS,           XML_TOK_DOC_SETTINGS    },
            { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OOO,    XML_STYLES,             XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OOO,    XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OFFICE, XML_DATABASE,           XML_TOK_DOC_DATABASE    },
            { XML_NAMESPACE_OOO,    XML_DATABASE,           XML_TOK_DOC_DATABASE    },
            { XML_NAMESPACE_OFFICE, XML_SCRIPTS,            XML_TOK_DOC_SCRIPT      },
            XML_TOKEN_MAP_END
        };
        m_pDocElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDocElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetDatabaseElemTokenMap() const
{
    if ( !m_pDatabaseElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_DATASOURCE,             XML_TOK_DATASOURCE  },
            { XML_NAMESPACE_DB, XML_FORMS,                  XML_TOK_FORMS},
            { XML_NAMESPACE_DB, XML_REPORTS,                XML_TOK_REPORTS},
            { XML_NAMESPACE_DB, XML_QUERIES,                XML_TOK_QUERIES},
            { XML_NAMESPACE_DB, XML_TABLES,                 XML_TOK_TABLES},
            { XML_NAMESPACE_DB, XML_TABLE_REPRESENTATIONS,  XML_TOK_TABLES},
            { XML_NAMESPACE_DB, XML_SCHEMA_DEFINITION,      XML_TOK_SCHEMA_DEFINITION},
            XML_TOKEN_MAP_END
        };
        m_pDatabaseElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDatabaseElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetDataSourceElemTokenMap() const
{
    if ( !m_pDataSourceElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB,     XML_CONNECTION_RESOURCE,            XML_TOK_CONNECTION_RESOURCE},
            { XML_NAMESPACE_DB,     XML_SUPPRESS_VERSION_COLUMNS,       XML_TOK_SUPPRESS_VERSION_COLUMNS},
            { XML_NAMESPACE_DB,     XML_JAVA_DRIVER_CLASS,              XML_TOK_JAVA_DRIVER_CLASS},
            { XML_NAMESPACE_DB,     XML_EXTENSION,                      XML_TOK_EXTENSION},
            { XML_NAMESPACE_DB,     XML_IS_FIRST_ROW_HEADER_LINE,       XML_TOK_IS_FIRST_ROW_HEADER_LINE},
            { XML_NAMESPACE_DB,     XML_SHOW_DELETED,                   XML_TOK_SHOW_DELETED},
            { XML_NAMESPACE_DB,     XML_IS_TABLE_NAME_LENGTH_LIMITED,   XML_TOK_IS_TABLE_NAME_LENGTH_LIMITED},
            { XML_NAMESPACE_DB,     XML_SYSTEM_DRIVER_SETTINGS,         XML_TOK_SYSTEM_DRIVER_SETTINGS},
            { XML_NAMESPACE_DB,     XML_ENABLE_SQL92_CHECK,             XML_TOK_ENABLE_SQL92_CHECK},
            { XML_NAMESPACE_DB,     XML_APPEND_TABLE_ALIAS_NAME,        XML_TOK_APPEND_TABLE_ALIAS_NAME},
            { XML_NAMESPACE_DB,     XML_PARAMETER_NAME_SUBSTITUTION,    XML_TOK_PARAMETER_NAME_SUBSTITUTION},
            { XML_NAMESPACE_DB,     XML_IGNORE_DRIVER_PRIVILEGES,       XML_TOK_IGNORE_DRIVER_PRIVILEGES},
            { XML_NAMESPACE_DB,     XML_BOOLEAN_COMPARISON_MODE,        XML_TOK_BOOLEAN_COMPARISON_MODE},
            { XML_NAMESPACE_DB,     XML_USE_CATALOG,                    XML_TOK_USE_CATALOG},
            { XML_NAMESPACE_DB,     XML_BASE_DN,                        XML_TOK_BASE_DN},
            { XML_NAMESPACE_DB,     XML_MAX_ROW_COUNT,                  XML_TOK_MAX_ROW_COUNT},
            { XML_NAMESPACE_DB,     XML_LOGIN,                          XML_TOK_LOGIN},
            { XML_NAMESPACE_DB,     XML_TABLE_FILTER,                   XML_TOK_TABLE_FILTER},
            { XML_NAMESPACE_DB,     XML_TABLE_TYPE_FILTER,              XML_TOK_TABLE_TYPE_FILTER},
            { XML_NAMESPACE_DB,     XML_AUTO_INCREMENT,                 XML_TOK_AUTO_INCREMENT},
            { XML_NAMESPACE_DB,     XML_DELIMITER,                      XML_TOK_DELIMITER},
            { XML_NAMESPACE_DB,     XML_DATA_SOURCE_SETTINGS,           XML_TOK_DATA_SOURCE_SETTINGS},
            { XML_NAMESPACE_DB,     XML_FONT_CHARSET,                   XML_TOK_FONT_CHARSET},
            // db odf 12
            { XML_NAMESPACE_DB,     XML_CONNECTION_DATA,                XML_TOK_CONNECTION_DATA},
            { XML_NAMESPACE_DB,     XML_DATABASE_DESCRIPTION,           XML_TOK_DATABASE_DESCRIPTION},
            { XML_NAMESPACE_DB,     XML_COMPOUND_DATABASE,              XML_TOK_COMPOUND_DATABASE},
            { XML_NAMESPACE_XLINK,  XML_HREF,                           XML_TOK_DB_HREF},
            { XML_NAMESPACE_DB,     XML_MEDIA_TYPE,                     XML_TOK_MEDIA_TYPE},
            { XML_NAMESPACE_DB,     XML_TYPE,                           XML_TOK_DB_TYPE},
            { XML_NAMESPACE_DB,     XML_HOSTNAME,                       XML_TOK_HOSTNAME},
            { XML_NAMESPACE_DB,     XML_PORT,                           XML_TOK_PORT},
            { XML_NAMESPACE_DB,     XML_LOCAL_SOCKET,                   XML_TOK_LOCAL_SOCKET},
            { XML_NAMESPACE_DB,     XML_DATABASE_NAME,                  XML_TOK_DATABASE_NAME},
            { XML_NAMESPACE_DB,     XML_DRIVER_SETTINGS,                XML_TOK_DRIVER_SETTINGS},
            { XML_NAMESPACE_DB,     XML_JAVA_CLASSPATH,                 XML_TOK_JAVA_CLASSPATH},
            { XML_NAMESPACE_DB,     XML_CHARACTER_SET,                  XML_TOK_CHARACTER_SET},
            { XML_NAMESPACE_DB,     XML_APPLICATION_CONNECTION_SETTINGS,XML_TOK_APPLICATION_CONNECTION_SETTINGS},
            XML_TOKEN_MAP_END
        };
        m_pDataSourceElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDataSourceElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetLoginElemTokenMap() const
{
    if ( !m_pLoginElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_USER_NAME,              XML_TOK_USER_NAME},
            { XML_NAMESPACE_DB, XML_IS_PASSWORD_REQUIRED,   XML_TOK_IS_PASSWORD_REQUIRED},
            { XML_NAMESPACE_DB, XML_USE_SYSTEM_USER,        XML_TOK_USE_SYSTEM_USER},
            { XML_NAMESPACE_DB, XML_LOGIN_TIMEOUT,          XML_TOK_LOGIN_TIMEOUT},
            XML_TOKEN_MAP_END
        };
        m_pLoginElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pLoginElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetDatabaseDescriptionElemTokenMap() const
{
    if ( !m_pDatabaseDescriptionElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_FILE_BASED_DATABASE,    XML_TOK_FILE_BASED_DATABASE},
            { XML_NAMESPACE_DB, XML_SERVER_DATABASE,        XML_TOK_SERVER_DATABASE},
            XML_TOKEN_MAP_END
        };
        m_pDatabaseDescriptionElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDatabaseDescriptionElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetDataSourceInfoElemTokenMap() const
{
    if ( !m_pDataSourceInfoElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_ADDITIONAL_COLUMN_STATEMENT,XML_TOK_ADDITIONAL_COLUMN_STATEMENT},
            { XML_NAMESPACE_DB, XML_ROW_RETRIEVING_STATEMENT,   XML_TOK_ROW_RETRIEVING_STATEMENT},
            { XML_NAMESPACE_DB, XML_STRING,                     XML_TOK_STRING},
            { XML_NAMESPACE_DB, XML_FIELD,                      XML_TOK_FIELD},
            { XML_NAMESPACE_DB, XML_DECIMAL,                    XML_TOK_DECIMAL},
            { XML_NAMESPACE_DB, XML_THOUSAND,                   XML_TOK_THOUSAND},
            { XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING,        XML_TOK_DATA_SOURCE_SETTING},
            { XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE,  XML_TOK_DATA_SOURCE_SETTING_VALUE},
            { XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_IS_LIST,XML_TOK_DATA_SOURCE_SETTING_IS_LIST},
            { XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_TYPE,   XML_TOK_DATA_SOURCE_SETTING_TYPE},
            { XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_NAME,   XML_TOK_DATA_SOURCE_SETTING_NAME},
            { XML_NAMESPACE_DB, XML_FONT_CHARSET,               XML_TOK_FONT_CHARSET},
            { XML_NAMESPACE_DB, XML_ENCODING,                   XML_TOK_ENCODING},
            XML_TOKEN_MAP_END
        };
        m_pDataSourceInfoElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDataSourceInfoElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetDocumentsElemTokenMap() const
{
    if ( !m_pDocumentsElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_COMPONENT,              XML_TOK_COMPONENT},
            { XML_NAMESPACE_DB, XML_COMPONENT_COLLECTION,   XML_TOK_COMPONENT_COLLECTION},
            { XML_NAMESPACE_DB, XML_QUERY_COLLECTION,       XML_TOK_QUERY_COLLECTION},
            { XML_NAMESPACE_DB, XML_QUERY,                  XML_TOK_QUERY},
            { XML_NAMESPACE_DB, XML_TABLE,                  XML_TOK_TABLE},
            { XML_NAMESPACE_DB, XML_TABLE_REPRESENTATION,   XML_TOK_TABLE},
            { XML_NAMESPACE_DB, XML_COLUMN,                 XML_TOK_COLUMN},
            XML_TOKEN_MAP_END
        };
        m_pDocumentsElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pDocumentsElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetComponentElemTokenMap() const
{
    if ( !m_pComponentElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_XLINK,  XML_HREF,           XML_TOK_HREF    },
            { XML_NAMESPACE_XLINK,  XML_TYPE,           XML_TOK_TYPE    },
            { XML_NAMESPACE_XLINK,  XML_SHOW,           XML_TOK_SHOW    },
            { XML_NAMESPACE_XLINK,  XML_ACTUATE,        XML_TOK_ACTUATE},
            { XML_NAMESPACE_DB, XML_AS_TEMPLATE,    XML_TOK_AS_TEMPLATE },
            { XML_NAMESPACE_DB, XML_NAME,           XML_TOK_COMPONENT_NAME  },
            XML_TOKEN_MAP_END
        };
        m_pComponentElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pComponentElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetQueryElemTokenMap() const
{
    if ( !m_pQueryElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_COMMAND,            XML_TOK_COMMAND },
            { XML_NAMESPACE_DB, XML_ESCAPE_PROCESSING,  XML_TOK_ESCAPE_PROCESSING   },
            { XML_NAMESPACE_DB, XML_NAME,               XML_TOK_QUERY_NAME  },
            { XML_NAMESPACE_DB, XML_FILTER_STATEMENT,   XML_TOK_FILTER_STATEMENT    },
            { XML_NAMESPACE_DB, XML_ORDER_STATEMENT,    XML_TOK_ORDER_STATEMENT },
            { XML_NAMESPACE_DB, XML_CATALOG_NAME,       XML_TOK_CATALOG_NAME    },
            { XML_NAMESPACE_DB, XML_SCHEMA_NAME,        XML_TOK_SCHEMA_NAME },
            { XML_NAMESPACE_DB, XML_STYLE_NAME,         XML_TOK_STYLE_NAME},
            { XML_NAMESPACE_DB, XML_APPLY_FILTER,       XML_TOK_APPLY_FILTER},
            { XML_NAMESPACE_DB, XML_APPLY_ORDER,        XML_TOK_APPLY_ORDER},
            { XML_NAMESPACE_DB, XML_COLUMNS,            XML_TOK_COLUMNS},
            XML_TOKEN_MAP_END
        };
        m_pQueryElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pQueryElemTokenMap;
}
// -----------------------------------------------------------------------------
const SvXMLTokenMap& ODBFilter::GetColumnElemTokenMap() const
{
    if ( !m_pColumnElemTokenMap.get() )
    {
        static const SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_NAME,                       XML_TOK_COLUMN_NAME             },
            { XML_NAMESPACE_DB, XML_STYLE_NAME,                 XML_TOK_COLUMN_STYLE_NAME       },
            { XML_NAMESPACE_DB, XML_HELP_MESSAGE,               XML_TOK_COLUMN_HELP_MESSAGE     },
            { XML_NAMESPACE_DB, XML_VISIBILITY,                 XML_TOK_COLUMN_VISIBILITY       },
            { XML_NAMESPACE_DB, XML_DEFAULT_VALUE,              XML_TOK_COLUMN_DEFAULT_VALUE    },
            { XML_NAMESPACE_DB, XML_TYPE_NAME,                  XML_TOK_COLUMN_TYPE_NAME        },
            { XML_NAMESPACE_DB, XML_VISIBLE,                    XML_TOK_COLUMN_VISIBLE          },
            { XML_NAMESPACE_DB, XML_DEFAULT_CELL_STYLE_NAME,    XML_TOK_DEFAULT_CELL_STYLE_NAME },
            XML_TOKEN_MAP_END
        };
        m_pColumnElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pColumnElemTokenMap;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateStylesContext(sal_uInt16 _nPrefix,const OUString& rLocalName,
                                     const uno::Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle )
{
    SvXMLImportContext *pContext = NULL;
    if (!pContext)
    {
        pContext = new OTableStylesContext(*this, _nPrefix, rLocalName, xAttrList, bIsAutoStyle);
        if (bIsAutoStyle)
            SetAutoStyles((SvXMLStylesContext*)pContext);
        else
            SetStyles((SvXMLStylesContext*)pContext);
    }
    return pContext;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateScriptContext( const OUString& _rLocalName )
{
    return new XMLScriptContext( *this, XML_NAMESPACE_OFFICE, _rLocalName, GetModel() );
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ODBFilter::GetTableStylesPropertySetMapper() const
{
    if ( !m_xTableStylesPropertySetMapper.is() )
    {
        m_xTableStylesPropertySetMapper = OXMLHelper::GetTableStylesPropertySetMapper();
    }
    return m_xTableStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ODBFilter::GetColumnStylesPropertySetMapper() const
{
    if ( !m_xColumnStylesPropertySetMapper.is() )
    {
        m_xColumnStylesPropertySetMapper = OXMLHelper::GetColumnStylesPropertySetMapper();
    }
    return m_xColumnStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ODBFilter::GetCellStylesPropertySetMapper() const
{
    if ( !m_xCellStylesPropertySetMapper.is() )
    {
        m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylesPropertySetMapper();
    }
    return m_xCellStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
void ODBFilter::setPropertyInfo()
{
    Reference<XPropertySet> xDataSource(getDataSource());
    if ( !xDataSource.is() )
        return;

    ::connectivity::DriversConfig aDriverConfig(GetComponentContext());
    const OUString sURL = ::comphelper::getString(xDataSource->getPropertyValue(PROPERTY_URL));
    ::comphelper::NamedValueCollection aDataSourceSettings = aDriverConfig.getProperties( sURL );

    Sequence<PropertyValue> aInfo;
    if ( !m_aInfoSequence.empty() )
        aInfo = Sequence<PropertyValue>(&(*m_aInfoSequence.begin()),m_aInfoSequence.size());
    aDataSourceSettings.merge( ::comphelper::NamedValueCollection( aInfo ), true );

    aDataSourceSettings >>= aInfo;
    if ( aInfo.getLength() )
    {
        try
        {
            xDataSource->setPropertyValue(PROPERTY_INFO,makeAny(aInfo));
        }
        catch (const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------
}// dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
