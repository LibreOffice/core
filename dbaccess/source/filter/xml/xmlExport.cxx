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


#include "xmlExport.hxx"
#include "xmlAutoStyle.hxx"
#include "flt_reghelper.hxx"
#include <sax/tools/converter.hxx>
#include <xmloff/ProgressBarHelper.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/txtimp.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include "xmlstrings.hrc"
#include "xmlEnums.hxx"
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XOfficeDatabaseDocument.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#include <com/sun/star/sdbcx/XDataDescriptorFactory.hpp>

#include <com/sun/star/awt/TextAlign.hpp>
#include <xmloff/xmluconv.hxx>
#include "xmlHelper.hxx"
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <svl/filenotation.hxx>
#include <unotools/pathoptions.hxx>
#include <tools/diagnose_ex.h>
#include <connectivity/DriversConfig.hxx>
#include <connectivity/dbtools.hxx>

#include <boost/optional.hpp>

namespace dbaxml
{
    using namespace comphelper;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star;

    class ODBExportHelper
    {
    public:
        static OUString SAL_CALL getImplementationName_Static(  ) throw (RuntimeException);
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(RuntimeException);
        static Reference< XInterface > SAL_CALL Create(const Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
    };
    class ODBFullExportHelper
    {
    public:
        static OUString SAL_CALL getImplementationName_Static(  ) throw (RuntimeException);
        static Sequence< OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(RuntimeException);
        static Reference< XInterface > SAL_CALL Create(const Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
    };
}
// -----------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_ODBFilterExport( )
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::ODBExport > aAutoRegistration;
}
//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_OSettingsExport( )
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::ODBExportHelper > aAutoRegistration;
}
//--------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_OFullExport( )
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::ODBFullExportHelper > aAutoRegistration;
}
//--------------------------------------------------------------------------
namespace dbaxml
{
    using namespace comphelper;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::util;

    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ODBExportHelper::Create(const Reference< XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new ODBExport(comphelper::getComponentContext(_rxORB),EXPORT_SETTINGS | EXPORT_PRETTY ));
    }
    //---------------------------------------------------------------------
    OUString SAL_CALL ODBExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.sdb.XMLSettingsExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > SAL_CALL ODBExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = OUString("com.sun.star.document.ExportFilter");
        return aSupported;
    }


    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ODBFullExportHelper::Create(const Reference< XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new ODBExport(comphelper::getComponentContext(_rxORB),EXPORT_ALL));
    }
    //---------------------------------------------------------------------
    OUString SAL_CALL ODBFullExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return OUString("com.sun.star.comp.sdb.XMLFullExporter");
    }
    //---------------------------------------------------------------------
    Sequence< OUString > SAL_CALL ODBFullExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< OUString > aSupported(1);
        aSupported[0] = OUString("com.sun.star.document.ExportFilter");
        return aSupported;
    }

    //---------------------------------------------------------------------
    OUString lcl_implGetPropertyXMLType(const Type& _rType)
    {
        // possible types we can write (either because we recognize them directly or because we convert _rValue
        // into one of these types)
        static const OUString s_sTypeBoolean ("boolean");
        static const OUString s_sTypeShort   ("short");
        static const OUString s_sTypeInteger ("int");
        static const OUString s_sTypeLong    ("long");
        static const OUString s_sTypeDouble  ("double");
        static const OUString s_sTypeString  ("string");

        // handle the type description
        switch (_rType.getTypeClass())
        {
            case TypeClass_STRING:
                return s_sTypeString;
            case TypeClass_DOUBLE:
                return s_sTypeDouble;
            case TypeClass_BOOLEAN:
                return s_sTypeBoolean;
            case TypeClass_BYTE:
            case TypeClass_SHORT:
                return s_sTypeShort;
            case TypeClass_LONG:
                return s_sTypeInteger;
            case TypeClass_HYPER:
                return s_sTypeLong;
            case TypeClass_ENUM:
                return s_sTypeInteger;

            default:
                OSL_FAIL( "lcl_implGetPropertyXMLType: unsupported value type!" );
                return s_sTypeDouble;
        }
    }

    class OSpecialHanldeXMLExportPropertyMapper : public SvXMLExportPropertyMapper
    {
    public:
        OSpecialHanldeXMLExportPropertyMapper(const UniReference< XMLPropertySetMapper >& rMapper) : SvXMLExportPropertyMapper(rMapper )
        {
        }
        /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
        virtual void handleSpecialItem(
                SvXMLAttributeList& /*rAttrList*/,
                const XMLPropertyState& /*rProperty*/,
                const SvXMLUnitConverter& /*rUnitConverter*/,
                const SvXMLNamespaceMap& /*rNamespaceMap*/,
                const ::std::vector< XMLPropertyState > * /*pProperties*/ ,
                sal_uInt32 /*nIdx*/ ) const
        {
            // nothing to do here
        }
    };
// -----------------------------------------------------------------------------
ODBExport::ODBExport(const Reference< XComponentContext >& _rxContext,sal_uInt16 nExportFlag)
: SvXMLExport( util::MeasureUnit::MM_10TH, _rxContext, XML_DATABASE,
        EXPORT_OASIS | nExportFlag)
,m_aTypeCollection(_rxContext)
,m_bAllreadyFilled(sal_False)
{
    GetMM100UnitConverter().SetCoreMeasureUnit(util::MeasureUnit::MM_10TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(util::MeasureUnit::CM);

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG), XML_NAMESPACE_SVG );

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_DB), GetXMLToken(XML_N_DB_OASIS), XML_NAMESPACE_DB );

    if( (nExportFlag & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS) ) != 0 )
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );

    if( (nExportFlag & (EXPORT_META|EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_SCRIPTS|EXPORT_SETTINGS) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( (nExportFlag & EXPORT_SETTINGS) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( (nExportFlag & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_CONTENT|EXPORT_FONTDECLS) ) != 0 )
    {
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );

    m_xExportHelper = new SvXMLExportPropertyMapper(GetTableStylesPropertySetMapper());
    m_xColumnExportHelper = new OSpecialHanldeXMLExportPropertyMapper(GetColumnStylesPropertySetMapper());

    m_xCellExportHelper = new OSpecialHanldeXMLExportPropertyMapper(GetCellStylesPropertySetMapper());
    m_xRowExportHelper = new OSpecialHanldeXMLExportPropertyMapper(OXMLHelper::GetRowStylesPropertySetMapper());

    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_TABLE_TABLE,
        OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME ),
        m_xExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_TABLE_COLUMN,
        OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ),
        m_xColumnExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_TABLE_CELL,
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME ),
        m_xCellExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_TABLE_ROW,
        OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME ),
        m_xRowExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX ));
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO_IMPLNAME_STATIC(ODBExport, "com.sun.star.comp.sdb.DBExportFilter")
IMPLEMENT_SERVICE_INFO_SUPPORTS(ODBExport)
IMPLEMENT_SERVICE_INFO_GETSUPPORTED1_STATIC(ODBExport, "com.sun.star.document.ExportFilter")

