/*************************************************************************
 *
 *  $RCSfile: xmlDataSource.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:19:19 $
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
#ifndef DBA_XMLDATASOURCE_HXX
#include "xmlDataSource.hxx"
#endif
#ifndef DBA_XMLLOGIN_HXX
#include "xmlLogin.hxx"
#endif
#ifndef DBA_XMLTABLEFILTERLIST_HXX
#include "xmlTableFilterList.hxx"
#endif
#ifndef DBA_XMLDATASOURCEINFO_HXX
#include "xmlDataSourceInfo.hxx"
#endif
#ifndef DBA_XMLDATASOURCESETTINGS_HXX
#include "xmlDataSourceSettings.hxx"
#endif
#ifndef DBA_XMLDATASOURCESETTING_HXX
#include "xmlDataSourceSetting.hxx"
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef DBA_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef DBACCESS_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif

namespace dbaxml
{
    using namespace ::rtl;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;

OXMLDataSource::OXMLDataSource( ODBFilter& rImport,
                sal_uInt16 nPrfx, const OUString& _sLocalName,
                const Reference< XAttributeList > & _xAttrList ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
{
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetDataSourceElemTokenMap();

    Reference<XPropertySet> xDataSource(rImport.GetModel(),UNO_QUERY);

    PropertyValue aProperty;

    sal_Int16 nLength = (xDataSource.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        OUString sLocalName;
        rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        aProperty.Name = ::rtl::OUString();
        aProperty.Value = Any();

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_CONNECTION_RESOURCE:
                try
                {
                    xDataSource->setPropertyValue(PROPERTY_URL,makeAny(sValue));
                }
                catch(Exception)
                {
                    OSL_ENSURE(0,"Could not set property!");
                }
                break;
            case XML_TOK_SUPPRESS_VERSION_COLUMNS:
                try
                {
                    xDataSource->setPropertyValue(PROPERTY_SUPPRESSVERSIONCL,makeAny(sValue == s_sTRUE ? sal_True : sal_False));
                }
                catch(Exception)
                {
                    OSL_ENSURE(0,"Could not set property!");
                }
                break;
            case XML_TOK_JAVA_DRIVER_CLASS:
                aProperty.Name = INFO_JDBCDRIVERCLASS;
                break;
            case XML_TOK_EXTENSION:
                aProperty.Name = INFO_TEXTFILEEXTENSION;
                break;
            case XML_TOK_IS_FIRST_ROW_HEADER_LINE:
                aProperty.Name = INFO_TEXTFILEHEADER;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_SHOW_DELETED:
                aProperty.Name = INFO_SHOWDELETEDROWS;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_IS_TABLE_NAME_LENGTH_LIMITED:
                aProperty.Name = INFO_ALLOWLONGTABLENAMES;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_SYSTEM_DRIVER_SETTINGS:
                aProperty.Name = INFO_ADDITIONALOPTIONS;
                break;
            case XML_TOK_ENABLE_SQL92_CHECK:
                aProperty.Name = PROPERTY_ENABLESQL92CHECK;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_APPEND_TABLE_ALIAS_NAME:
                aProperty.Name = INFO_APPEND_TABLE_ALIAS;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_PARAMETER_NAME_SUBSTITUTION:
                aProperty.Name = INFO_PARAMETERNAMESUBST;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_IGNORE_DRIVER_PRIVILEGES:
                aProperty.Name = INFO_IGNOREDRIVER_PRIV;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_BOOLEAN_COMPARISON_MODE:
                aProperty.Name = PROPERTY_BOOLEANCOMPARISONMODE;
                if ( sValue.equalsAscii("equal-integer") )
                    aProperty.Value <<= sal_Int32(0);
                else if ( sValue.equalsAscii("is-boolean") )
                    aProperty.Value <<= sal_Int32(1);
                else if ( sValue.equalsAscii("equal-boolean") )
                    aProperty.Value <<= sal_Int32(2);
                else if ( sValue.equalsAscii("equal-use-only-zero") )
                    aProperty.Value <<= sal_Int32(3);
                break;
            case XML_TOK_USE_CATALOG:
                aProperty.Name = INFO_USECATALOG;
                aProperty.Value <<= (sValue == s_sTRUE ? sal_True : sal_False);
                break;
            case XML_TOK_BASE_DN:
                aProperty.Name = INFO_CONN_LDAP_BASEDN;
                break;
            case XML_TOK_MAX_ROW_COUNT:
                aProperty.Name = INFO_CONN_LDAP_ROWCOUNT;
                aProperty.Value <<= sValue.toInt32();
                break;
        }
        if ( aProperty.Name.getLength() )
        {
            if ( !aProperty.Value.hasValue() )
                aProperty.Value <<= sValue;
            addInfo(aProperty);
        }
    }
}
// -----------------------------------------------------------------------------

OXMLDataSource::~OXMLDataSource()
{
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLDataSource::CreateChildContext(
        sal_uInt16 nPrefix,
        const OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = GetOwnImport().GetDataSourceElemTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_LOGIN:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLLogin( GetOwnImport(), nPrefix, rLocalName,xAttrList );
            break;

        case XML_TOK_TABLE_FILTER:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTableFilterList( GetImport(), nPrefix, rLocalName,sal_True);
            break;
        case XML_TOK_TABLE_TYPE_FILTER:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLTableFilterList( GetImport(), nPrefix, rLocalName,sal_False);
            break;
        case XML_TOK_AUTO_INCREMENT:
        case XML_TOK_DELIMITER:
        case XML_TOK_FONT_CHARSET:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceInfo( GetOwnImport(), nPrefix, rLocalName,xAttrList,*this );
            break;
        case XML_TOK_DATA_SOURCE_SETTINGS:
            GetOwnImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLDataSourceSettings( GetOwnImport(), nPrefix, rLocalName,xAttrList,*this );
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ODBFilter& OXMLDataSource::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLDataSource::EndElement()
{
    Reference<XPropertySet> xDataSource(GetImport().GetModel(),UNO_QUERY);
    if ( !m_aInfoSequence.empty() && xDataSource.is() )
    {
        try
        {
            xDataSource->setPropertyValue(PROPERTY_INFO,makeAny(Sequence<PropertyValue>(m_aInfoSequence.begin(),m_aInfoSequence.size())));
        }
        catch(Exception)
        {
            OSL_ENSURE(0,"Could not set property!");
        }
    }
}

//----------------------------------------------------------------------------
} // namespace dbaxml
// -----------------------------------------------------------------------------
