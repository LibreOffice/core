 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlfilter.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:49:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#ifndef _COM_SUN_STAR_PACKAGES_ZIP_ZIPIOEXCEPTION_HPP_
#include <com/sun/star/packages/zip/ZipIOException.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_ELEMENTMODES_HPP_
#include <com/sun/star/embed/ElementModes.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XOFFICEDATABASEDOCUMENT_HPP_
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _FLT_REGHELPER_HXX_
#include "flt_reghelper.hxx"
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_INPUTSOURCE_HPP_
#include <com/sun/star/xml/sax/InputSource.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XPARSER_HPP_
#include <com/sun/star/xml/sax/XParser.hpp>
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen wg. SfxMedium
#include <sfx2/docfile.hxx>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XNAMINGSERVICE_HPP_
#include <com/sun/star/uno/XNamingService.hpp>
#endif
#ifndef DBA_XMLDATABASE_HXX
#include "xmlDatabase.hxx"
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _URLOBJ_HXX //autogen wg. INetURLObject
#include <tools/urlobj.hxx>
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _XMLOFF_DOCUMENTSETTINGSCONTEXT_HXX
#include <xmloff/DocumentSettingsContext.hxx>
#endif
#ifndef DBA_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef DBA_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SFXECODE_HXX
#include <svtools/sfxecode.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

extern "C" void SAL_CALL createRegistryInfo_ODBFilter( )
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::ODBFilter > aAutoRegistration;
}
//--------------------------------------------------------------------------
namespace dbaxml
{
    sal_Char __READONLY_DATA sXML_np__db[] = "_db";
    sal_Char __READONLY_DATA sXML_np___db[] = "__db";

    using namespace ::com::sun::star::util;
    /// read a component (file + filter version)
sal_Int32 ReadThroughComponent(
    const uno::Reference<XInputStream>& xInputStream,
    const uno::Reference<XComponent>& xModelComponent,
    const uno::Reference<XMultiServiceFactory> & rFactory,
    const uno::Reference< XDocumentHandler >& _xFilter )
{
    DBG_ASSERT(xInputStream.is(), "input stream missing");
    DBG_ASSERT(xModelComponent.is(), "document missing");
    DBG_ASSERT(rFactory.is(), "factory missing");

    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "dbaxml", "oj", "ReadThroughComponent" );

