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
#include <sax/tools/converter.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/namespacemap.hxx>
#include <comphelper/string.hxx>
#include <comphelper/types.hxx>
#include <strings.hxx>
#include <sal/log.hxx>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
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

#include <optional>
#include <memory>

using namespace comphelper;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star;

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sdb_DBExportFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::dbaxml::ODBExport(context,
        "com.sun.star.comp.sdb.DBExportFilter"));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sdb_XMLSettingsExporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::dbaxml::ODBExport(context,
        "com.sun.star.comp.sdb.XMLSettingsExporter",
        SvXMLExportFlags::SETTINGS | SvXMLExportFlags::PRETTY ));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_sdb_XMLFullExporter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new ::dbaxml::ODBExport(context,
        "com.sun.star.comp.sdb.XMLFullExporter",
        SvXMLExportFlags::ALL));
}

namespace dbaxml
{
    static OUString lcl_implGetPropertyXMLType(const Type& _rType)
    {
        // possible types we can write (either because we recognize them directly or because we convert _rValue
        // into one of these types)

        // handle the type description
        switch (_rType.getTypeClass())
        {
            case TypeClass_STRING:
                return "string";
            case TypeClass_DOUBLE:
                return "double";
            case TypeClass_BOOLEAN:
                return "boolean";
            case TypeClass_BYTE:
            case TypeClass_SHORT:
                return "short";
            case TypeClass_LONG:
                return "int";
            case TypeClass_HYPER:
                return "long";
            case TypeClass_ENUM:
                return "int";

            default:
                OSL_FAIL( "lcl_implGetPropertyXMLType: unsupported value type!" );
                return "double";
        }
    }

    namespace {

    class OSpecialHandleXMLExportPropertyMapper : public SvXMLExportPropertyMapper
    {
    public:
        explicit OSpecialHandleXMLExportPropertyMapper(const rtl::Reference< XMLPropertySetMapper >& rMapper) : SvXMLExportPropertyMapper(rMapper )
        {
        }
        /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
        virtual void handleSpecialItem(
                SvXMLAttributeList& /*rAttrList*/,
                const XMLPropertyState& /*rProperty*/,
                const SvXMLUnitConverter& /*rUnitConverter*/,
                const SvXMLNamespaceMap& /*rNamespaceMap*/,
                const std::vector< XMLPropertyState > * /*pProperties*/ ,
                sal_uInt32 /*nIdx*/ ) const override
        {
            // nothing to do here
        }
    };

    }

ODBExport::ODBExport(const Reference< XComponentContext >& _rxContext, OUString const & implementationName, SvXMLExportFlags nExportFlag)
: SvXMLExport( _rxContext, implementationName, util::MeasureUnit::MM_10TH, XML_DATABASE,
        SvXMLExportFlags::OASIS | nExportFlag)
,m_aTypeCollection(_rxContext)
,m_bAllreadyFilled(false)
{
    GetMM100UnitConverter().SetCoreMeasureUnit(util::MeasureUnit::MM_10TH);
    GetMM100UnitConverter().SetXMLMeasureUnit(util::MeasureUnit::CM);

    GetNamespaceMap_().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken(XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_SVG), GetXMLToken(XML_N_SVG), XML_NAMESPACE_SVG );

    GetNamespaceMap_().Add( GetXMLToken(XML_NP_DB), GetXMLToken(XML_N_DB_OASIS), XML_NAMESPACE_DB );