::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
    SAL_CALL ODBExport::Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB)
{
    return static_cast< XServiceInfo* >(new ODBExport( comphelper::getComponentContext(_rxORB)));
}

// -----------------------------------------------------------------------------
void ODBExport::exportDataSource()
{
    try
    {
        Reference<XPropertySet> xProp( getDataSource(), UNO_SET_THROW );

        sal_Bool bAutoIncrementEnabled = sal_True;
        TStringPair aAutoIncrement;

        Reference< XPropertySet > xDataSourceSettings;
        OSL_VERIFY( xProp->getPropertyValue( PROPERTY_SETTINGS ) >>= xDataSourceSettings );
        Reference< XPropertyState > xSettingsState( xDataSourceSettings, UNO_QUERY_THROW );
        Reference< XPropertySetInfo > xSettingsInfo( xDataSourceSettings->getPropertySetInfo(), UNO_SET_THROW );

        TDelimiter aDelimiter;
        xSettingsState->getPropertyDefault( INFO_TEXTDELIMITER ) >>= aDelimiter.sText;
        xSettingsState->getPropertyDefault( INFO_FIELDDELIMITER ) >>= aDelimiter.sField;
        xSettingsState->getPropertyDefault( INFO_DECIMALDELIMITER ) >>= aDelimiter.sDecimal;
        xSettingsState->getPropertyDefault( INFO_THOUSANDSDELIMITER ) >>= aDelimiter.sThousand;

        ::connectivity::DriversConfig aDriverConfig(getComponentContext());
        const OUString sURL = ::comphelper::getString(xProp->getPropertyValue(PROPERTY_URL));
        ::comphelper::NamedValueCollection aDriverSupportedProperties( aDriverConfig.getProperties( sURL ) );

        static OUString s_sTrue(::xmloff::token::GetXMLToken( XML_TRUE ));
        static OUString s_sFalse(::xmloff::token::GetXMLToken( XML_FALSE ));
        // loop through the properties, and export only those which are not defaulted
        TSettingsMap aSettingsMap;
        Sequence< Property > aProperties = xSettingsInfo->getProperties();
        const Property* pProperties = aProperties.getConstArray();
        const Property* pPropertiesEnd = pProperties + aProperties.getLength();
        for ( ; pProperties != pPropertiesEnd; ++pProperties )
        {
            OUString sValue;
            Any aValue = xDataSourceSettings->getPropertyValue( pProperties->Name );
            switch ( aValue.getValueTypeClass() )
            {
                case TypeClass_STRING:
                    aValue >>= sValue;
                break;
                case TypeClass_DOUBLE:
                    // let the unit converter format is as string
                    sValue = OUString::number( getDouble( aValue ) );
                    break;
                case TypeClass_BOOLEAN:
                    sValue = ::xmloff::token::GetXMLToken( getBOOL( aValue ) ? XML_TRUE : XML_FALSE );
                    break;
                case TypeClass_BYTE:
                case TypeClass_SHORT:
                case TypeClass_LONG:
                    // let the unit converter format is as string
                    sValue = OUString::number( getINT32( aValue ) );
                    break;
                default:
                    break;
            }

            ::xmloff::token::XMLTokenEnum eToken = XML_TOKEN_INVALID;

            struct PropertyMap
            {
                const OUString                       sPropertyName;
                const XMLTokenEnum                          eAttributeToken;
                const ::boost::optional< OUString >  aXMLDefault;

                PropertyMap( const OUString& _rPropertyName, const XMLTokenEnum _eToken )
                    :sPropertyName( _rPropertyName )
                    ,eAttributeToken( _eToken )
                    ,aXMLDefault()
                {
                }

                PropertyMap( const OUString& _rPropertyName, const XMLTokenEnum _eToken, const OUString& _rDefault )
                    :sPropertyName( _rPropertyName )
                    ,eAttributeToken( _eToken )
                    ,aXMLDefault( _rDefault )
                {
                }
            };

            PropertyMap aTokens[] =
            {
                PropertyMap( INFO_TEXTFILEHEADER,       XML_IS_FIRST_ROW_HEADER_LINE,       s_sTrue     ),
                PropertyMap( INFO_SHOWDELETEDROWS,      XML_SHOW_DELETED,                   s_sFalse    ),
                PropertyMap( INFO_ALLOWLONGTABLENAMES,  XML_IS_TABLE_NAME_LENGTH_LIMITED,   s_sTrue     ),
                PropertyMap( INFO_ADDITIONALOPTIONS,    XML_SYSTEM_DRIVER_SETTINGS                      ),
                PropertyMap( PROPERTY_ENABLESQL92CHECK, XML_ENABLE_SQL92_CHECK,             s_sFalse    ),
                PropertyMap( INFO_APPEND_TABLE_ALIAS,   XML_APPEND_TABLE_ALIAS_NAME,        s_sTrue     ),
                PropertyMap( INFO_PARAMETERNAMESUBST,   XML_PARAMETER_NAME_SUBSTITUTION,    s_sTrue     ),
                PropertyMap( INFO_IGNOREDRIVER_PRIV,    XML_IGNORE_DRIVER_PRIVILEGES,       s_sTrue     ),
                PropertyMap( INFO_USECATALOG,           XML_USE_CATALOG,                    s_sFalse    ),
                PropertyMap( PROPERTY_SUPPRESSVERSIONCL,XML_SUPPRESS_VERSION_COLUMNS,       s_sTrue     ),
                PropertyMap( INFO_CONN_LDAP_BASEDN,     XML_BASE_DN                                     ),
                PropertyMap( INFO_CONN_LDAP_ROWCOUNT,   XML_MAX_ROW_COUNT                               )
            };

            bool bIsXMLDefault = false;
            for ( size_t i=0; i < sizeof( aTokens ) / sizeof( aTokens[0] ); ++i )
            {
                if ( pProperties->Name == aTokens[i].sPropertyName )
                {
                    eToken = aTokens[i].eAttributeToken;

                    if  (   !!aTokens[i].aXMLDefault
                        &&  ( sValue == *aTokens[i].aXMLDefault )
                        )
                    {
                        bIsXMLDefault = true;
                    }
                    break;
                }
            }

            if ( bIsXMLDefault )
                // the property has the value which is specified as default in the XML schema -> no need to write it
                continue;

            if ( eToken == XML_TOKEN_INVALID )
            {
                // for properties which are not REMOVABLE, we care for their state, and
                // only export them if they're not DEFAULTed
                if ( ( pProperties->Attributes & PropertyAttribute::REMOVABLE ) == 0 )
                {
                    PropertyState ePropertyState = xSettingsState->getPropertyState( pProperties->Name );
                    if ( PropertyState_DEFAULT_VALUE == ePropertyState )
                        continue;
                }

                // special handlings
                if ( pProperties->Name == PROPERTY_BOOLEANCOMPARISONMODE )
                {
                    sal_Int32 nValue = 0;
                    aValue >>= nValue;
                    if ( sValue == "0" )
                        sValue = OUString("equal-integer");
                    else if ( sValue == "1" )
                        sValue = OUString("is-boolean");
                    else if ( sValue == "2" )
                        sValue = OUString("equal-boolean");
                    else if ( sValue == "3" )
                        sValue = OUString("equal-use-only-zero");
                    if ( sValue == "equal-integer" )
                        continue;
                    eToken = XML_BOOLEAN_COMPARISON_MODE;
                }
                else if ( pProperties->Name == INFO_AUTORETRIEVEENABLED )
                {
                    aValue >>= bAutoIncrementEnabled;
                    continue;
                }
                else if ( pProperties->Name == INFO_AUTORETRIEVEVALUE )
                {
                    aAutoIncrement.first = sValue;
                    continue;
                }
                else if ( pProperties->Name == PROPERTY_AUTOINCREMENTCREATION )
                {
                    aAutoIncrement.second = sValue;
                    continue;
                }
                else if ( pProperties->Name == INFO_TEXTDELIMITER )
                {
                    aDelimiter.sText = sValue;
                    aDelimiter.bUsed = true;
                    continue;
                }
                else if ( pProperties->Name == INFO_FIELDDELIMITER )
                {
                    aDelimiter.sField = sValue;
                    aDelimiter.bUsed = true;
                    continue;
                }
                else if ( pProperties->Name == INFO_DECIMALDELIMITER )
                {
                    aDelimiter.sDecimal = sValue;
                    aDelimiter.bUsed = true;
                    continue;
                }
                else if ( pProperties->Name == INFO_THOUSANDSDELIMITER )
                {
                    aDelimiter.sThousand = sValue;
                    aDelimiter.bUsed = true;
                    continue;
                }
                else if ( pProperties->Name == INFO_CHARSET )
                {
                    m_sCharSet = sValue;
                    continue;
                }
                else
                {
                    if ( !aDriverSupportedProperties.has(pProperties->Name) || aDriverSupportedProperties.get(pProperties->Name) != aValue )
                    {
                        m_aDataSourceSettings.push_back( TypedPropertyValue(
                            pProperties->Name, pProperties->Type, aValue ) );
                    }
                    continue;
                }
            }

            aSettingsMap.insert(TSettingsMap::value_type(eToken,sValue));
        }
        if ( bAutoIncrementEnabled && !(aAutoIncrement.first.isEmpty() && aAutoIncrement.second.isEmpty()) )
            m_aAutoIncrement.reset( new TStringPair(aAutoIncrement));
        if ( aDelimiter.bUsed )
            m_aDelimiter.reset( new TDelimiter( aDelimiter ) );

        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DATASOURCE, sal_True, sal_True);

        exportConnectionData();
        exportDriverSettings(aSettingsMap);
        exportApplicationConnectionSettings(aSettingsMap);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportApplicationConnectionSettings(const TSettingsMap& _aSettings)
{
    const ::xmloff::token::XMLTokenEnum pSettings[] = {
        XML_IS_TABLE_NAME_LENGTH_LIMITED
        ,XML_ENABLE_SQL92_CHECK
        ,XML_APPEND_TABLE_ALIAS_NAME
        ,XML_IGNORE_DRIVER_PRIVILEGES
        ,XML_BOOLEAN_COMPARISON_MODE
        ,XML_USE_CATALOG
        ,XML_MAX_ROW_COUNT
        ,XML_SUPPRESS_VERSION_COLUMNS
    };
    for (size_t i = 0; i< sizeof(pSettings)/sizeof(pSettings[0]); ++i)
    {
        TSettingsMap::const_iterator aFind = _aSettings.find(pSettings[i]);
        if ( aFind != _aSettings.end() )
            AddAttribute(XML_NAMESPACE_DB, aFind->first,aFind->second);
    }
    SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_APPLICATION_CONNECTION_SETTINGS, sal_True, sal_True);

    Reference<XPropertySet> xProp(getDataSource());
    Sequence< OUString> aValue;
    xProp->getPropertyValue(PROPERTY_TABLEFILTER) >>= aValue;
    if ( aValue.getLength() )
    {
        SvXMLElementExport aElem2(*this,XML_NAMESPACE_DB, XML_TABLE_FILTER, sal_True, sal_True);
        exportSequence(aValue,XML_TABLE_INCLUDE_FILTER,XML_TABLE_FILTER_PATTERN);
    }

    xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER) >>= aValue;
    if ( aValue.getLength() )
        exportSequence(aValue,XML_TABLE_TYPE_FILTER,XML_TABLE_TYPE);

    exportDataSourceSettings();
}
// -----------------------------------------------------------------------------
void ODBExport::exportDriverSettings(const TSettingsMap& _aSettings)
{
    const ::xmloff::token::XMLTokenEnum pSettings[] = {
        XML_SHOW_DELETED
        ,XML_SYSTEM_DRIVER_SETTINGS
        ,XML_BASE_DN
        ,XML_IS_FIRST_ROW_HEADER_LINE
        ,XML_PARAMETER_NAME_SUBSTITUTION
    };
    for (size_t i = 0; i< sizeof(pSettings)/sizeof(pSettings[0]); ++i)
    {
        TSettingsMap::const_iterator aFind = _aSettings.find(pSettings[i]);
        if ( aFind != _aSettings.end() )
            AddAttribute(XML_NAMESPACE_DB, aFind->first,aFind->second);
    }
    SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DRIVER_SETTINGS, sal_True, sal_True);
    exportAutoIncrement();
    exportDelimiter();
    exportCharSet();
}
// -----------------------------------------------------------------------------
void ODBExport::exportConnectionData()
{
    SvXMLElementExport aConnData(*this,XML_NAMESPACE_DB, XML_CONNECTION_DATA, sal_True, sal_True);

    {
        OUString sValue;
        Reference<XPropertySet> xProp(getDataSource());
        xProp->getPropertyValue(PROPERTY_URL) >>= sValue;
        if ( m_aTypeCollection.isFileSystemBased(sValue) )
        {
            SvXMLElementExport aDatabaseDescription(*this,XML_NAMESPACE_DB, XML_DATABASE_DESCRIPTION, sal_True, sal_True);
            {
                SvtPathOptions aPathOptions;
                const OUString sOrigUrl = m_aTypeCollection.cutPrefix(sValue);
                OUString sFileName = aPathOptions.SubstituteVariable(sOrigUrl);
                if ( sOrigUrl == sFileName )
                {
                    ::svt::OFileNotation aTransformer( sFileName );
                    OUStringBuffer sURL( aTransformer.get( ::svt::OFileNotation::N_URL ) );
                    if (sURL.isEmpty() || sURL[sURL.getLength() - 1] != '/')
                        sURL.append('/');

                    AddAttribute(XML_NAMESPACE_XLINK,XML_HREF,GetRelativeReference(sURL.makeStringAndClear()));
                }
                else
                    AddAttribute(XML_NAMESPACE_XLINK,XML_HREF,sOrigUrl);
                AddAttribute(XML_NAMESPACE_DB,XML_MEDIA_TYPE,m_aTypeCollection.getMediaType(sValue));
                const ::dbaccess::DATASOURCE_TYPE eType = m_aTypeCollection.determineType(sValue);
                try
                {
                    OUString sExtension;
                    if ( eType == dbaccess::DST_MSACCESS )
                        sExtension = OUString("mdb");
                    else
                    {
                        Reference< XPropertySet > xDataSourceSettings;
                        OSL_VERIFY( xProp->getPropertyValue( PROPERTY_SETTINGS ) >>= xDataSourceSettings );
                        xDataSourceSettings->getPropertyValue( INFO_TEXTFILEEXTENSION ) >>= sExtension;
                    }
                    if ( !sExtension.isEmpty() )
                        AddAttribute(XML_NAMESPACE_DB,XML_EXTENSION,sExtension);
                }
                catch(const Exception&)
                {
                }
                SvXMLElementExport aFileBasedDB(*this,XML_NAMESPACE_DB, XML_FILE_BASED_DATABASE, sal_True, sal_True);
            }
        }
        else
        {
            OUString sDatabaseName,sHostName;
            sal_Int32 nPort = -1;
            m_aTypeCollection.extractHostNamePort(sValue,sDatabaseName,sHostName,nPort);
            if ( sHostName.getLength() )
            {
                SvXMLElementExport aDatabaseDescription(*this,XML_NAMESPACE_DB, XML_DATABASE_DESCRIPTION, sal_True, sal_True);
                {
                    OUString sType = comphelper::string::stripEnd(m_aTypeCollection.getPrefix(sValue), ':');
                    AddAttribute(XML_NAMESPACE_DB,XML_TYPE,sType);
                    AddAttribute(XML_NAMESPACE_DB,XML_HOSTNAME,sHostName);
                    if ( nPort != -1 )
                        AddAttribute(XML_NAMESPACE_DB,XML_PORT,OUString::number(nPort));
                    if ( sDatabaseName.getLength() )
                        AddAttribute(XML_NAMESPACE_DB,XML_DATABASE_NAME,sDatabaseName);

                    try
                    {
                        Reference< XPropertySet > xDataSourceSettings( xProp->getPropertyValue( PROPERTY_SETTINGS ), UNO_QUERY_THROW );
                        Reference< XPropertySetInfo > xSettingsInfo( xDataSourceSettings->getPropertySetInfo(), UNO_SET_THROW );

                        struct PropertyMap
                        {
                            const sal_Char* pAsciiPropertyName;
                            sal_uInt16      nAttributeId;

                            PropertyMap() :pAsciiPropertyName( NULL ), nAttributeId(0) { }
                            PropertyMap( const sal_Char* _pAsciiPropertyName, const sal_uInt16 _nAttributeId )
                                :pAsciiPropertyName( _pAsciiPropertyName )
                                ,nAttributeId( _nAttributeId )
                            {
                            }
                        };
                        PropertyMap aProperties[] =
                        {
                            PropertyMap( "LocalSocket", XML_LOCAL_SOCKET )
                            //PropertyMap( "NamedPipe", 0 /* TODO */ )
                        };

                        for (   size_t i=0;
                                i < sizeof( aProperties ) / sizeof( aProperties[0] );
                                ++i
                            )
                        {
                            const OUString sPropertyName = OUString::createFromAscii( aProperties[i].pAsciiPropertyName );
                            if ( xSettingsInfo->hasPropertyByName( sPropertyName ) )
                            {
                                OUString sPropertyValue;
                                if ( ( xDataSourceSettings->getPropertyValue( sPropertyName ) >>= sPropertyValue ) && !sPropertyValue.isEmpty() )
                                    AddAttribute( XML_NAMESPACE_DB, XML_LOCAL_SOCKET, sPropertyValue );

                            }
                        }
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }

                    SvXMLElementExport aServerDB(*this,XML_NAMESPACE_DB, XML_SERVER_DATABASE, sal_True, sal_True);
                }
            }
            else
            {
                AddAttribute(XML_NAMESPACE_XLINK, XML_HREF,sValue);
                SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_CONNECTION_RESOURCE, sal_True, sal_True);
            }
        }

    }

    exportLogin();
}
// -----------------------------------------------------------------------------
template< typename T > void ODBExport::exportDataSourceSettingsSequence(
    ::std::vector< TypedPropertyValue >::iterator const & in)
{
    OSequenceIterator< T > i( in->Value );
    while (i.hasMoreElements())
    {
        SvXMLElementExport aDataValue(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE, sal_True, sal_False);
        // (no whitespace inside the tag)
        Characters(implConvertAny(i.nextElement()));
    }
}

