 /*************************************************************************
 *
 *  $RCSfile: xmlfilter.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:24:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
    const Reference<XInputStream>& xInputStream,
    const Reference<XComponent>& xModelComponent,
    const sal_Char* pStreamName,
    const Reference<XMultiServiceFactory> & rFactory,
    const Reference< XDocumentHandler >& _xFilter,
    sal_Bool bEncrypted )
{
    DBG_ASSERT(xInputStream.is(), "input stream missing");
    DBG_ASSERT(xModelComponent.is(), "document missing");
    DBG_ASSERT(rFactory.is(), "factory missing");

    RTL_LOGFILE_CONTEXT_AUTHOR( aLog, "dbaxml", "oj", "ReadThroughComponent" );

    // prepare ParserInputSrouce
    InputSource aParserInput;
    aParserInput.aInputStream = xInputStream;

    // get parser
    Reference< XParser > xParser(
        rFactory->createInstance(
            OUString::createFromAscii("com.sun.star.xml.sax.Parser") ),
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
    Reference < XImporter > xImporter( _xFilter, UNO_QUERY );
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
    catch( SAXParseException& r )
    {
#if OSL_DEBUG_LEVEL > 1
        ByteString aError( "SAX parse exception catched while importing:\n" );
        aError += ByteString( String( r.Message), RTL_TEXTENCODING_ASCII_US );
        aError += ByteString::CreateFromInt32( r.LineNumber );
        aError += ',';
        aError += ByteString::CreateFromInt32( r.ColumnNumber );

        DBG_ERROR( aError.GetBuffer() );
#endif
        return 1;
    }
    catch( SAXException& )
    {
        return 1;
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
    SvStorage* pStorage,
    const Reference<XComponent>& xModelComponent,
    const sal_Char* pStreamName,
    const sal_Char* pCompatibilityStreamName,
    const Reference<XMultiServiceFactory> & rFactory,
    const Reference< XDocumentHandler >& _xFilter)
{
    DBG_ASSERT(NULL != pStorage, "Need storage!");
    DBG_ASSERT(NULL != pStreamName, "Please, please, give me a name!");

    // open stream (and set parser input)
    OUString sStreamName = OUString::createFromAscii(pStreamName);
    if (! pStorage->IsStream(sStreamName))
    {
        // stream name not found! Then try the compatibility name.
        // if no stream can be opened, return immediatly with OK signal

        // do we even have an alternative name?
        if ( NULL == pCompatibilityStreamName )
            return 0;

        // if so, does the stream exist?
        sStreamName = OUString::createFromAscii(pCompatibilityStreamName);
        if (! pStorage->IsStream(sStreamName) )
            return 0;
    }

    // get input stream
    SvStorageStreamRef xEventsStream;
    xEventsStream = pStorage->OpenStream( sStreamName,
                                          STREAM_READ | STREAM_NOCREATE );

    Any aAny;
    sal_Bool bEncrypted =
        xEventsStream->GetProperty(
                OUString( RTL_CONSTASCII_USTRINGPARAM("Encrypted") ), aAny ) &&
        aAny.getValueType() == ::getBooleanCppuType() &&
        *(sal_Bool *)aAny.getValue();

    Reference< XInputStream > xStream = xEventsStream->GetXInputStream();
    // read from the stream
    return ReadThroughComponent( xStream
                                ,xModelComponent
                                ,pStreamName
                                ,rFactory
                                ,_xFilter
                                ,bEncrypted );
}

// -------------
// - ODBFilter -
// -------------

ODBFilter::ODBFilter( const Reference< XMultiServiceFactory >& _rxMSF )
    :SvXMLImport(_rxMSF)
{
    GetMM100UnitConverter().setCoreMeasureUnit(MAP_10TH_MM);
    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np__db) ),
                        GetXMLToken(XML_N_DB),
                        XML_NAMESPACE_DB );

    GetNamespaceMap().Add( OUString( RTL_CONSTASCII_USTRINGPARAM ( sXML_np___db) ),
                        GetXMLToken(XML_N_DB_OASIS),
                        XML_NAMESPACE_DB );
}

// -----------------------------------------------------------------------------

ODBFilter::~ODBFilter() throw()
{
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO1_STATIC( ODBFilter, "com.sun.star.comp.sdb.DBFilter", "com.sun.star.document.ImportFilter")
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL ODBFilter::filter( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    Window*     pFocusWindow = Application::GetFocusWindow();
    sal_Bool    bRet = sal_False;

    if( pFocusWindow )
        pFocusWindow->EnterWait();

    if ( GetModel().is() )
        bRet = implImport( rDescriptor );

    if ( pFocusWindow )
        pFocusWindow->LeaveWait();

    return bRet;
}
// -----------------------------------------------------------------------------
sal_Bool ODBFilter::implImport( const Sequence< PropertyValue >& rDescriptor )
    throw (RuntimeException)
{
    OUString                            sFileName;

    const PropertyValue* pIter = rDescriptor.getConstArray();
    const PropertyValue* pEnd   = pIter + rDescriptor.getLength();
    for(;pIter != pEnd;++pIter)
    {
        if( pIter->Name.equalsAscii( "FileName" ) )
            pIter->Value >>= sFileName;
    }


    sal_Bool bRet;
    if ( bRet = (sFileName.getLength() != 0) )
    {
        Reference<XComponent> xCom(GetModel(),UNO_QUERY);

        SfxMediumRef pMedium = new SfxMedium(
                sFileName, ( STREAM_READ | STREAM_NOCREATE ), FALSE, 0 );
        SvStorage *pStorage = 0;
        if( pMedium )
        {
            pStorage = pMedium->GetStorage();

            sal_uInt32 nError = pMedium->GetError();

            nError = nError;

        }

        OSL_ENSURE(pStorage,"No Storage for read!");
        if ( pStorage )
        {
            Reference<XPropertySet> xProp(GetModel(),UNO_QUERY);
            Reference< XNumberFormatsSupplier > xNum(xProp->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
            SetNumberFormatsSupplier(xNum);


            Reference<XComponent> xModel(GetModel(),UNO_QUERY);
            bRet = ReadThroughComponent( pStorage
                                        ,xModel
                                        ,"settings.xml"
                                        ,"Settings.xml"
                                        ,getServiceFactory()
                                        ,this
                                        ) == 0;

            bRet = bRet && ReadThroughComponent( pStorage
                                        ,xModel
                                        ,"content.xml"
                                        ,"Content.xml"
                                        ,getServiceFactory()
                                        ,this
                                        ) == 0;
            if ( bRet )
            {
                GetModel()->attachResource(sFileName,Sequence< PropertyValue >());

                Reference< XModifiable > xModi(GetModel(),UNO_QUERY);
                if ( xModi.is() )
                    xModi->setModified(sal_False);
            }
        }
    }
    return bRet;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList )
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
            pContext = new OXMLDatabase( *this, nPrefix, rLocalName,xAttrList );
            break;
        case XML_TOK_DOC_STYLES:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateStylesContext( rLocalName, xAttrList, sal_False);
            break;
        case XML_TOK_DOC_AUTOSTYLES:
            GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = CreateStylesContext( rLocalName, xAttrList, sal_True);
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
            Reference<XPropertySet> xProp(GetModel(),UNO_QUERY);
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
            { XML_NAMESPACE_OOO, XML_SETTINGS,          XML_TOK_DOC_SETTINGS    },
            { XML_NAMESPACE_OFFICE, XML_STYLES,             XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OOO, XML_STYLES,                XML_TOK_DOC_STYLES      },
            { XML_NAMESPACE_OFFICE, XML_AUTOMATIC_STYLES,   XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OOO, XML_AUTOMATIC_STYLES,  XML_TOK_DOC_AUTOSTYLES  },
            { XML_NAMESPACE_OFFICE, XML_DATABASE,           XML_TOK_DOC_DATABASE    },
            { XML_NAMESPACE_OOO, XML_DATABASE,          XML_TOK_DOC_DATABASE    },
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
            { XML_NAMESPACE_DB, XML_DATASOURCE, XML_TOK_DATASOURCE  },
            { XML_NAMESPACE_DB, XML_FORMS,      XML_TOK_FORMS},
            { XML_NAMESPACE_DB, XML_REPORTS,    XML_TOK_REPORTS},
            { XML_NAMESPACE_DB, XML_QUERIES,    XML_TOK_QUERIES},
            { XML_NAMESPACE_DB, XML_TABLES,     XML_TOK_TABLES},
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
            { XML_NAMESPACE_DB, XML_CONNECTION_RESOURCE,            XML_TOK_CONNECTION_RESOURCE},
            { XML_NAMESPACE_DB, XML_SUPPRESS_VERSION_COLUMNS,       XML_TOK_SUPPRESS_VERSION_COLUMNS},
            { XML_NAMESPACE_DB, XML_JAVA_DRIVER_CLASS,              XML_TOK_JAVA_DRIVER_CLASS},
            { XML_NAMESPACE_DB, XML_EXTENSION,                      XML_TOK_EXTENSION},
            { XML_NAMESPACE_DB, XML_IS_FIRST_ROW_HEADER_LINE,       XML_TOK_IS_FIRST_ROW_HEADER_LINE},
            { XML_NAMESPACE_DB, XML_SHOW_DELETED,                   XML_TOK_SHOW_DELETED},
            { XML_NAMESPACE_DB, XML_IS_TABLE_NAME_LENGTH_LIMITED,   XML_TOK_IS_TABLE_NAME_LENGTH_LIMITED},
            { XML_NAMESPACE_DB, XML_SYSTEM_DRIVER_SETTINGS,         XML_TOK_SYSTEM_DRIVER_SETTINGS},
            { XML_NAMESPACE_DB, XML_ENABLE_SQL92_CHECK,             XML_TOK_ENABLE_SQL92_CHECK},
            { XML_NAMESPACE_DB, XML_APPEND_TABLE_ALIAS_NAME,        XML_TOK_APPEND_TABLE_ALIAS_NAME},
            { XML_NAMESPACE_DB, XML_PARAMETER_NAME_SUBSTITUTION,    XML_TOK_PARAMETER_NAME_SUBSTITUTION},
            { XML_NAMESPACE_DB, XML_IGNORE_DRIVER_PRIVILEGES,       XML_TOK_IGNORE_DRIVER_PRIVILEGES},
            { XML_NAMESPACE_DB, XML_BOOLEAN_COMPARISON_MODE,        XML_TOK_BOOLEAN_COMPARISON_MODE},
            { XML_NAMESPACE_DB, XML_USE_CATALOG,                    XML_TOK_USE_CATALOG},
            { XML_NAMESPACE_DB, XML_BASE_DN,                        XML_TOK_BASE_DN},
            { XML_NAMESPACE_DB, XML_MAX_ROW_COUNT,                  XML_TOK_MAX_ROW_COUNT},
            { XML_NAMESPACE_DB, XML_LOGIN,                          XML_TOK_LOGIN},
            { XML_NAMESPACE_DB, XML_TABLE_FILTER,                   XML_TOK_TABLE_FILTER},
            { XML_NAMESPACE_DB, XML_TABLE_TYPE_FILTER,              XML_TOK_TABLE_TYPE_FILTER},
            { XML_NAMESPACE_DB, XML_AUTO_INCREMENT,                 XML_TOK_AUTO_INCREMENT},
            { XML_NAMESPACE_DB, XML_DELIMITER,                      XML_TOK_DELIMITER},
            { XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTINGS,           XML_TOK_DATA_SOURCE_SETTINGS},
            { XML_NAMESPACE_DB, XML_FONT_CHARSET,                   XML_TOK_FONT_CHARSET},
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
            XML_TOKEN_MAP_END
        };
        m_pLoginElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pLoginElemTokenMap;
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
            XML_TOKEN_MAP_END
        };
        m_pColumnElemTokenMap.reset(new SvXMLTokenMap( aElemTokenMap ));
    }
    return *m_pColumnElemTokenMap;
}
// -----------------------------------------------------------------------------
SvXMLImportContext* ODBFilter::CreateStylesContext(const ::rtl::OUString& rLocalName,
                                     const Reference< XAttributeList>& xAttrList, sal_Bool bIsAutoStyle )
{
    SvXMLImportContext *pContext = NULL;
    if (!pContext)
    {
        pContext = new OTableStylesContext(*this, XML_NAMESPACE_OFFICE, rLocalName, xAttrList, bIsAutoStyle);
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
}// dbaxml
// -----------------------------------------------------------------------------