    if( nExportFlag & (SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::FONTDECLS) )
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO_COMPAT), XML_NAMESPACE_FO );

    if( nExportFlag & (SvXMLExportFlags::META|SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::SCRIPTS|SvXMLExportFlags::SETTINGS) )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_XLINK), GetXMLToken(XML_N_XLINK), XML_NAMESPACE_XLINK );
    }
    if( nExportFlag & SvXMLExportFlags::SETTINGS )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_CONFIG), GetXMLToken(XML_N_CONFIG), XML_NAMESPACE_CONFIG );
    }

    if( nExportFlag & (SvXMLExportFlags::STYLES|SvXMLExportFlags::MASTERSTYLES|SvXMLExportFlags::AUTOSTYLES|SvXMLExportFlags::CONTENT|SvXMLExportFlags::FONTDECLS) )
    {
        GetNamespaceMap_().Add( GetXMLToken(XML_NP_STYLE), GetXMLToken(XML_N_STYLE), XML_NAMESPACE_STYLE );
    }

    GetNamespaceMap_().Add( GetXMLToken(XML_NP_TABLE), GetXMLToken(XML_N_TABLE), XML_NAMESPACE_TABLE );
    GetNamespaceMap_().Add( GetXMLToken(XML_NP_NUMBER), GetXMLToken(XML_N_NUMBER), XML_NAMESPACE_NUMBER );

    m_xExportHelper = new SvXMLExportPropertyMapper(GetTableStylesPropertySetMapper());
    m_xColumnExportHelper = new OSpecialHandleXMLExportPropertyMapper(GetColumnStylesPropertySetMapper());

    m_xCellExportHelper = new OSpecialHandleXMLExportPropertyMapper(GetCellStylesPropertySetMapper());
    m_xRowExportHelper = new OSpecialHandleXMLExportPropertyMapper(OXMLHelper::GetRowStylesPropertySetMapper());

    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::TABLE_TABLE,
        OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME ),
        m_xExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::TABLE_COLUMN,
        OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ),
        m_xColumnExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::TABLE_CELL,
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_NAME ),
        m_xCellExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_CELL_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XmlStyleFamily::TABLE_ROW,
        OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_NAME ),
        m_xRowExportHelper.get(),
        OUString(XML_STYLE_FAMILY_TABLE_ROW_STYLES_PREFIX ));
}

void ODBExport::exportDataSource()
{
    try
    {
        Reference<XPropertySet> xProp( getDataSource(), UNO_SET_THROW );

        bool bAutoIncrementEnabled = true;
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
        const ::comphelper::NamedValueCollection& aDriverSupportedProperties( aDriverConfig.getProperties( sURL ) );

        static OUString s_sTrue(::xmloff::token::GetXMLToken( XML_TRUE ));
        static OUString s_sFalse(::xmloff::token::GetXMLToken( XML_FALSE ));
        // loop through the properties, and export only those which are not defaulted
        TSettingsMap aSettingsMap;
        // Don't try to get XPropertySetInfo from xProp, simply wrap the attempt into try block
        try
        {
            const Any aValue = xProp->getPropertyValue(PROPERTY_SUPPRESSVERSIONCL);
            if (!getBOOL(aValue)) // default in the XML schema is true -> only write false
                aSettingsMap.emplace(XML_SUPPRESS_VERSION_COLUMNS, s_sFalse);
        }
        catch (const UnknownPropertyException&)
        {
        }

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
                const ::std::optional< OUString >  aXMLDefault;

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

            const PropertyMap aTokens[] =
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
                PropertyMap( INFO_CONN_LDAP_BASEDN,     XML_BASE_DN                                     ),
                PropertyMap( INFO_CONN_LDAP_ROWCOUNT,   XML_MAX_ROW_COUNT                               )
            };

            bool bIsXMLDefault = false;
            for (const auto & aToken : aTokens)
            {
                if ( pProperties->Name == aToken.sPropertyName )
                {
                    eToken = aToken.eAttributeToken;

                    if  (   !!aToken.aXMLDefault
                        &&  ( sValue == *aToken.aXMLDefault )
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
                    if ( sValue == "0" )
                        sValue = "equal-integer";
                    else if ( sValue == "1" )
                        sValue = "is-boolean";
                    else if ( sValue == "2" )
                        sValue = "equal-boolean";
                    else if ( sValue == "3" )
                        sValue = "equal-use-only-zero";
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
                        m_aDataSourceSettings.emplace_back(
                            pProperties->Name, pProperties->Type, aValue );
                    }
                    continue;
                }
            }

            aSettingsMap.emplace(eToken,sValue);
        }
        if ( bAutoIncrementEnabled && !(aAutoIncrement.first.isEmpty() && aAutoIncrement.second.isEmpty()) )
            m_aAutoIncrement.reset( new TStringPair(aAutoIncrement));
        if ( aDelimiter.bUsed )
            m_aDelimiter.reset( new TDelimiter( aDelimiter ) );

        SvXMLElementExport aElem(*this, XML_NAMESPACE_DB, XML_DATA_SOURCE, true, true);

        exportConnectionData();
        exportDriverSettings(aSettingsMap);
        exportApplicationConnectionSettings(aSettingsMap);
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

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
    for (::xmloff::token::XMLTokenEnum i : pSettings)
    {
        TSettingsMap::const_iterator aFind = _aSettings.find(i);
        if ( aFind != _aSettings.end() )
            AddAttribute(XML_NAMESPACE_DB, aFind->first,aFind->second);
    }
    SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_APPLICATION_CONNECTION_SETTINGS, true, true);

    Reference<XPropertySet> xProp(getDataSource());
    Sequence< OUString> aValue;
    xProp->getPropertyValue(PROPERTY_TABLEFILTER) >>= aValue;
    if ( aValue.hasElements() )
    {
        SvXMLElementExport aElem2(*this,XML_NAMESPACE_DB, XML_TABLE_FILTER, true, true);
        exportSequence(aValue,XML_TABLE_INCLUDE_FILTER,XML_TABLE_FILTER_PATTERN);
    }

    xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER) >>= aValue;
    if ( aValue.hasElements() )
        exportSequence(aValue,XML_TABLE_TYPE_FILTER,XML_TABLE_TYPE);

    exportDataSourceSettings();
}