    // prepare ParserInputSrouce
    InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get parser
    uno::Reference< XParser > xParser(
        rFactory->createInstance(
        ::rtl::OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
        UNO_QUERY );
    DBG_ASSERT( xParser.is(), "Can't create parser" );
    if( !xParser.is() )
        return 1;
    RTL_LOGFILE_CONTEXT_TRACE( aLog, "parser created" );

    // get filter
    DBG_ASSERT( _xFilter.is(), "Can't instantiate filter component." );
    if( !_xFilter.is() )
        return 1;

    // connect parser and filter
    xParser->setDocumentHandler( _xFilter );

    // connect model and filter
    uno::Reference < XImporter > xImporter( _xFilter, UNO_QUERY );
    xImporter->setTargetDocument( xModelComponent );


#ifdef TIMELOG
    // if we do profiling, we want to know the stream
    RTL_LOGFILE_TRACE_AUTHOR1( "dbaxml", "oj",
                               "ReadThroughComponent : parsing \"%s\"", pStreamName );
#endif

    // finally, parser the stream
    try
    {
        xParser->parseStream( aParserInput );
    }
#if OSL_DEBUG_LEVEL > 1
    catch( SAXParseException& r )
    {
        ByteString aError( "SAX parse exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        aError += ByteString::CreateFromInt32( r.LineNumber );
        aError += ',';
        aError += ByteString::CreateFromInt32( r.ColumnNumber );

        DBG_ERROR( aError.GetBuffer() );
        return 1;
    }
#else
    catch( SAXParseException& )
    {
        return 1;
    }
#endif
    catch( SAXException& )
    {
        return 1;
    }
    catch( packages::zip::ZipIOException& )
    {
        return ERRCODE_IO_BROKENPACKAGE;
    }
    catch( IOException& )
    {
        return 1;
    }
    catch( Exception& )
    {
        return 1;
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
    const uno::Reference<XMultiServiceFactory> & rFactory,
    const uno::Reference< XDocumentHandler >& _xFilter)
{
    DBG_ASSERT( xStorage.is(), "Need storage!");
    DBG_ASSERT(NULL != pStreamName, "Please, please, give me a name!");

    if ( xStorage.is() )
    {
        uno::Reference< io::XStream > xDocStream;
        sal_Bool bEncrypted = sal_False;

        try
        {
            // open stream (and set parser input)
        ::rtl::OUString sStreamName = ::rtl::OUString::createFromAscii(pStreamName);
            if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
            {
                // stream name not found! Then try the compatibility name.
                // if no stream can be opened, return immediatly with OK signal

                // do we even have an alternative name?
                if ( NULL == pCompatibilityStreamName )
                    return 0;

                // if so, does the stream exist?
                sStreamName = ::rtl::OUString::createFromAscii(pCompatibilityStreamName);
                if ( !xStorage->hasByName( sStreamName ) || !xStorage->isStreamElement( sStreamName ) )
                    return 0;
            }

            // get input stream
            xDocStream = xStorage->openStreamElement( sStreamName, embed::ElementModes::READ );

            uno::Reference< beans::XPropertySet > xProps( xDocStream, uno::UNO_QUERY_THROW );
            uno::Any aAny = xProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Encrypted") ) );
            aAny >>= bEncrypted;
        }
        catch( packages::WrongPasswordException& )
        {
            return ERRCODE_SFX_WRONGPASSWORD;
        }
        catch ( uno::Exception& )
        {
            return 1; // TODO/LATER: error handling
        }

        uno::Reference< XInputStream > xInputStream = xDocStream->getInputStream();
        // read from the stream
        return ReadThroughComponent( xInputStream
                                    ,xModelComponent
                                    ,rFactory
                                    ,_xFilter );
    }

    // TODO/LATER: better error handling
    return 1;
}

// -------------
// - ODBFilter -
// -------------
DBG_NAME(ODBFilter)

ODBFilter::ODBFilter( const uno::Reference< XMultiServiceFactory >& _rxMSF )
    :SvXMLImport(_rxMSF)
    ,m_bNewFormat(false)
{
    DBG_CTOR(ODBFilter,NULL);

    GetMM100UnitConverter().setCoreMeasureUnit(MAP_10TH_MM);
    GetMM100UnitConverter().setXMLMeasureUnit(MAP_CM);
    GetNamespaceMap().Add( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__db) ),
                        GetXMLToken(XML_N_DB),
                        XML_NAMESPACE_DB );

    GetNamespaceMap().Add( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np___db) ),
                        GetXMLToken(XML_N_DB_OASIS),
                        XML_NAMESPACE_DB );
}

// -----------------------------------------------------------------------------