void ODBExport::exportDataSourceSettings()
{
    if ( m_aDataSourceSettings.empty() )
        return;

    SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTINGS, sal_True, sal_True);
    ::std::vector< TypedPropertyValue >::iterator aIter = m_aDataSourceSettings.begin();
    ::std::vector< TypedPropertyValue >::iterator aEnd = m_aDataSourceSettings.end();
    for ( ; aIter != aEnd; ++aIter )
    {
        sal_Bool bIsSequence = TypeClass_SEQUENCE == aIter->Type.getTypeClass();

        Type aSimpleType;
        if ( bIsSequence )
            aSimpleType = ::comphelper::getSequenceElementType( aIter->Value.getValueType() );
        else
            aSimpleType = aIter->Type;

        AddAttribute( XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_IS_LIST,bIsSequence ? XML_TRUE : XML_FALSE );
        AddAttribute( XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_NAME, aIter->Name );

        OUString sTypeName = lcl_implGetPropertyXMLType( aSimpleType );
        if ( bIsSequence && aSimpleType.getTypeClass() == TypeClass_ANY )
        {
            Sequence<Any> aSeq;
            aIter->Value >>= aSeq;
            if ( aSeq.getLength() )
                sTypeName = lcl_implGetPropertyXMLType(aSeq[0].getValueType());
        }

        AddAttribute( XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_TYPE, sTypeName );

        SvXMLElementExport aDataSourceSetting( *this, XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING, sal_True, sal_True );

        if ( !bIsSequence )
        {
            SvXMLElementExport aDataValue( *this, XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE, sal_True, sal_False );
            // (no whitespace inside the tag)
            Characters( implConvertAny( aIter->Value ) );
        }
        else
        {
            // the not-that-simple case, we need to iterate through the sequence elements
            switch (aSimpleType.getTypeClass())
            {
                case TypeClass_STRING:
                    exportDataSourceSettingsSequence< OUString >(
                        aIter );
                    break;
                case TypeClass_DOUBLE:
                    exportDataSourceSettingsSequence< double >( aIter );
                    break;
                case TypeClass_BOOLEAN:
                    exportDataSourceSettingsSequence< sal_Bool >( aIter );
                    break;
                case TypeClass_BYTE:
                    exportDataSourceSettingsSequence< sal_Int8 >( aIter );
                    break;
                case TypeClass_SHORT:
                    exportDataSourceSettingsSequence< sal_Int16 >( aIter );
                    break;
                case TypeClass_LONG:
                    exportDataSourceSettingsSequence< sal_Int32 >( aIter );
                    break;
                case TypeClass_ANY:
                    exportDataSourceSettingsSequence< Any >( aIter );
                    break;
                default:
                    OSL_FAIL("unsupported sequence type !");
                    break;
            }
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportCharSet()
{
    if ( !m_sCharSet.isEmpty() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_ENCODING,m_sCharSet);

        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_FONT_CHARSET, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportDelimiter()
{
    if ( m_aDelimiter.get() && m_aDelimiter->bUsed )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_FIELD,m_aDelimiter->sField);
        AddAttribute(XML_NAMESPACE_DB, XML_STRING,m_aDelimiter->sText);
        AddAttribute(XML_NAMESPACE_DB, XML_DECIMAL,m_aDelimiter->sDecimal);
        AddAttribute(XML_NAMESPACE_DB, XML_THOUSAND,m_aDelimiter->sThousand);
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DELIMITER, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportAutoIncrement()
{
    if ( m_aAutoIncrement.get() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_ADDITIONAL_COLUMN_STATEMENT,m_aAutoIncrement->second);
        AddAttribute(XML_NAMESPACE_DB, XML_ROW_RETRIEVING_STATEMENT,m_aAutoIncrement->first);
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_AUTO_INCREMENT, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportSequence(const Sequence< OUString>& _aValue
                            ,::xmloff::token::XMLTokenEnum _eTokenFilter
                            ,::xmloff::token::XMLTokenEnum _eTokenType)
{
    Reference<XPropertySet> xProp(getDataSource());
    Sequence< OUString> aValue;
    if ( _aValue.getLength() )
    {
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, _eTokenFilter, sal_True, sal_True);

        const OUString* pIter = _aValue.getConstArray();
        const OUString* pEnd   = pIter + _aValue.getLength();
        for(;pIter != pEnd;++pIter)
        {
            SvXMLElementExport aDataSource(*this,XML_NAMESPACE_DB, _eTokenType, sal_True, sal_False);
            Characters(*pIter);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportLogin()
{
    Reference<XPropertySet> xProp(getDataSource());
    OUString sValue;
    xProp->getPropertyValue(PROPERTY_USER) >>= sValue;
    sal_Bool bAddLogin = !sValue.isEmpty();
    if ( bAddLogin )
        AddAttribute(XML_NAMESPACE_DB, XML_USER_NAME,sValue);
    sal_Bool bValue = sal_False;
    if ( xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED) >>= bValue )
    {
        bAddLogin = sal_True;
        AddAttribute(XML_NAMESPACE_DB, XML_IS_PASSWORD_REQUIRED,bValue ? XML_TRUE : XML_FALSE);
    }
    if ( bAddLogin )
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_LOGIN, sal_True, sal_True);
}
// -----------------------------------------------------------------------------
void ODBExport::exportCollection(const Reference< XNameAccess >& _xCollection
                                ,enum ::xmloff::token::XMLTokenEnum _eComponents
                                ,enum ::xmloff::token::XMLTokenEnum _eSubComponents
                                ,sal_Bool _bExportContext
                                ,const ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >& _aMemFunc
                                )
{
    if ( _xCollection.is() )
    {
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<SvXMLElementExport> pComponents;
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if ( _bExportContext )
            pComponents.reset( new SvXMLElementExport(*this,XML_NAMESPACE_DB, _eComponents, sal_True, sal_True));
        Sequence< OUString> aSeq = _xCollection->getElementNames();
        const OUString* pIter = aSeq.getConstArray();
        const OUString* pEnd   = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            Reference<XPropertySet> xProp(_xCollection->getByName(*pIter),UNO_QUERY);
            if ( _bExportContext && XML_TABLE_REPRESENTATIONS != _eComponents )
                AddAttribute(XML_NAMESPACE_DB, XML_NAME,*pIter);
            Reference< XNameAccess > xSub(xProp,UNO_QUERY);
            if ( xSub.is() )
            {
                exportCollection(xSub,_eSubComponents,_eSubComponents,_bExportContext,_aMemFunc);
            }
            else if ( xProp.is() )
                _aMemFunc(this,xProp.get());
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportComponent(XPropertySet* _xProp)
{
    OUString sValue;
    _xProp->getPropertyValue(PROPERTY_PERSISTENT_NAME) >>= sValue;
    sal_Bool bIsForm = sal_True;
    _xProp->getPropertyValue("IsForm") >>= bIsForm;
    if ( bIsForm )
        sValue = OUString("forms/") + sValue;
    else
        sValue = OUString("reports/") + sValue;

    AddAttribute(XML_NAMESPACE_XLINK, XML_HREF,sValue);
    sal_Bool bAsTemplate = sal_False;
    _xProp->getPropertyValue(PROPERTY_AS_TEMPLATE) >>= bAsTemplate;
    AddAttribute(XML_NAMESPACE_DB, XML_AS_TEMPLATE,bAsTemplate ? XML_TRUE : XML_FALSE);
    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_COMPONENT, sal_True, sal_True);
}
// -----------------------------------------------------------------------------
void ODBExport::exportQuery(XPropertySet* _xProp)
{
    AddAttribute(XML_NAMESPACE_DB, XML_COMMAND,getString(_xProp->getPropertyValue(PROPERTY_COMMAND)));

    if ( getBOOL(_xProp->getPropertyValue(PROPERTY_APPLYFILTER)) )
        AddAttribute(XML_NAMESPACE_DB, XML_APPLY_FILTER,XML_TRUE);

    if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_APPLYORDER)
        && getBOOL(_xProp->getPropertyValue(PROPERTY_APPLYORDER)) )
        AddAttribute(XML_NAMESPACE_DB, XML_APPLY_ORDER,XML_TRUE);

    if ( ! getBOOL(_xProp->getPropertyValue(PROPERTY_ESCAPE_PROCESSING)) )
        AddAttribute(XML_NAMESPACE_DB, XML_ESCAPE_PROCESSING,XML_FALSE);

    exportStyleName(_xProp,GetAttrList());

    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_QUERY, sal_True, sal_True);
    Reference<XColumnsSupplier> xCol(_xProp,UNO_QUERY);
    exportColumns(xCol);
    exportFilter(_xProp,PROPERTY_FILTER,XML_FILTER_STATEMENT);
    exportFilter(_xProp,PROPERTY_ORDER,XML_ORDER_STATEMENT);
    exportTableName(_xProp,sal_True);
}
// -----------------------------------------------------------------------------
void ODBExport::exportTable(XPropertySet* _xProp)
{
    exportTableName(_xProp,sal_False);

    if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION) )
        AddAttribute(XML_NAMESPACE_DB, XML_DESCRIPTION,getString(_xProp->getPropertyValue(PROPERTY_DESCRIPTION)));

    if ( getBOOL(_xProp->getPropertyValue(PROPERTY_APPLYFILTER)) )
        AddAttribute(XML_NAMESPACE_DB, XML_APPLY_FILTER,XML_TRUE);

    if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_APPLYORDER)
        && getBOOL(_xProp->getPropertyValue(PROPERTY_APPLYORDER)) )
        AddAttribute(XML_NAMESPACE_DB, XML_APPLY_ORDER,XML_TRUE);

    exportStyleName(_xProp,GetAttrList());

    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_TABLE_REPRESENTATION, sal_True, sal_True);
    Reference<XColumnsSupplier> xCol(_xProp,UNO_QUERY);
    exportColumns(xCol);
    exportFilter(_xProp,PROPERTY_FILTER,XML_FILTER_STATEMENT);
    exportFilter(_xProp,PROPERTY_ORDER,XML_ORDER_STATEMENT);
}
// -----------------------------------------------------------------------------
void ODBExport::exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt)
{
    Reference<XPropertySet> xFind(_xProp);
    exportStyleName(XML_STYLE_NAME,xFind,_rAtt,m_aAutoStyleNames);
    exportStyleName(XML_DEFAULT_CELL_STYLE_NAME,xFind,_rAtt,m_aCellAutoStyleNames);
    exportStyleName(XML_DEFAULT_ROW_STYLE_NAME,xFind,_rAtt,m_aRowAutoStyleNames);
}
// -----------------------------------------------------------------------------
void ODBExport::exportStyleName(const ::xmloff::token::XMLTokenEnum _eToken,const uno::Reference<beans::XPropertySet>& _xProp,SvXMLAttributeList& _rAtt,TPropertyStyleMap& _rMap)
{
    TPropertyStyleMap::iterator aFind = _rMap.find(_xProp);
    if ( aFind != _rMap.end() )
    {
        _rAtt.AddAttribute( GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DB, GetXMLToken(_eToken) ),
                            aFind->second );
        _rMap.erase(aFind);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportTableName(XPropertySet* _xProp,sal_Bool _bUpdate)
{
    OUString sValue;
    _xProp->getPropertyValue(_bUpdate ? OUString(PROPERTY_UPDATE_TABLENAME) : OUString(PROPERTY_NAME)) >>= sValue;
    if ( !sValue.isEmpty() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_NAME,sValue);
        _xProp->getPropertyValue(_bUpdate ? OUString(PROPERTY_UPDATE_SCHEMANAME) : OUString(PROPERTY_SCHEMANAME)) >>= sValue;
        if ( !sValue.isEmpty() )
            AddAttribute(XML_NAMESPACE_DB, XML_SCHEMA_NAME,sValue);
        _xProp->getPropertyValue(_bUpdate ? OUString(PROPERTY_UPDATE_CATALOGNAME) : OUString(PROPERTY_CATALOGNAME)) >>= sValue;
        if ( !sValue.isEmpty() )
            AddAttribute(XML_NAMESPACE_DB, XML_CATALOG_NAME,sValue);

        if ( _bUpdate )
        {
            SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_UPDATE_TABLE, sal_True, sal_True);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportFilter(XPropertySet* _xProp
                             ,const OUString& _sProp
                             ,enum ::xmloff::token::XMLTokenEnum _eStatementType)
{
    OSL_PRECOND(!GetAttrList().getLength(),"Invalid attribute length!");
    OUString sCommand;
    _xProp->getPropertyValue(_sProp) >>= sCommand;
    if ( !sCommand.isEmpty() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_COMMAND,sCommand);
        SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, _eStatementType, sal_True, sal_True);
    }
    OSL_POSTCOND(!GetAttrList().getLength(),"Invalid attribute length!");
}
// -----------------------------------------------------------------------------
void ODBExport::exportColumns(const Reference<XColumnsSupplier>& _xColSup)
{
    OSL_PRECOND( _xColSup.is(), "ODBExport::exportColumns: invalid columns supplier!" );
    if ( !_xColSup.is() )
        return;

    try
    {
        Reference<XNameAccess> xNameAccess( _xColSup->getColumns(), UNO_SET_THROW );
        if ( !xNameAccess->hasElements() )
        {
            Reference< XPropertySet > xComponent(_xColSup,UNO_QUERY);
            TTableColumnMap::iterator aFind = m_aTableDummyColumns.find(xComponent);
            if ( aFind != m_aTableDummyColumns.end() )
            {
                SvXMLElementExport aColumns(*this,XML_NAMESPACE_DB, XML_COLUMNS, sal_True, sal_True);
                SvXMLAttributeList* pAtt = new SvXMLAttributeList;
                Reference<XAttributeList> xAtt = pAtt;
                exportStyleName(aFind->second.get(),*pAtt);
                AddAttributeList(xAtt);
                SvXMLElementExport aColumn(*this,XML_NAMESPACE_DB, XML_COLUMN, sal_True, sal_True);

            }
            return;
        }

        SvXMLElementExport aColumns(*this,XML_NAMESPACE_DB, XML_COLUMNS, sal_True, sal_True);
        Sequence< OUString> aSeq = xNameAccess->getElementNames();
        const OUString* pIter = aSeq.getConstArray();
        const OUString* pEnd   = pIter + aSeq.getLength();
        for( ; pIter != pEnd ; ++pIter)
        {
            Reference<XPropertySet> xProp(xNameAccess->getByName(*pIter),UNO_QUERY);
            if ( xProp.is() )
            {
                SvXMLAttributeList* pAtt = new SvXMLAttributeList;
                Reference<XAttributeList> xAtt = pAtt;
                exportStyleName(xProp.get(),*pAtt);

                sal_Bool bHidden = getBOOL(xProp->getPropertyValue(PROPERTY_HIDDEN));

                OUString sValue;
                xProp->getPropertyValue(PROPERTY_HELPTEXT) >>= sValue;
                Any aColumnDefault;
                aColumnDefault = xProp->getPropertyValue(PROPERTY_CONTROLDEFAULT);

                if ( bHidden || !sValue.isEmpty() || aColumnDefault.hasValue() || pAtt->getLength() )
                {
                    AddAttribute(XML_NAMESPACE_DB, XML_NAME,*pIter);
                    if ( bHidden )
                        AddAttribute(XML_NAMESPACE_DB, XML_VISIBLE,XML_FALSE);

                    if ( !sValue.isEmpty() )
                        AddAttribute(XML_NAMESPACE_DB, XML_HELP_MESSAGE,sValue);

                    if ( aColumnDefault.hasValue() )
                    {
                        OUStringBuffer sColumnDefaultString,sType;
                        ::sax::Converter::convertAny(
                            sColumnDefaultString, sType, aColumnDefault );
                        AddAttribute(XML_NAMESPACE_DB, XML_TYPE_NAME,sType.makeStringAndClear());
                        AddAttribute(XML_NAMESPACE_DB, XML_DEFAULT_VALUE,sColumnDefaultString.makeStringAndClear());
                    }

                    if ( pAtt->getLength() )
                        AddAttributeList(xAtt);
                }

                if ( GetAttrList().getLength() )
                {
                    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_COLUMN, sal_True, sal_True);
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportForms()
{
    Any aValue;
    OUString sService;
    dbtools::getDataSourceSetting(getDataSource(),"Forms",aValue);
    aValue >>= sService;
    if ( sService.isEmpty() )
    {
        Reference<XFormDocumentsSupplier> xSup(GetModel(),UNO_QUERY);
        if ( xSup.is() )
        {
            Reference< XNameAccess > xCollection = xSup->getFormDocuments();
            if ( xCollection.is() && xCollection->hasElements() )
            {
                ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportComponent);
                exportCollection(xCollection,XML_FORMS,XML_COMPONENT_COLLECTION,sal_True,aMemFunc);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportReports()
{
    Any aValue;
    OUString sService;
    dbtools::getDataSourceSetting(getDataSource(),"Reports",aValue);
    aValue >>= sService;
    if ( sService.isEmpty() )
    {
        Reference<XReportDocumentsSupplier> xSup(GetModel(),UNO_QUERY);
        if ( xSup.is() )
        {
            Reference< XNameAccess > xCollection = xSup->getReportDocuments();
            if ( xCollection.is() && xCollection->hasElements() )
            {
                ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportComponent);
                exportCollection(xCollection,XML_REPORTS,XML_COMPONENT_COLLECTION,sal_True,aMemFunc);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportQueries(sal_Bool _bExportContext)
{
    Any aValue;
    OUString sService;
    dbtools::getDataSourceSetting(getDataSource(),"CommandDefinitions",aValue);
    aValue >>= sService;
    if ( sService.isEmpty() )
    {
        Reference<XQueryDefinitionsSupplier> xSup(getDataSource(),UNO_QUERY);
        if ( xSup.is() )
        {
            Reference< XNameAccess > xCollection = xSup->getQueryDefinitions();
            if ( xCollection.is() && xCollection->hasElements() )
            {
                SAL_WNODEPRECATED_DECLARATIONS_PUSH
                ::std::auto_ptr< ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > > pMemFunc;
                SAL_WNODEPRECATED_DECLARATIONS_POP
                if ( _bExportContext )
                    pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportQuery) );
                else
                    pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportAutoStyle) );

                exportCollection(xCollection,XML_QUERIES,XML_QUERY_COLLECTION,_bExportContext,*pMemFunc);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportTables(sal_Bool _bExportContext)
{
    Reference<XTablesSupplier> xSup(getDataSource(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getTables();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            SAL_WNODEPRECATED_DECLARATIONS_PUSH
            ::std::auto_ptr< ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > > pMemFunc;
            SAL_WNODEPRECATED_DECLARATIONS_POP
            if ( _bExportContext )
                pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportTable) );
            else
                pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportAutoStyle) );
            exportCollection(xCollection,XML_TABLE_REPRESENTATIONS,XML_TOKEN_INVALID,_bExportContext,*pMemFunc);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportAutoStyle(XPropertySet* _xProp)
{
    typedef ::std::pair<TPropertyStyleMap*,sal_uInt16> TEnumMapperPair;
    typedef ::std::pair< UniReference < SvXMLExportPropertyMapper> , TEnumMapperPair> TExportPropMapperPair;
    Reference<XColumnsSupplier> xSup(_xProp,UNO_QUERY);
    if ( xSup.is() )
    {
        const TExportPropMapperPair pExportHelper[] = {
             TExportPropMapperPair(m_xExportHelper,TEnumMapperPair(&m_aAutoStyleNames,XML_STYLE_FAMILY_TABLE_TABLE ))
            // ,TExportPropMapperPair(m_xCellExportHelper,TEnumMapperPair(&m_aCellAutoStyleNames,XML_STYLE_FAMILY_TABLE_CELL))
            ,TExportPropMapperPair(m_xRowExportHelper,TEnumMapperPair(&m_aRowAutoStyleNames,XML_STYLE_FAMILY_TABLE_ROW))
        };

        ::std::vector< XMLPropertyState > aPropertyStates;
        for (size_t i = 0 ; i < sizeof(pExportHelper)/sizeof(pExportHelper[0]); ++i)
        {
            aPropertyStates = pExportHelper[i].first->Filter(_xProp);
            if ( !aPropertyStates.empty() )
                pExportHelper[i].second.first->insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( pExportHelper[i].second.second, aPropertyStates )));
        }

        Reference< XNameAccess > xCollection;
        try
        {
            xCollection.set( xSup->getColumns(), UNO_SET_THROW );
            awt::FontDescriptor aFont;
            _xProp->getPropertyValue(PROPERTY_FONT) >>= aFont;
            GetFontAutoStylePool()->Add(aFont.Name,aFont.StyleName,static_cast<FontFamily>(aFont.Family),
                static_cast<FontPitch>(aFont.Pitch),aFont.CharSet );

            m_aCurrentPropertyStates = m_xCellExportHelper->Filter(_xProp);
            if ( !m_aCurrentPropertyStates.empty() && !xCollection->hasElements() )
            {
                Reference< XDataDescriptorFactory> xFac(xCollection,UNO_QUERY);
                if ( xFac.is() )
                {
                    Reference< XPropertySet> xColumn = xFac->createDataDescriptor();
                    m_aTableDummyColumns.insert(TTableColumnMap::value_type(Reference< XPropertySet>(_xProp),xColumn));
                    exportAutoStyle(xColumn.get());
                }
            }
            else
            {
                ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportAutoStyle);
                exportCollection(xCollection,XML_TOKEN_INVALID,XML_TOKEN_INVALID,sal_False,aMemFunc);
            }
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        m_aCurrentPropertyStates.clear();
    }
    else
    { // here I know I have a column
        TExportPropMapperPair pExportHelper[] = {
             TExportPropMapperPair(m_xColumnExportHelper,TEnumMapperPair(&m_aAutoStyleNames,XML_STYLE_FAMILY_TABLE_COLUMN ))
            ,TExportPropMapperPair(m_xCellExportHelper,TEnumMapperPair(&m_aCellAutoStyleNames,XML_STYLE_FAMILY_TABLE_CELL))
        };
        for (size_t i = 0 ; i < sizeof(pExportHelper)/sizeof(pExportHelper[0]); ++i)
        {
            ::std::vector< XMLPropertyState > aPropStates = pExportHelper[i].first->Filter( _xProp );
            if ( !aPropStates.empty() )
            {
                ::std::vector< XMLPropertyState >::iterator aItr = aPropStates.begin();
                ::std::vector< XMLPropertyState >::iterator aEnd = aPropStates.end();
                const UniReference < XMLPropertySetMapper >& pStyle = pExportHelper[i].first->getPropertySetMapper();
                while ( aItr != aEnd )
                {
                    if ( aItr->mnIndex != -1 )
                    {
                        switch ( pStyle->GetEntryContextId(aItr->mnIndex) )
                        {
                            case CTF_DB_NUMBERFORMAT:
                                {
                                    sal_Int32 nNumberFormat = -1;
                                    if ( aItr->maValue >>= nNumberFormat )
                                        addDataStyle(nNumberFormat);
                                }
                                break;
                            case CTF_DB_COLUMN_TEXT_ALIGN:
                                if ( !aItr->maValue.hasValue() )
                                    aItr->maValue <<= ::com::sun::star::awt::TextAlign::LEFT;
                                break;
                        }
                    }
                    ++aItr;
                }

            }
            if ( XML_STYLE_FAMILY_TABLE_CELL == pExportHelper[i].second.second )
                ::std::copy( m_aCurrentPropertyStates.begin(), m_aCurrentPropertyStates.end(), ::std::back_inserter( aPropStates ));
            if ( !aPropStates.empty() )
                pExportHelper[i].second.first->insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( pExportHelper[i].second.second, aPropStates )));
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::_ExportContent()
{
    exportDataSource();
    exportForms();
    exportReports();
    exportQueries(sal_True);
    exportTables(sal_True);
}
// -----------------------------------------------------------------------------
void ODBExport::_ExportMasterStyles()
{
    GetPageExport()->exportMasterStyles( sal_True );
}
// -----------------------------------------------------------------------------
void ODBExport::_ExportAutoStyles()
{
    // there are no styles that require their own autostyles
    if ( getExportFlags() & EXPORT_CONTENT )
    {
        collectComponentStyles();
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_TABLE
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_COLUMN
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_CELL
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        GetAutoStylePool()->exportXML(XML_STYLE_FAMILY_TABLE_ROW
                                        ,GetDocHandler()
                                        ,GetMM100UnitConverter()
                                        ,GetNamespaceMap()
        );
        exportDataStyles();
    }
}
// -----------------------------------------------------------------------------
void ODBExport::_ExportStyles(sal_Bool bUsed)
{
    SvXMLExport::_ExportStyles(bUsed);
}
// -----------------------------------------------------------------------------
sal_uInt32 ODBExport::exportDoc(enum ::xmloff::token::XMLTokenEnum eClass)
{
    return SvXMLExport::exportDoc( eClass );
}
// -----------------------------------------------------------------------------
void ODBExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    Reference<XQueryDefinitionsSupplier> xSup(getDataSource(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getQueryDefinitions();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            try
            {
                sal_Int32 nLength = aProps.getLength();
                aProps.realloc(nLength + 1);
                aProps[nLength].Name = OUString("Queries");
                Sequence< OUString> aSeq = xCollection->getElementNames();
                const OUString* pIter = aSeq.getConstArray();
                const OUString* pEnd   = pIter + aSeq.getLength();

                Sequence<PropertyValue> aQueries(aSeq.getLength());
                for(sal_Int32 i = 0;pIter != pEnd;++pIter,++i)
                {
                    Reference<XPropertySet> xProp(xCollection->getByName(*pIter),UNO_QUERY);
                    if ( xProp.is() )
                    {
                        aQueries[i].Name = *pIter;
                        aQueries[i].Value = xProp->getPropertyValue(PROPERTY_LAYOUTINFORMATION);
                    }
                }
                aProps[nLength].Value <<= aQueries;
            }
            catch(const Exception&)
            {
                OSL_FAIL("ODBExport::GetViewSettings: Exception caught!");
            }
        }
    }

}
// -----------------------------------------------------------------------------
void ODBExport::GetConfigurationSettings(Sequence<PropertyValue>& aProps)
{
    Reference<XPropertySet> xProp(getDataSource());
    if ( xProp.is() )
    {
        sal_Int32 nLength = aProps.getLength();
        try
        {
            Any aValue = xProp->getPropertyValue(PROPERTY_LAYOUTINFORMATION);
            Sequence< PropertyValue > aPropValues;
            aValue >>= aPropValues;
            if ( aPropValues.getLength() )
            {
                aProps.realloc(nLength + 1);
                aProps[nLength].Name = OUString("layout-settings");
                aProps[nLength].Value = aValue;
            }
        }
        catch(const Exception&)
        {
            OSL_FAIL("Could not access layout information from the data source!");
        }
    }
}
// -----------------------------------------------------------------------------
OUString ODBExport::implConvertAny(const Any& _rValue)
{
    OUStringBuffer aBuffer;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_STRING:
        {   // extract the string
            OUString sCurrentValue;
            _rValue >>= sCurrentValue;
            aBuffer.append(sCurrentValue);
        }
        break;
        case TypeClass_DOUBLE:
            // let the unit converter format is as string
            ::sax::Converter::convertDouble(aBuffer, getDouble(_rValue));
            break;
        case TypeClass_BOOLEAN:
            aBuffer = getBOOL(_rValue) ? ::xmloff::token::GetXMLToken(XML_TRUE) : ::xmloff::token::GetXMLToken(XML_FALSE);
            break;
        case TypeClass_BYTE:
        case TypeClass_SHORT:
        case TypeClass_LONG:
            // let the unit converter format is as string
            ::sax::Converter::convertNumber(aBuffer, getINT32(_rValue));
            break;
        default:
            OSL_FAIL("ODBExport::implConvertAny: Invalid type");
    }

    return aBuffer.makeStringAndClear();
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ODBExport::GetTableStylesPropertySetMapper() const
{
    if ( !m_xTableStylesPropertySetMapper.is() )
    {
        m_xTableStylesPropertySetMapper = OXMLHelper::GetTableStylesPropertySetMapper();
    }
    return m_xTableStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ODBExport::GetCellStylesPropertySetMapper() const
{
    if ( !m_xCellStylesPropertySetMapper.is() )
    {
        m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylesPropertySetMapper();
    }
    return m_xCellStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
UniReference < XMLPropertySetMapper > ODBExport::GetColumnStylesPropertySetMapper() const
{
    if ( !m_xColumnStylesPropertySetMapper.is() )
    {
        m_xColumnStylesPropertySetMapper = OXMLHelper::GetColumnStylesPropertySetMapper();
    }
    return m_xColumnStylesPropertySetMapper;
}
// -----------------------------------------------------------------------------
SvXMLAutoStylePoolP* ODBExport::CreateAutoStylePool()
{
    return new OXMLAutoStylePoolP(*this);
}
// -----------------------------------------------------------------------------
void SAL_CALL ODBExport::setSourceDocument( const Reference< XComponent >& xDoc ) throw(IllegalArgumentException, RuntimeException)
{
    Reference<XOfficeDatabaseDocument> xOfficeDoc(xDoc,UNO_QUERY_THROW);
    m_xDataSource.set(xOfficeDoc->getDataSource(),UNO_QUERY_THROW);
    OSL_ENSURE(m_xDataSource.is(),"DataSource is NULL!");
    Reference< XNumberFormatsSupplier > xNum(m_xDataSource->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
    SetNumberFormatsSupplier(xNum);
    SvXMLExport::setSourceDocument(xDoc);
}
// -----------------------------------------------------------------------------
void ODBExport::_ExportFontDecls()
{
    GetFontAutoStylePool(); // make sure the pool is created
    collectComponentStyles();
    SvXMLExport::_ExportFontDecls();
}
// -----------------------------------------------------------------------------
void ODBExport::collectComponentStyles()
{
    if ( m_bAllreadyFilled )
        return;

    m_bAllreadyFilled = sal_True;
    exportQueries(sal_False);
    exportTables(sal_False);
}
// -----------------------------------------------------------------------------
}// dbaxml
// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