void ODBExport::exportDriverSettings(const TSettingsMap& _aSettings)
{
    const ::xmloff::token::XMLTokenEnum pSettings[] = {
        XML_SHOW_DELETED
        ,XML_SYSTEM_DRIVER_SETTINGS
        ,XML_BASE_DN
        ,XML_IS_FIRST_ROW_HEADER_LINE
        ,XML_PARAMETER_NAME_SUBSTITUTION
    };
    for (::xmloff::token::XMLTokenEnum nSetting : pSettings)
    {
        TSettingsMap::const_iterator aFind = _aSettings.find(nSetting);
        if ( aFind != _aSettings.end() )
            AddAttribute(XML_NAMESPACE_DB, aFind->first,aFind->second);
    }
    SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DRIVER_SETTINGS, true, true);
    exportAutoIncrement();
    exportDelimiter();
    exportCharSet();
}

void ODBExport::exportConnectionData()
{
    SvXMLElementExport aConnData(*this,XML_NAMESPACE_DB, XML_CONNECTION_DATA, true, true);

    {
        OUString sValue;
        Reference<XPropertySet> xProp(getDataSource());
        xProp->getPropertyValue(PROPERTY_URL) >>= sValue;
        if ( m_aTypeCollection.isFileSystemBased(sValue) )
        {
            SvXMLElementExport aDatabaseDescription(*this,XML_NAMESPACE_DB, XML_DATABASE_DESCRIPTION, true, true);
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
                        sExtension = "mdb";
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
                SvXMLElementExport aFileBasedDB(*this,XML_NAMESPACE_DB, XML_FILE_BASED_DATABASE, true, true);
            }
        }
        else
        {
            OUString sDatabaseName,sHostName;
            sal_Int32 nPort = -1;
            m_aTypeCollection.extractHostNamePort(sValue,sDatabaseName,sHostName,nPort);
            if ( sHostName.getLength() )
            {
                SvXMLElementExport aDatabaseDescription(*this,XML_NAMESPACE_DB, XML_DATABASE_DESCRIPTION, true, true);
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


                        const OUString sPropertyName = "LocalSocket";
                        if ( xSettingsInfo->hasPropertyByName( sPropertyName ) )
                        {
                            OUString sPropertyValue;
                            if ( ( xDataSourceSettings->getPropertyValue( sPropertyName ) >>= sPropertyValue ) && !sPropertyValue.isEmpty() )
                                AddAttribute( XML_NAMESPACE_DB, XML_LOCAL_SOCKET, sPropertyValue );
                        }
                    }
                    catch( const Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION("dbaccess");
                    }

                    SvXMLElementExport aServerDB(*this,XML_NAMESPACE_DB, XML_SERVER_DATABASE, true, true);
                }
            }
            else
            {
                AddAttribute(XML_NAMESPACE_XLINK, XML_HREF,sValue);
                AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
                SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_CONNECTION_RESOURCE, true, true);
            }
        }

    }

    exportLogin();
}

template< typename T > void ODBExport::exportDataSourceSettingsSequence(
    std::vector< TypedPropertyValue >::iterator const & in)
{
    css::uno::Sequence<T> anySeq;
    bool bSuccess = in->Value >>= anySeq;
    assert(bSuccess); (void)bSuccess;
    for (T const & i : anySeq )
    {
        SvXMLElementExport aDataValue(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE, true, false);
        // (no whitespace inside the tag)
        Characters(implConvertAny(css::uno::Any(i)));
    }
}

