/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: datasourceui.cxx,v $
 * $Revision: 1.5.68.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
        case DSID_BOOLEANCOMPARISON:    return rAdvancedSupport.bBooleanComparisonMode;
        case DSID_CHECK_REQUIRED_FIELDS:return rAdvancedSupport.bFormsCheckRequiredFields;
        case DSID_AUTORETRIEVEENABLED:  return rAdvancedSupport.bGeneratedValues;
        case DSID_AUTOINCREMENTVALUE:   return rAdvancedSupport.bGeneratedValues;
        case DSID_AUTORETRIEVEVALUE:    return rAdvancedSupport.bGeneratedValues;
        case DSID_IGNORECURRENCY:       return rAdvancedSupport.bIgnoreCurrency;
        case DSID_ESCAPE_DATETIME:      return rAdvancedSupport.bEscapeDateTime;
        }

        OSL_ENSURE( false, "DataSourceUI::hasSetting: this item id is currently not supported!" );
            // Support for *all* items is a medium-term goal only.
        return false;
    }

//........................................................................
} // namespace dbaui
//........................................................................
