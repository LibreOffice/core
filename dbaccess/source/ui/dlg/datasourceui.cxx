/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: datasourceui.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-27 12:11:03 $
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

#include "datasourceui.hxx"
#include "dsmeta.hxx"
#include "dsitems.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace dbaui
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    /** === end UNO using === **/

    //====================================================================
    //= DataSourceUI
    //====================================================================
    //--------------------------------------------------------------------
    DataSourceUI::DataSourceUI( DATASOURCE_TYPE _eType )
        :m_aDSMeta( DataSourceMetaData( _eType ) )
    {
    }

    //--------------------------------------------------------------------
    DataSourceUI::DataSourceUI( const DataSourceMetaData& _rDSMeta )
        :m_aDSMeta( _rDSMeta )
    {
    }

    //--------------------------------------------------------------------
    DataSourceUI::~DataSourceUI()
    {
    }

    //--------------------------------------------------------------------
    bool DataSourceUI::hasSetting( const USHORT _nItemId ) const
    {
        const AdvancedSettingsSupport& rAdvancedSupport( m_aDSMeta.getAdvancedSettingsSupport() );

        switch ( _nItemId )
        {
        case DSID_SQL92CHECK:           return rAdvancedSupport.bUseSQL92NamingConstraints;
        case DSID_APPEND_TABLE_ALIAS:   return rAdvancedSupport.bAppendTableAliasInSelect;
        case DSID_AS_BEFORE_CORRNAME:   return rAdvancedSupport.bUseKeywordAsBeforeAlias;
        case DSID_ENABLEOUTERJOIN:      return rAdvancedSupport.bUseBracketedOuterJoinSyntax;
        case DSID_IGNOREDRIVER_PRIV:    return rAdvancedSupport.bIgnoreDriverPrivileges;
        case DSID_PARAMETERNAMESUBST:   return rAdvancedSupport.bParameterNameSubstitution;
        case DSID_SUPPRESSVERSIONCL:    return rAdvancedSupport.bDisplayVersionColumns;
        case DSID_CATALOG:              return rAdvancedSupport.bUseCatalogInSelect;
        case DSID_SCHEMA:               return rAdvancedSupport.bUseSchemaInSelect;
        case DSID_INDEXAPPENDIX:        return rAdvancedSupport.bUseIndexDirectionKeyword;
        case DSID_DOSLINEENDS:          return rAdvancedSupport.bUseDOSLineEnds;
        case DSID_CHECK_REQUIRED_FIELDS:return rAdvancedSupport.bFormsCheckRequiredFields;
        case DSID_AUTORETRIEVEENABLED:  return rAdvancedSupport.bGeneratedValues;
        case DSID_AUTOINCREMENTVALUE:   return rAdvancedSupport.bGeneratedValues;
        case DSID_AUTORETRIEVEVALUE:    return rAdvancedSupport.bGeneratedValues;
        case DSID_IGNORECURRENCY:       return rAdvancedSupport.bIgnoreCurrency;
        }

        OSL_ENSURE( false, "DataSourceUI::hasSetting: this item id is currently not supported!" );
            // Support for *all* items is a medium-term goal only.
        return false;
    }

//........................................................................
} // namespace dbaui
//........................................................................