void ODBExport::exportDataSourceSettings()
{
    if ( m_aDataSourceSettings.empty() )
        return;

    SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTINGS, true, true);
    std::vector< TypedPropertyValue >::iterator aIter = m_aDataSourceSettings.begin();
    std::vector< TypedPropertyValue >::const_iterator aEnd = m_aDataSourceSettings.end();
    for ( ; aIter != aEnd; ++aIter )
    {
        const bool bIsSequence = TypeClass_SEQUENCE == aIter->Type.getTypeClass();

        Type aSimpleType(bIsSequence ? comphelper::getSequenceElementType(aIter->Value.getValueType()) : aIter->Type);

        AddAttribute( XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_IS_LIST,bIsSequence ? XML_TRUE : XML_FALSE );
        AddAttribute( XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_NAME, aIter->Name );

        OUString sTypeName = lcl_implGetPropertyXMLType( aSimpleType );
        if ( bIsSequence && aSimpleType.getTypeClass() == TypeClass_ANY )
        {
            Sequence<Any> aSeq;
            aIter->Value >>= aSeq;
            if ( aSeq.hasElements() )
                sTypeName = lcl_implGetPropertyXMLType(aSeq[0].getValueType());
        }

        AddAttribute( XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_TYPE, sTypeName );

        SvXMLElementExport aDataSourceSetting( *this, XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING, true, true );

        if ( !bIsSequence )
        {
            SvXMLElementExport aDataValue( *this, XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE, true, false );
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

void ODBExport::exportCharSet()
{
    if ( !m_sCharSet.isEmpty() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_ENCODING,m_sCharSet);

        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_FONT_CHARSET, true, true);
    }
}

void ODBExport::exportDelimiter()
{
    if ( m_aDelimiter && m_aDelimiter->bUsed )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_FIELD,m_aDelimiter->sField);
        AddAttribute(XML_NAMESPACE_DB, XML_STRING,m_aDelimiter->sText);
        AddAttribute(XML_NAMESPACE_DB, XML_DECIMAL,m_aDelimiter->sDecimal);
        AddAttribute(XML_NAMESPACE_DB, XML_THOUSAND,m_aDelimiter->sThousand);
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DELIMITER, true, true);
    }
}

void ODBExport::exportAutoIncrement()
{
    if (m_aAutoIncrement)
    {
        AddAttribute(XML_NAMESPACE_DB, XML_ADDITIONAL_COLUMN_STATEMENT,m_aAutoIncrement->second);
        AddAttribute(XML_NAMESPACE_DB, XML_ROW_RETRIEVING_STATEMENT,m_aAutoIncrement->first);
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_AUTO_INCREMENT, true, true);
    }
}

void ODBExport::exportSequence(const Sequence< OUString>& _aValue
                            ,::xmloff::token::XMLTokenEnum _eTokenFilter
                            ,::xmloff::token::XMLTokenEnum _eTokenType)
{
    if ( _aValue.hasElements() )
    {
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, _eTokenFilter, true, true);

        const OUString* pIter = _aValue.getConstArray();
        const OUString* pEnd   = pIter + _aValue.getLength();
        for(;pIter != pEnd;++pIter)
        {
            SvXMLElementExport aDataSource(*this,XML_NAMESPACE_DB, _eTokenType, true, false);
            Characters(*pIter);
        }
    }
}

void ODBExport::exportLogin()
{
    Reference<XPropertySet> xProp(getDataSource());
    OUString sValue;
    xProp->getPropertyValue(PROPERTY_USER) >>= sValue;
    bool bAddLogin = !sValue.isEmpty();
    if ( bAddLogin )
        AddAttribute(XML_NAMESPACE_DB, XML_USER_NAME,sValue);
    bool bValue = false;
    if ( xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED) >>= bValue )
    {
        bAddLogin = true;
        AddAttribute(XML_NAMESPACE_DB, XML_IS_PASSWORD_REQUIRED,bValue ? XML_TRUE : XML_FALSE);
    }
    if ( bAddLogin )
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_LOGIN, true, true);
}

