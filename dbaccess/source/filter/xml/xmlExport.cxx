/*************************************************************************
 *
 *  $RCSfile: xmlExport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:21:21 $
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
 *  Source License Version 1.1 (the License); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an AS IS basis,
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

#ifndef DBA_XMLEXPORT_HXX
#include "xmlExport.hxx"
#endif
#ifndef DBA_XMLAUTOSTYLE_HXX
#include "xmlAutoStyle.hxx"
#endif
#ifndef _FLT_REGHELPER_HXX_
#include "flt_reghelper.hxx"
#endif
#ifndef _XMLOFF_PROGRESSBARHELPER_HXX
#include <xmloff/ProgressBarHelper.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_TEXTIMP_HXX_
#include <xmloff/txtimp.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _COM_SUN_STAR_SDB_XFORMDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XREPORTDOCUMENTSSUPPLIER_HPP_
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XQUERYDEFINITIONSSUPPLIER_HPP_
#include <com/sun/star/sdb/XQueryDefinitionsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_XTABLESSUPPLIER_HPP_
#include <com/sun/star/sdbcx/XTablesSupplier.hpp>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef DBA_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif

namespace dbaxml
{
    using namespace comphelper;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::sdbcx;
    using namespace ::com::sun::star::util;

    class ODBExportHelper
    {
    public:
        static ::rtl::OUString SAL_CALL getImplementationName_Static(  ) throw (RuntimeException);
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(RuntimeException);
        static Reference< XInterface > SAL_CALL Create(const Reference< ::com::sun::star::lang::XMultiServiceFactory >&);
    };
    class ODBFullExportHelper
    {
    public:
        static ::rtl::OUString SAL_CALL getImplementationName_Static(  ) throw (RuntimeException);
        static Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames_Static(  ) throw(RuntimeException);
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
        return static_cast< XServiceInfo* >(new ODBExport(_rxORB,EXPORT_SETTINGS | EXPORT_PRETTY ));
    }
    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ODBExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.sdb.XMLSettingsExporter");
    }
    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ODBExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExportFilter");
        return aSupported;
    }


    //---------------------------------------------------------------------
    Reference< XInterface > SAL_CALL ODBFullExportHelper::Create(const Reference< XMultiServiceFactory >& _rxORB)
    {
        return static_cast< XServiceInfo* >(new ODBExport(_rxORB,EXPORT_ALL));
    }
    //---------------------------------------------------------------------
    ::rtl::OUString SAL_CALL ODBFullExportHelper::getImplementationName_Static(  ) throw (RuntimeException)
    {
        return ::rtl::OUString::createFromAscii("com.sun.star.comp.sdb.XMLFullExporter");
    }
    //---------------------------------------------------------------------
    Sequence< ::rtl::OUString > SAL_CALL ODBFullExportHelper::getSupportedServiceNames_Static(  ) throw(RuntimeException)
    {
        Sequence< ::rtl::OUString > aSupported(1);
        aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExportFilter");
        return aSupported;
    }

    //---------------------------------------------------------------------
    ::rtl::OUString lcl_implGetPropertyXMLType(const Type& _rType)
    {
        // possible types we can write (either because we recognize them directly or because we convert _rValue
        // into one of these types)
        static const ::rtl::OUString s_sTypeBoolean (RTL_CONSTASCII_USTRINGPARAM("boolean"));
        static const ::rtl::OUString s_sTypeShort   (RTL_CONSTASCII_USTRINGPARAM("short"));
        static const ::rtl::OUString s_sTypeInteger (RTL_CONSTASCII_USTRINGPARAM("int"));
        static const ::rtl::OUString s_sTypeLong    (RTL_CONSTASCII_USTRINGPARAM("long"));
        static const ::rtl::OUString s_sTypeDouble  (RTL_CONSTASCII_USTRINGPARAM("double"));
        static const ::rtl::OUString s_sTypeString  (RTL_CONSTASCII_USTRINGPARAM("string"));

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
                return s_sTypeDouble;
        }
    }
// -----------------------------------------------------------------------------
ODBExport::ODBExport(const Reference< XMultiServiceFactory >& _rxMSF,sal_uInt16 nExportFlag)
: SvXMLExport( _rxMSF,MAP_10TH_MM,XML_DATABASE, EXPORT_OASIS)
{
    setExportFlags( EXPORT_OASIS | nExportFlag);
    GetMM100UnitConverter().setCoreMeasureUnit(MAP_10TH_MM);

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OFFICE), GetXMLToken((getExportFlags() & EXPORT_CONTENT) != 0 ? XML_N_OOO : XML_N_OFFICE), XML_NAMESPACE_OFFICE );
    _GetNamespaceMap().Add( GetXMLToken(XML_NP_OOO), GetXMLToken(XML_N_OOO), XML_NAMESPACE_OOO );

    _GetNamespaceMap().Add( GetXMLToken(XML_NP_DB), GetXMLToken(XML_N_DB), XML_NAMESPACE_DB );

    if( (nExportFlag & (EXPORT_STYLES|EXPORT_MASTERSTYLES|EXPORT_AUTOSTYLES|EXPORT_FONTDECLS) ) != 0 )
        _GetNamespaceMap().Add( GetXMLToken(XML_NP_FO), GetXMLToken(XML_N_FO), XML_NAMESPACE_FO );

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
    m_xColumnExportHelper = new SvXMLExportPropertyMapper(GetColumnStylesPropertySetMapper());

    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_TABLE_TABLE,
        rtl::OUString::createFromAscii( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME ),
        m_xExportHelper.get(),
        rtl::OUString::createFromAscii( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_PREFIX ));

    GetAutoStylePool()->AddFamily(
        XML_STYLE_FAMILY_TABLE_COLUMN,
        rtl::OUString::createFromAscii( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ),
        m_xExportHelper.get(),
        rtl::OUString::createFromAscii( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_PREFIX ));
}
// -----------------------------------------------------------------------------
IMPLEMENT_SERVICE_INFO1_STATIC( ODBExport, "com.sun.star.comp.sdb.DBExportFilter", "com.sun.star.document.ExportFilter")
// -----------------------------------------------------------------------------
void ODBExport::exportDataSource()
{
    Reference<XPropertySet> xProp(GetModel(),UNO_QUERY);
    if ( xProp.is() )
    {
        ::rtl::OUString sValue;
        xProp->getPropertyValue(PROPERTY_URL) >>= sValue;
        AddAttribute(XML_NAMESPACE_DB, XML_CONNECTION_RESOURCE,sValue);

        sal_Bool bSupress = sal_False;
        xProp->getPropertyValue(PROPERTY_SUPPRESSVERSIONCL) >>= bSupress;
        if ( bSupress )
            AddAttribute(XML_NAMESPACE_DB, XML_SUPPRESS_VERSION_COLUMNS,XML_TRUE);

        sal_Bool bAutoIncrementEnabled = sal_True;
        TStringPair aAutoIncrement;
        TDelimiter aDelimiter;
        Sequence< PropertyValue> aInfo;
        xProp->getPropertyValue(PROPERTY_INFO) >>= aInfo;
        const PropertyValue* pIter = aInfo.getConstArray();
        const PropertyValue* pEnd  = pIter + aInfo.getLength();
        for(;pIter != pEnd;++pIter)
        {
            switch ( pIter->Value.getValueTypeClass() )
            {
                case TypeClass_STRING:
                    pIter->Value >>= sValue;
                break;
                case TypeClass_DOUBLE:
                    // let the unit converter format is as string
                    sValue = ::rtl::OUString::valueOf(getDouble(pIter->Value));
                    break;
                case TypeClass_BOOLEAN:
                    sValue = ::xmloff::token::GetXMLToken(getBOOL(pIter->Value) ? XML_TRUE : XML_FALSE);
                    break;
                case TypeClass_BYTE:
                case TypeClass_SHORT:
                case TypeClass_LONG:
                    // let the unit converter format is as string
                    sValue = ::rtl::OUString::valueOf(getINT32(pIter->Value));
                    break;
            }

            ::xmloff::token::XMLTokenEnum eToken = XML_TOKEN_INVALID;
            if ( pIter->Name == INFO_JDBCDRIVERCLASS )
                eToken = XML_JAVA_DRIVER_CLASS;
            else if ( pIter->Name == INFO_TEXTFILEEXTENSION )
                eToken = XML_EXTENSION;
            else if ( pIter->Name == INFO_TEXTFILEHEADER )
                eToken = XML_IS_FIRST_ROW_HEADER_LINE;
            else if ( pIter->Name == INFO_SHOWDELETEDROWS )
                eToken = XML_SHOW_DELETED;
            else if ( pIter->Name == INFO_ALLOWLONGTABLENAMES )
                eToken = XML_IS_TABLE_NAME_LENGTH_LIMITED;
            else if ( pIter->Name == INFO_ADDITIONALOPTIONS )
                eToken = XML_SYSTEM_DRIVER_SETTINGS;
            else if ( pIter->Name == PROPERTY_ENABLESQL92CHECK )
                eToken = XML_ENABLE_SQL92_CHECK;
            else if ( pIter->Name == INFO_APPEND_TABLE_ALIAS )
                eToken = XML_APPEND_TABLE_ALIAS_NAME;
            else if ( pIter->Name == INFO_PARAMETERNAMESUBST )
                eToken = XML_PARAMETER_NAME_SUBSTITUTION;
            else if ( pIter->Name == INFO_IGNOREDRIVER_PRIV )
                eToken = XML_IGNORE_DRIVER_PRIVILEGES;
            else if ( pIter->Name == PROPERTY_BOOLEANCOMPARISONMODE )
            {
                sal_Int32 nValue = 0;
                pIter->Value >>= nValue;
                if ( sValue.equalsAscii("0") )
                    sValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("equal-integer"));
                else if ( sValue.equalsAscii("1") )
                    sValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("is-boolean"));
                else if ( sValue.equalsAscii("2") )
                    sValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("equal-boolean"));
                else if ( sValue.equalsAscii("3") )
                    sValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("equal-use-only-zero"));
                eToken = XML_BOOLEAN_COMPARISON_MODE;
            }
            else if ( pIter->Name == INFO_USECATALOG )
                eToken = XML_USE_CATALOG;
            else if ( pIter->Name == INFO_CONN_LDAP_BASEDN )
                eToken = XML_BASE_DN;
            else if ( pIter->Name == INFO_CONN_LDAP_ROWCOUNT )
                eToken = XML_MAX_ROW_COUNT;
            else if ( pIter->Name == INFO_AUTORETRIEVEENABLED )
            {
                pIter->Value >>= bAutoIncrementEnabled;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_AUTORETRIEVEVALUE )
            {
                aAutoIncrement.first = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == PROPERTY_AUTOINCREMENTCREATION )
            {
                aAutoIncrement.second = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_TEXTDELIMITER )
            {
                aDelimiter.sText = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_FIELDDELIMITER )
            {
                aDelimiter.sField = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_DECIMALDELIMITER )
            {
                aDelimiter.sDecimal = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_THOUSANDSDELIMITER )
            {
                aDelimiter.sThousand = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_CHARSET )
            {
                m_sCharSet = sValue;
                // special handling
                continue;
            }
            else if ( pIter->Name == INFO_PREVIEW )
            {
                m_aPreviewMode = pIter->Value;
                continue;
            }
            else
            {
                m_aDataSourceSettings.push_back(makeAny(*pIter));
                // special handling
                continue;
            }

            AddAttribute(XML_NAMESPACE_DB, eToken,sValue);
        }
        if ( bAutoIncrementEnabled && (aAutoIncrement.first.getLength() || aAutoIncrement.second.getLength()) )
            m_aAutoIncrement.reset( new TStringPair(aAutoIncrement));
        if (    aDelimiter.sText.getLength()
            ||  aDelimiter.sField.getLength()
            ||  aDelimiter.sDecimal.getLength()
            ||  aDelimiter.sThousand.getLength() )
        {
            m_aDelimiter.reset( new TDelimiter(aDelimiter));
        }

        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DATASOURCE, sal_True, sal_True);

        exportLogin();

        Sequence< ::rtl::OUString> aValue;
        xProp->getPropertyValue(PROPERTY_TABLEFILTER) >>= aValue;
        exportSequence(aValue,XML_TABLE_FILTER,XML_TABLE_FILTER_PATTERN);

        xProp->getPropertyValue(PROPERTY_TABLETYPEFILTER) >>= aValue;
        exportSequence(aValue,XML_TABLE_TYPE_FILTER,XML_TABLE_TYPE);

        exportAutoIncrement();
        exportDelimiter();
        exportCharSet();
        exportDataSourceSettings();
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportDataSourceSettings()
{
    if ( !m_aDataSourceSettings.empty() )
    {
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTINGS, sal_True, sal_True);
        PropertyValue aValue;
        ::std::vector< Any>::iterator aIter = m_aDataSourceSettings.begin();
        ::std::vector< Any>::iterator aEnd = m_aDataSourceSettings.end();
        for (; aIter != aEnd; ++aIter)
        {
            *aIter >>= aValue;
            sal_Bool bIsSequence = TypeClass_SEQUENCE == aValue.Value.getValueTypeClass();

            Type aSimpleType;
            if ( bIsSequence )
                aSimpleType = ::comphelper::getSequenceElementType(aValue.Value.getValueType());
            else
                aSimpleType = aValue.Value.getValueType();

            AddAttribute(XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_IS_LIST,bIsSequence ? XML_TRUE : XML_FALSE);
            AddAttribute(XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_NAME,aValue.Name);
            ::rtl::OUString sTypeName = lcl_implGetPropertyXMLType(aSimpleType);
            AddAttribute(XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_TYPE,sTypeName);

            SvXMLElementExport aDataSourceSetting(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING, sal_True, sal_True);

            if ( !bIsSequence )
            {
                SvXMLElementExport aDataValue(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE, sal_True, sal_True);
                // (no whitespace inside the tag)
                Characters(implConvertAny(aValue.Value));
            }
            else
            {
                // the not-that-simple case, we need to iterate through the sequence elements
                ::std::auto_ptr<IIterator> pSequenceIterator;
                switch (aSimpleType.getTypeClass())
                {
                    case TypeClass_STRING:
                        pSequenceIterator.reset(new OSequenceIterator< ::rtl::OUString >(aValue.Value));
                        break;
                    case TypeClass_DOUBLE:
                        pSequenceIterator.reset(new OSequenceIterator< double >(aValue.Value));
                        break;
                    case TypeClass_BOOLEAN:
                        pSequenceIterator.reset(new OSequenceIterator< sal_Bool >(aValue.Value));
                        break;
                    case TypeClass_BYTE:
                        pSequenceIterator.reset(new OSequenceIterator< sal_Int8 >(aValue.Value));
                        break;
                    case TypeClass_SHORT:
                        pSequenceIterator.reset(new OSequenceIterator< sal_Int16 >(aValue.Value));
                        break;
                    case TypeClass_LONG:
                        pSequenceIterator.reset(new OSequenceIterator< sal_Int32 >(aValue.Value));
                        break;
                    default:
                        OSL_ENSURE(sal_False, "unsupported sequence type !");
                        break;
                }
                if ( pSequenceIterator.get() )
                {
                    ::rtl::OUString sCurrent;
                    while (pSequenceIterator->hasMoreElements())
                    {
                        SvXMLElementExport aDataValue(*this,XML_NAMESPACE_DB, XML_DATA_SOURCE_SETTING_VALUE, sal_True, sal_True);
                        // (no whitespace inside the tag)
                        Characters(implConvertAny(pSequenceIterator->nextElement()));
                    }
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportCharSet()
{
    if ( m_sCharSet.getLength() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_ENCODING,m_sCharSet);

        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_FONT_CHARSET, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportDelimiter()
{
    if ( m_aDelimiter.get() )
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
void ODBExport::exportSequence(const Sequence< ::rtl::OUString>& _aValue
                            ,::xmloff::token::XMLTokenEnum _eTokenFilter
                            ,::xmloff::token::XMLTokenEnum _eTokenType)
{
    Reference<XPropertySet> xProp(GetModel(),UNO_QUERY);
    Sequence< ::rtl::OUString> aValue;
    if ( _aValue.getLength() )
    {
        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, _eTokenFilter, sal_True, sal_True);

        const ::rtl::OUString* pIter = _aValue.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + _aValue.getLength();
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
    Reference<XPropertySet> xProp(GetModel(),UNO_QUERY);
    ::rtl::OUString sValue;
    xProp->getPropertyValue(PROPERTY_USER) >>= sValue;
    if ( sValue.getLength() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_USER_NAME,sValue);
        sal_Bool bValue = sal_False;
        xProp->getPropertyValue(PROPERTY_ISPASSWORDREQUIRED) >>= bValue;
        AddAttribute(XML_NAMESPACE_DB, XML_IS_PASSWORD_REQUIRED,bValue ? XML_TRUE : XML_FALSE);

        SvXMLElementExport aElem(*this,XML_NAMESPACE_DB, XML_LOGIN, sal_True, sal_True);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportCollection(const Reference< XNameAccess >& _xCollection
                                ,enum ::xmloff::token::XMLTokenEnum _eComponents
                                ,enum ::xmloff::token::XMLTokenEnum _eSubComponents
                                ,sal_Bool _bExportContext
                                ,const ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >& _aMemFunc)
{
    if ( _xCollection.is() )
    {
        ::std::auto_ptr<SvXMLElementExport> pComponents;
        if ( _bExportContext )
            pComponents.reset( new SvXMLElementExport(*this,XML_NAMESPACE_DB, _eComponents, sal_True, sal_True));
        Sequence< ::rtl::OUString> aSeq = _xCollection->getElementNames();
        const ::rtl::OUString* pIter = aSeq.getConstArray();
        const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
        for(;pIter != pEnd;++pIter)
        {
            Reference<XPropertySet> xProp(_xCollection->getByName(*pIter),UNO_QUERY);
            if ( _bExportContext && XML_TABLES != _eComponents )
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
    ::rtl::OUString sValue;
    _xProp->getPropertyValue(PROPERTY_PERSISTENT_NAME) >>= sValue;
    sal_Bool bIsForm = sal_True;
    _xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("IsForm"))) >>= bIsForm;
    if ( bIsForm )
        sValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("forms/")) + sValue;
    else
        sValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("reports/")) + sValue;

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

    if ( ! getBOOL(_xProp->getPropertyValue(PROPERTY_USE_ESCAPE_PROCESSING)) )
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

    SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_TABLE, sal_True, sal_True);
    Reference<XColumnsSupplier> xCol(_xProp,UNO_QUERY);
    exportColumns(xCol);
    exportFilter(_xProp,PROPERTY_FILTER,XML_FILTER_STATEMENT);
    exportFilter(_xProp,PROPERTY_ORDER,XML_ORDER_STATEMENT);
}
// -----------------------------------------------------------------------------
void ODBExport::exportStyleName(XPropertySet* _xProp,SvXMLAttributeList& _rAtt)
{
    TPropertyStyleMap::iterator aFind = m_aAutoStyleNames.find(_xProp);
    if ( aFind != m_aAutoStyleNames.end() )
    {
        _rAtt.AddAttribute( GetNamespaceMap().GetQNameByKey( XML_NAMESPACE_DB, GetXMLToken(XML_STYLE_NAME) ),
                            aFind->second );
        m_aAutoStyleNames.erase(aFind);
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportTableName(XPropertySet* _xProp,sal_Bool _bUpdate)
{
    ::rtl::OUString sValue;
    _xProp->getPropertyValue(_bUpdate ? PROPERTY_UPDATE_TABLENAME : PROPERTY_NAME) >>= sValue;
    if ( sValue.getLength() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_NAME,sValue);
        _xProp->getPropertyValue(_bUpdate ? PROPERTY_UPDATE_SCHEMANAME : PROPERTY_SCHEMANAME) >>= sValue;
        if ( sValue.getLength() )
            AddAttribute(XML_NAMESPACE_DB, XML_SCHEMA_NAME,sValue);
        _xProp->getPropertyValue(_bUpdate ? PROPERTY_UPDATE_CATALOGNAME : PROPERTY_CATALOGNAME) >>= sValue;
        if ( sValue.getLength() )
            AddAttribute(XML_NAMESPACE_DB, XML_CATALOG_NAME,sValue);

        if ( _bUpdate )
        {
            SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, XML_UPDATE_TABLE, sal_True, sal_True);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportFilter(XPropertySet* _xProp
                             ,const ::rtl::OUString& _sProp
                             ,enum ::xmloff::token::XMLTokenEnum _eStatementType)
{
    OSL_PRECOND(!GetAttrList().getLength(),"Invalid attribute length!");
    ::rtl::OUString sCommand;
    _xProp->getPropertyValue(_sProp) >>= sCommand;
    if ( sCommand.getLength() )
    {
        AddAttribute(XML_NAMESPACE_DB, XML_COMMAND,sCommand);
        SvXMLElementExport aComponents(*this,XML_NAMESPACE_DB, _eStatementType, sal_True, sal_True);
    }
    OSL_POSTCOND(!GetAttrList().getLength(),"Invalid attribute length!");
}
// -----------------------------------------------------------------------------
void ODBExport::exportColumns(const Reference<XColumnsSupplier>& _xColSup)
{
    if ( _xColSup.is() )
    {
        Reference<XNameAccess> xNameAccess = _xColSup->getColumns();
        if ( xNameAccess.is() && xNameAccess->hasElements() )
        {
            SvXMLElementExport aColumns(*this,XML_NAMESPACE_DB, XML_COLUMNS, sal_True, sal_True);
            Sequence< ::rtl::OUString> aSeq = xNameAccess->getElementNames();
            const ::rtl::OUString* pIter = aSeq.getConstArray();
            const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();
            for( ; pIter != pEnd ; ++pIter)
            {
                Reference<XPropertySet> xProp(xNameAccess->getByName(*pIter),UNO_QUERY);
                if ( xProp.is() )
                {
                    SvXMLAttributeList* pAtt = new SvXMLAttributeList;
                    Reference<XAttributeList> xAtt = pAtt;
                    exportStyleName(xProp.get(),*pAtt);

                    sal_Bool bHidden = getBOOL(xProp->getPropertyValue(PROPERTY_HIDDEN));

                    ::rtl::OUString sValue;
                    xProp->getPropertyValue(PROPERTY_HELPTEXT) >>= sValue;

                    if ( bHidden || sValue.getLength() || pAtt->getLength() )
                    {
                        AddAttribute(XML_NAMESPACE_DB, XML_NAME,*pIter);
                        if ( bHidden )
                            AddAttribute(XML_NAMESPACE_DB, XML_VISIBILITY,XML_COLLAPSE);

                        if ( sValue.getLength() )
                            AddAttribute(XML_NAMESPACE_DB, XML_HELP_MESSAGE,sValue);

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
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportForms()
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
// -----------------------------------------------------------------------------
void ODBExport::exportReports()
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
// -----------------------------------------------------------------------------
void ODBExport::exportQueries(sal_Bool _bExportContext)
{
    Reference<XQueryDefinitionsSupplier> xSup(GetModel(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getQueryDefinitions();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            ::std::auto_ptr< ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > > pMemFunc;
            if ( _bExportContext )
                pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportQuery) );
            else
                pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportAutoStyle) );

            exportCollection(xCollection,XML_QUERIES,XML_QUERY_COLLECTION,_bExportContext,*pMemFunc);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportTables(sal_Bool _bExportContext)
{
    Reference<XTablesSupplier> xSup(GetModel(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getTables();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            ::std::auto_ptr< ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > > pMemFunc;
            if ( _bExportContext )
                pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportTable) );
            else
                pMemFunc.reset( new ::comphelper::mem_fun1_t<ODBExport,XPropertySet* >(&ODBExport::exportAutoStyle) );
            exportCollection(xCollection,XML_TABLES,XML_TOKEN_INVALID,_bExportContext,*pMemFunc);
        }
    }
}
// -----------------------------------------------------------------------------
void ODBExport::exportAutoStyle(XPropertySet* _xProp)
{
    ::std::vector< XMLPropertyState > aPropertyStates( m_xExportHelper->Filter(_xProp) );

    Reference<XColumnsSupplier> xSup(_xProp,UNO_QUERY);
    if ( xSup.is() )
    {
        if ( !aPropertyStates.empty() )
            m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( XML_STYLE_FAMILY_TABLE_TABLE, aPropertyStates )));
        Reference< XNameAccess > xCollection = xSup->getColumns();
        ::comphelper::mem_fun1_t<ODBExport,XPropertySet* > aMemFunc(&ODBExport::exportAutoStyle);
        exportCollection(xCollection,XML_TOKEN_INVALID,XML_TOKEN_INVALID,sal_False,aMemFunc);
    }
    else
    { // here I know I have a column
        ::std::vector< XMLPropertyState > aPropStates = m_xColumnExportHelper->Filter( _xProp );
        if ( !aPropStates.empty() )
        {
            ::std::vector< XMLPropertyState >::iterator aItr = aPropStates.begin();
            ::std::vector< XMLPropertyState >::iterator aEnd = aPropStates.end();
            UniReference < XMLPropertySetMapper > pColumnStyle = GetColumnStylesPropertySetMapper();
            sal_Int32 nCount(0);
            while ( aItr != aEnd )
            {
                if ( aItr->mnIndex != -1 )
                {
                    switch ( pColumnStyle->GetEntryContextId(aItr->mnIndex) )
                    {
                        case CTF_DB_NUMBERFORMAT:
                            {
                                sal_Int32 nNumberFormat = -1;
                                if ( aItr->maValue >>= nNumberFormat )
                                    addDataStyle(nNumberFormat);
                            }
                            break;
                    }
                }
                ++aItr;
            }
            m_aAutoStyleNames.insert( TPropertyStyleMap::value_type(_xProp,GetAutoStylePool()->Add( XML_STYLE_FAMILY_TABLE_COLUMN, aPropStates )));
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
        exportQueries(sal_False);
        exportTables(sal_False);
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
        exportDataStyles();
    }
}
// -----------------------------------------------------------------------------
void ODBExport::_ExportStyles(BOOL bUsed)
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
    Reference<XQueryDefinitionsSupplier> xSup(GetModel(),UNO_QUERY);
    if ( xSup.is() )
    {
        Reference< XNameAccess > xCollection = xSup->getQueryDefinitions();
        if ( xCollection.is() && xCollection->hasElements() )
        {
            try
            {
                sal_Int32 nLength = aProps.getLength();
                aProps.realloc(nLength + 1);
                aProps[nLength].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Queries"));
                Sequence< ::rtl::OUString> aSeq = xCollection->getElementNames();
                const ::rtl::OUString* pIter = aSeq.getConstArray();
                const ::rtl::OUString* pEnd   = pIter + aSeq.getLength();

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
            catch(Exception)
            {
                OSL_ENSURE(0,"ODBExport::GetViewSettings: Exception catched!");
            }
        }
    }

}
// -----------------------------------------------------------------------------
void ODBExport::GetConfigurationSettings(Sequence<PropertyValue>& aProps)
{
    Reference<XPropertySet> xProp(GetModel(),UNO_QUERY);
    if ( xProp.is() )
    {
        sal_Int32 nLength = aProps.getLength();
        try
        {
            aProps.realloc(nLength + 1);
            aProps[nLength].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("layout-settings"));
            aProps[nLength].Value = xProp->getPropertyValue(PROPERTY_LAYOUTINFORMATION);
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Could not access layout information from the data source!");
        }
    }
}
// -----------------------------------------------------------------------------
::rtl::OUString ODBExport::implConvertAny(const Any& _rValue)
{
    ::rtl::OUStringBuffer aBuffer;
    switch (_rValue.getValueTypeClass())
    {
        case TypeClass_STRING:
        {   // extract the string
            ::rtl::OUString sCurrentValue;
            _rValue >>= sCurrentValue;
            aBuffer.append(sCurrentValue);
        }
        break;
        case TypeClass_DOUBLE:
            // let the unit converter format is as string
            GetMM100UnitConverter().convertDouble(aBuffer, getDouble(_rValue));
            break;
        case TypeClass_BOOLEAN:
            aBuffer = getBOOL(_rValue) ? ::xmloff::token::GetXMLToken(XML_TRUE) : ::xmloff::token::GetXMLToken(XML_FALSE);
            break;
        case TypeClass_BYTE:
        case TypeClass_SHORT:
        case TypeClass_LONG:
            // let the unit converter format is as string
            GetMM100UnitConverter().convertNumber(aBuffer, getINT32(_rValue));
            break;
        default:
            OSL_ENSURE(0,"ODBExport::implConvertAny: Invalid type");
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
    Reference<XPropertySet> xProp(xDoc,UNO_QUERY);
    Reference< XNumberFormatsSupplier > xNum(xProp->getPropertyValue(PROPERTY_NUMBERFORMATSSUPPLIER),UNO_QUERY);
    SetNumberFormatsSupplier(xNum);
    SvXMLExport::setSourceDocument(xDoc);
}
// -----------------------------------------------------------------------------
}// dbaxml
// -----------------------------------------------------------------------------