ODBFilter::~ODBFilter() throw()
{

    DBG_DTOR(ODBFilter,NULL);
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO1_STATIC( ODBFilter, "com.sun.star.comp.sdb.DBFilter", "com.sun.star.document.ImportFilter")
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODBFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    uno::Reference< ::com::sun::star::awt::XWindow > xWindow;
    {
        ::vos::OGuard aGuard(Application::GetSolarMutex());
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
        ::vos::OGuard aGuard(Application::GetSolarMutex());
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
::rtl::OUString                         sFileName;

    const PropertyValue* pIter = rDescriptor.getConstArray();
    const PropertyValue* pEnd   = pIter + rDescriptor.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if( pIter->Name.equalsAscii( "FileName" ) )
            pIter->Value >>= sFileName;
    }


    sal_Bool bRet = (sFileName.getLength() != 0);
    if ( bRet )
    {
        uno::Reference<XComponent> xCom(GetModel(),UNO_QUERY);

        SfxMediumRef pMedium = new SfxMedium(
                sFileName, ( STREAM_READ | STREAM_NOCREATE ), FALSE, 0 );
        uno::Reference< embed::XStorage > xStorage;
        if( pMedium )
        {
            try
            {
                xStorage = pMedium->GetStorage();
                //  nError = pMedium->GetError();
            }
            catch(const Exception&)
            {
            }
        }

        OSL_ENSURE(xStorage.is(),"No Storage for read!");
        if ( xStorage.is() )
        {
            uno::Reference<sdb::XOfficeDatabaseDocument> xOfficeDoc(GetModel(),UNO_QUERY_THROW);
            m_xDataSource.set(xOfficeDoc->getDataSource(),UNO_QUERY_THROW);
            OSL_ENSURE(m_xDataSource.is(),"DataSource is NULL!");
            uno::Reference< XNumberFormatsSupplier > xNum(m_xDataSource->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
            SetNumberFormatsSupplier(xNum);


            uno::Reference<XComponent> xModel(GetModel(),UNO_QUERY);
            sal_Int32 nRet = ReadThroughComponent( xStorage
                                        ,xModel
                                        ,"settings.xml"
                                        ,"Settings.xml"
                                        ,getServiceFactory()
                                        ,this
                                        );

            if ( nRet == 0 )
                nRet = ReadThroughComponent( xStorage
                                        ,xModel
                                        ,"content.xml"
                                        ,"Content.xml"
                                        ,getServiceFactory()
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
                        if( xStorage.is() )
                        {
                            // TODO/LATER: no way to transport the error outside from the filter!
                            break;
                        }
                        // fall through intented
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
    }

    return bRet;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
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
        if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Queries" ) ) )
        {
            fillPropertyMap(pIter->Value,m_aQuerySettings);
        }
        else if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "Tables" ) ) )
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
        if ( pIter->Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( "layout-settings" ) ) )
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_OFFICE, XML_SETTINGS,           XML_TOK_DOC_SETTINGS    },
            { XML_NAMESPACE_OOO, XML_SETTINGS,              XML_TOK_DOC_SETTINGS    },
            { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OOO, XML_STYLES,                XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OOO, XML_AUTOMATIC_STYLES,      XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OFFICE, XML_DATABASE,           XML_TOK_DOC_DATABASE    },
            { XML_NAMESPACE_OOO, XML_DATABASE,              XML_TOK_DOC_DATABASE    },
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
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
        static __FAR_DATA SvXMLTokenMapEntry aElemTokenMap[]=
        {
            { XML_NAMESPACE_DB, XML_NAME,           XML_TOK_COLUMN_NAME },
            { XML_NAMESPACE_DB, XML_STYLE_NAME,     XML_TOK_COLUMN_STYLE_NAME   },
            { XML_NAMESPACE_DB, XML_HELP_MESSAGE,   XML_TOK_COLUMN_HELP_MESSAGE },
            { XML_NAMESPACE_DB, XML_VISIBILITY,     XML_TOK_COLUMN_VISIBILITY   },
            { XML_NAMESPACE_DB, XML_DEFAULT_VALUE,  XML_TOK_COLUMN_DEFAULT_VALUE},
            { XML_NAMESPACE_DB, XML_TYPE_NAME,      XML_TOK_COLUMN_TYPE_NAME    },
            { XML_NAMESPACE_DB, XML_VISIBLE,        XML_TOK_COLUMN_VISIBLE      },
            XML_TOKEN_MAP_END
        };
        m_pColumnElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pColumnElemTokenMap;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateStylesContext(sal_uInt16 _nPrefix,const ::rtl::OUString& rLocalName,
                                     const uno::Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle )
{
    SvXMLImportContext *pContext = NULL;
    if (!pContext)
    {
        pContext = new OTableStylesContext(*this, _nPrefix, rLocalName, xAttrList, bIsAutoStyle);
        if (bIsAutoStyle)
            //xAutoStyles = pContext;
            SetAutoStyles((SvXMLStylesContext*)pContext);
        else
            //xStyles = pContext;
            SetStyles((SvXMLStylesContext*)pContext);
    }
    return pContext;
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
void ODBFilter::setPropertyInfo()
{
    Reference<XPropertySet> xDataSource(getDataSource());
    if ( !m_aInfoSequence.empty() && xDataSource.is() )
    {
        try
        {
            xDataSource->setPropertyValue(PROPERTY_INFO,makeAny(Sequence<PropertyValue>(&(*m_aInfoSequence.begin()),m_aInfoSequence.size())));
        }
        catch(Exception)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}
// -----------------------------------------------------------------------------
}// dbaxml
// -----------------------------------------------------------------------------