void ODBExport::exportCollection(const Reference< XNameAccess >& _xCollection
                                ,enum ::xmloff::token::XMLTokenEnum _eComponents
                                ,enum ::xmloff::token::XMLTokenEnum _eSubComponents
                                ,bool _bExportContext
                                ,const ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >& _aMemFunc
                                )
{
    if ( !_xCollection.is() )
        return;

    std::unique_ptr<SvXMLElementExport> pComponents;
    if ( _bExportContext )
        pComponents.reset( new SvXMLElementExport(*this,XML_NAMESPACE_DB, _eComponents, true, true));
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

void ODBExport::exportComponent(XPropertySet* _xProp)
{
    OUString sValue;
    _xProp->getPropertyValue(PROPERTY_PERSISTENT_NAME) >>= sValue;
    bool bIsForm = true;
    _xProp->getPropertyValue("IsForm") >>= bIsForm;
    if ( bIsForm )
        sValue = "forms/" + sValue;
    else
        sValue = "reports/" + sValue;

    AddAttribute(XML_NAMESPACE_XLINK, XML_HREF, sValue);
    AddAttribute(XML_NAMESPACE_XLINK, XML_TYPE, XML_SIMPLE);
    bool bAsTemplate = false;
    _xProp->getPropertyValue(PROPERTY_AS_TEMPLATE) >>= bAsTemplate;
    AddAttribute(XML_NAMESPACE_DB, XML_AS_TEMPLATE,bAsTemplate ? XML_TRUE : XML_FALSE);
    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_COMPONENT, true, true);
}

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

    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_QUERY, true, true);
    Reference<XColumnsSupplier> xCol(_xProp,UNO_QUERY);
    exportColumns(xCol);
    exportFilter(_xProp,PROPERTY_FILTER,XML_FILTER_STATEMENT);
    exportFilter(_xProp,PROPERTY_ORDER,XML_ORDER_STATEMENT);
    exportTableName(_xProp,true);
}

void ODBExport::exportTable(XPropertySet* _xProp)
{
    exportTableName(_xProp,false);

    if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_DESCRIPTION) )
        AddAttribute(XML_NAMESPACE_DB, XML_DESCRIPTION,getString(_xProp->getPropertyValue(PROPERTY_DESCRIPTION)));

    if ( getBOOL(_xProp->getPropertyValue(PROPERTY_APPLYFILTER)) )
        AddAttribute(XML_NAMESPACE_DB, XML_APPLY_FILTER,XML_TRUE);

    if ( _xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_APPLYORDER)
        && getBOOL(_xProp->getPropertyValue(PROPERTY_APPLYORDER)) )
        AddAttribute(XML_NAMESPACE_DB, XML_APPLY_ORDER,XML_TRUE);

    exportStyleName(_xProp,GetAttrList());

    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_TABLE_REPRESENTATION, true, true);
    Reference<XColumnsSupplier> xCol(_xProp,UNO_QUERY);
    exportColumns(xCol);
    exportFilter(_xProp,PROPERTY_FILTER,XML_FILTER_STATEMENT);
    exportFilter(_xProp,PROPERTY_ORDER,XML_ORDER_STATEMENT);
}

void ODBExport::exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt)
{
    Reference<XPropertySet> xFind(_xProp);
    exportStyleName(XML_STYLE_NAME,xFind,_rAtt,m_aAutoStyleNames);
    exportStyleName(XML_DEFAULT_CELL_STYLE_NAME,xFind,_rAtt,m_aCellAutoStyleNames);
    exportStyleName(XML_DEFAULT_ROW_STYLE_NAME,xFind,_rAtt,m_aRowAutoStyleNames);
}

void ODBExport::exportStyleName(const ::xmloff::token::XMLTokenEnum _eToken,const uno::Reference<beans::XPropertySet>& _xProp,SvXMLAttributeList& _rAtt,TPropertyStyleMap& _rMap)
{
    TPropertyStyleMap::const_iterator aFind = _rMap.find(_xProp);
    if ( aFind != _rMap.end() )
    {
        _rAtt.AddAttribute( GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DB, GetXMLToken(_eToken) ),
                            aFind->second );
        _rMap.erase(aFind);
    }
}

void ODBExport::exportTableName(XPropertySet* _xProp,bool _bUpdate)
{
    OUString sValue;
    _xProp->getPropertyValue(_bUpdate ? OUString(PROPERTY_UPDATE_TABLENAME) : OUString(PROPERTY_NAME)) >>= sValue;
    if ( sValue.isEmpty() )
        return;

    AddAttribute(XML_NAMESPACE_DB, XML_NAME,sValue);
    _xProp->getPropertyValue(_bUpdate ? OUString(PROPERTY_UPDATE_SCHEMANAME) : OUString(PROPERTY_SCHEMANAME)) >>= sValue;
    if ( !sValue.isEmpty() )
        AddAttribute(XML_NAMESPACE_DB, XML_SCHEMA_NAME,sValue);
    _xProp->getPropertyValue(_bUpdate ? OUString(PROPERTY_UPDATE_CATALOGNAME) : OUString(PROPERTY_CATALOGNAME)) >>= sValue;
    if ( !sValue.isEmpty() )
        AddAttribute(XML_NAMESPACE_DB, XML_CATALOG_NAME,sValue);

    if ( _bUpdate )
    {
        SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_UPDATE_TABLE, true, true);
    }
}

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
        SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, _eStatementType, true, true);
    }
    SAL_WARN_IF(GetAttrList().getLength(), "dbaccess", "Invalid attribute length!");
}

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
            TTableColumnMap::const_iterator aFind = m_aTableDummyColumns.find(xComponent);
            if ( aFind != m_aTableDummyColumns.end() )
            {
                SvXMLElementExport aColumns(*this,XML_NAMESPACE_DB, XML_COLUMNS, true, true);
                SvXMLAttributeList* pAtt = new SvXMLAttributeList;
                Reference<XAttributeList> xAtt = pAtt;
                exportStyleName(aFind->second.get(),*pAtt);
                AddAttributeList(xAtt);
                SvXMLElementExport aColumn(*this,XML_NAMESPACE_DB, XML_COLUMN, true, true);

            }
            return;
        }

        SvXMLElementExport aColumns(*this,XML_NAMESPACE_DB, XML_COLUMNS, true, true);
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

                bool bHidden = getBOOL(xProp->getPropertyValue(PROPERTY_HIDDEN));

                OUString sValue;
                xProp->getPropertyValue(PROPERTY_HELPTEXT) >>= sValue;
                Any aColumnDefault = xProp->getPropertyValue(PROPERTY_CONTROLDEFAULT);

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
                    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_COLUMN, true, true);
                }
            }
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION("dbaccess");
    }
}

void ODBExport::exportForms()
{
    Any aValue;
    OUString sService;
    dbtools::getDataSourceSetting(getDataSource(),"Forms",aValue);
    aValue >>= sService;
    if ( !sService.isEmpty() )
        return;

    Reference<XFormDocumentsSupplier> xSup(GetModel(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getFormDocuments();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportComponent);
            exportCollection(xCollection,XML_FORMS,XML_COMPONENT_COLLECTION,true,aMemFunc);
        }
    }
}

void ODBExport::exportReports()
{
    Any aValue;
    OUString sService;
    dbtools::getDataSourceSetting(getDataSource(),"Reports",aValue);
    aValue >>= sService;
    if ( !sService.isEmpty() )
        return;

    Reference<XReportDocumentsSupplier> xSup(GetModel(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getReportDocuments();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportComponent);
            exportCollection(xCollection,XML_REPORTS,XML_COMPONENT_COLLECTION,true,aMemFunc);
        }
    }
}

void ODBExport::exportQueries(bool _bExportContext)
{
    Any aValue;
    OUString sService;
    dbtools::getDataSourceSetting(getDataSource(),"CommandDefinitions",aValue);
    aValue >>= sService;
    if ( !sService.isEmpty() )
        return;

    Reference<XQueryDefinitionsSupplier> xSup(getDataSource(),UNO_QUERY);
    if ( !xSup.is() )
        return;

    Reference< XNameAccess > xCollection = xSup->getQueryDefinitions();
    if ( xCollection.is() && xCollection->hasElements() )
    {
        std::unique_ptr< ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > > pMemFunc;
        if ( _bExportContext )
            pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportQuery) );
        else
            pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportAutoStyle) );

        exportCollection(xCollection,XML_QUERIES,XML_QUERY_COLLECTION,_bExportContext,*pMemFunc);
    }
}

void ODBExport::exportTables(bool _bExportContext)
{
    Reference<XTablesSupplier> xSup(getDataSource(),UNO_QUERY);
    if ( !xSup.is() )
        return;

    Reference< XNameAccess > xCollection = xSup->getTables();
    if ( xCollection.is() && xCollection->hasElements() )
    {
        std::unique_ptr< ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > > pMemFunc;
        if ( _bExportContext )
            pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportTable) );
        else
            pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportAutoStyle) );
        exportCollection(xCollection,XML_TABLE_REPRESENTATIONS,XML_TOKEN_INVALID,_bExportContext,*pMemFunc);
    }
}

void ODBExport::exportAutoStyle(XPropertySet* _xProp)
{
    typedef std::pair<TPropertyStyleMap*,XmlStyleFamily> TEnumMapperPair;
    typedef std::pair< rtl::Reference < SvXMLExportPropertyMapper> , TEnumMapperPair> TExportPropMapperPair;
    Reference<XColumnsSupplier> xSup(_xProp,UNO_QUERY);
    if ( xSup.is() )
    {
        const TExportPropMapperPair pExportHelper[] = {
             TExportPropMapperPair(m_xExportHelper,TEnumMapperPair(&m_aAutoStyleNames,XmlStyleFamily::TABLE_TABLE ))
            // ,TExportPropMapperPair(m_xCellExportHelper,TEnumMapperPair(&m_aCellAutoStyleNames,XmlStyleFamily::TABLE_CELL))
            ,TExportPropMapperPair(m_xRowExportHelper,TEnumMapperPair(&m_aRowAutoStyleNames,XmlStyleFamily::TABLE_ROW))
        };

        std::vector< XMLPropertyState > aPropertyStates;
        for (const auto & i : pExportHelper)
        {
            aPropertyStates = i.first->Filter(*this, _xProp);
            if ( !aPropertyStates.empty() )
                i.second.first->emplace( _xProp,GetAutoStylePool()->Add( i.second.second, aPropertyStates ) );
        }

        Reference< XNameAccess > xCollection;
        try
        {
            xCollection.set( xSup->getColumns(), UNO_SET_THROW );
            awt::FontDescriptor aFont;
            _xProp->getPropertyValue(PROPERTY_FONT) >>= aFont;
            GetFontAutoStylePool()->Add(aFont.Name,aFont.StyleName,static_cast<FontFamily>(aFont.Family),
                static_cast<FontPitch>(aFont.Pitch),aFont.CharSet );

            m_aCurrentPropertyStates = m_xCellExportHelper->Filter(*this, _xProp);
            if ( !m_aCurrentPropertyStates.empty() && !xCollection->hasElements() )
            {
                Reference< XDataDescriptorFactory> xFac(xCollection,UNO_QUERY);
                if ( xFac.is() )
                {
                    Reference< XPropertySet> xColumn = xFac->createDataDescriptor();
                    m_aTableDummyColumns.emplace( Reference< XPropertySet>(_xProp),xColumn );
                    exportAutoStyle(xColumn.get());
                }
            }
            else
            {
                ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportAutoStyle);
                exportCollection(xCollection,XML_TOKEN_INVALID,XML_TOKEN_INVALID,false,aMemFunc);
            }
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION("dbaccess");
        }
        m_aCurrentPropertyStates.clear();
    }
    else
    { // here I know I have a column
        const TExportPropMapperPair pExportHelper[] = {
             TExportPropMapperPair(m_xColumnExportHelper,TEnumMapperPair(&m_aAutoStyleNames,XmlStyleFamily::TABLE_COLUMN ))
            ,TExportPropMapperPair(m_xCellExportHelper,TEnumMapperPair(&m_aCellAutoStyleNames,XmlStyleFamily::TABLE_CELL))
        };
        for (const auto & i : pExportHelper)
        {
            std::vector< XMLPropertyState > aPropStates = i.first->Filter(*this, _xProp);
            if ( !aPropStates.empty() )
            {
                const rtl::Reference < XMLPropertySetMapper >& pStyle = i.first->getPropertySetMapper();
                for (auto & propState : aPropStates)
                {
                    if ( propState.mnIndex != -1 )
                    {
                        switch ( pStyle->GetEntryContextId(propState.mnIndex) )
                        {
                            case CTF_DB_NUMBERFORMAT:
                                {
                                    sal_Int32 nNumberFormat = -1;
                                    if ( propState.maValue >>= nNumberFormat )
                                        addDataStyle(nNumberFormat);
                                }
                                break;
                            case CTF_DB_COLUMN_TEXT_ALIGN:
                                if ( !propState.maValue.hasValue() )
                                    propState.maValue <<= css::awt::TextAlign::LEFT;
                                break;
                        }
                    }
                }

            }
            if ( XmlStyleFamily::TABLE_CELL == i.second.second )
                aPropStates.insert( aPropStates.end(), m_aCurrentPropertyStates.begin(), m_aCurrentPropertyStates.end() );
            if ( !aPropStates.empty() )
                i.second.first->emplace( _xProp,GetAutoStylePool()->Add( i.second.second, aPropStates ) );
        }
    }
}

void ODBExport::ExportContent_()
{
    exportDataSource();
    exportForms();
    exportReports();
    exportQueries(true);
    exportTables(true);
}

void ODBExport::ExportMasterStyles_()
{
    GetPageExport()->exportMasterStyles( true );
}

void ODBExport::ExportAutoStyles_()
{
    // there are no styles that require their own autostyles
    if ( getExportFlags() & SvXMLExportFlags::CONTENT )
    {
        collectComponentStyles();
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_TABLE);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_COLUMN);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_CELL);
        GetAutoStylePool()->exportXML(XmlStyleFamily::TABLE_ROW);
        exportDataStyles();
    }
}

void ODBExport::GetViewSettings(Sequence<PropertyValue>& aProps)
{
    Reference<XQueryDefinitionsSupplier> xSup(getDataSource(),UNO_QUERY);
    if ( !xSup.is() )
        return;

    Reference< XNameAccess > xCollection = xSup->getQueryDefinitions();
    if ( !(xCollection.is() && xCollection->hasElements()) )
        return;

    try
    {
        sal_Int32 nLength = aProps.getLength();
        aProps.realloc(nLength + 1);
        aProps[nLength].Name = "Queries";
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

void ODBExport::GetConfigurationSettings(Sequence<PropertyValue>& aProps)
{
    Reference<XPropertySet> xProp(getDataSource());
    if ( !xProp.is() )
        return;

    sal_Int32 nLength = aProps.getLength();
    try
    {
        Any aValue = xProp->getPropertyValue(PROPERTY_LAYOUTINFORMATION);
        Sequence< PropertyValue > aPropValues;
        aValue >>= aPropValues;
        if ( aPropValues.hasElements() )
        {
            aProps.realloc(nLength + 1);
            aProps[nLength].Name = "layout-settings";
            aProps[nLength].Value = aValue;
        }
    }
    catch(const Exception&)
    {
        OSL_FAIL("Could not access layout information from the data source!");
    }
}

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
            aBuffer.append(getINT32(_rValue));
            break;
        default:
            OSL_FAIL("ODBExport::implConvertAny: Invalid type");
    }

    return aBuffer.makeStringAndClear();
}

rtl::Reference < XMLPropertySetMapper > const & ODBExport::GetTableStylesPropertySetMapper() const
{
    if ( !m_xTableStylesPropertySetMapper.is() )
    {
        m_xTableStylesPropertySetMapper = OXMLHelper::GetTableStylesPropertySetMapper( true);
    }
    return m_xTableStylesPropertySetMapper;
}

rtl::Reference < XMLPropertySetMapper > const & ODBExport::GetCellStylesPropertySetMapper() const
{
    if ( !m_xCellStylesPropertySetMapper.is() )
    {
        m_xCellStylesPropertySetMapper = OXMLHelper::GetCellStylesPropertySetMapper( true);
    }
    return m_xCellStylesPropertySetMapper;
}

rtl::Reference < XMLPropertySetMapper > const & ODBExport::GetColumnStylesPropertySetMapper() const
{
    if ( !m_xColumnStylesPropertySetMapper.is() )
    {
        m_xColumnStylesPropertySetMapper = OXMLHelper::GetColumnStylesPropertySetMapper( true);
    }
    return m_xColumnStylesPropertySetMapper;
}

SvXMLAutoStylePoolP* ODBExport::CreateAutoStylePool()
{
    return new OXMLAutoStylePoolP(*this);
}

void SAL_CALL ODBExport::setSourceDocument( const Reference< XComponent >& xDoc )
{
    Reference<XOfficeDatabaseDocument> xOfficeDoc(xDoc,UNO_QUERY_THROW);
    m_xDataSource.set(xOfficeDoc->getDataSource(),UNO_QUERY_THROW);
    OSL_ENSURE(m_xDataSource.is(),"DataSource is NULL!");
    Reference< XNumberFormatsSupplier > xNum(m_xDataSource->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
    SetNumberFormatsSupplier(xNum);
    SvXMLExport::setSourceDocument(xDoc);
}

void ODBExport::ExportFontDecls_()
{
    GetFontAutoStylePool(); // make sure the pool is created
    collectComponentStyles();
    SvXMLExport::ExportFontDecls_();
}

void ODBExport::collectComponentStyles()
{
    if ( m_bAllreadyFilled )
        return;

    m_bAllreadyFilled = true;
    exportQueries(false);
    exportTables(false);
}

}// dbaxml

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
