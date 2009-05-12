/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dsmeta.hxx,v $
 * $Revision: 1.5.68.2 $
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

#ifndef DBACCESS_DSMETA_HXX
#define DBACCESS_DSMETA_HXX

#include "dsntypes.hxx"

/** === begin UNO includes === **/
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

//........................................................................
namespace dbaui
{
//........................................................................

    //====================================================================
    //= AutheticationMode
    //====================================================================
    enum AuthenticationMode
    {
        AuthNone,
        AuthUserPwd,
        AuthPwd
    };

    //====================================================================
    //= DataSourceMetaData
    //====================================================================
    struct AdvancedSettingsSupport;
    class DataSourceMetaData_Impl;
    /** encapsulates meta data for a data source

        On the long run, this class should a) encapsulate *all* meta data which
        currently is hard coded somewhere in the program logic and b) be initialized
        from the configuration.

        At the moment, the data a) is still hard coded in the, well, code and b)
        contains meta data about the advanced settings only.
    */
    class DataSourceMetaData
    {
    public:
        DataSourceMetaData( ::dbaccess::DATASOURCE_TYPE _eType );
        ~DataSourceMetaData();

        /// returns a struct describing this data source type's support for our known advanced settings
        const AdvancedSettingsSupport&  getAdvancedSettingsSupport() const;

        /// determines whether or not the data source requires authentication
        AuthenticationMode  getAuthentication() const;

        static  AuthenticationMode  getAuthentication( ::dbaccess::DATASOURCE_TYPE _eType );

    private:
        ::boost::shared_ptr< DataSourceMetaData_Impl >  m_pImpl;
    };

    //====================================================================
    //= AdvancedSettingsSupport
    //====================================================================
    /// struct taking flags for the supported advanced settings
    struct AdvancedSettingsSupport
    {
        // auto-generated values
        bool    bGeneratedValues;
        // various settings as found on the "Special Settings" page in the UI
        bool    bUseSQL92NamingConstraints;
        bool    bAppendTableAliasInSelect;
        bool    bUseKeywordAsBeforeAlias;
        bool    bUseBracketedOuterJoinSyntax;
        bool    bIgnoreDriverPrivileges;
        bool    bParameterNameSubstitution;
        bool    bDisplayVersionColumns;
        bool    bUseCatalogInSelect;
        bool    bUseSchemaInSelect;
        bool    bUseIndexDirectionKeyword;
        bool    bUseDOSLineEnds;
        bool    bBooleanComparisonMode;
        bool    bFormsCheckRequiredFields;
        bool    bIgnoreCurrency;
        bool    bAutoIncrementIsPrimaryKey;
        bool    bEscapeDateTime;

        // Note: If you extend this list, you need to adjust the ctor (of course)
        // and (maybe) the implementation of supportsAnySpecialSetting

        AdvancedSettingsSupport()
            :bGeneratedValues               ( true )
            ,bUseSQL92NamingConstraints     ( true )
            ,bAppendTableAliasInSelect      ( true )
            ,bUseKeywordAsBeforeAlias       ( true )
            ,bUseBracketedOuterJoinSyntax   ( true )
            ,bIgnoreDriverPrivileges        ( true )
            ,bParameterNameSubstitution     ( true )
            ,bDisplayVersionColumns         ( true )
            ,bUseCatalogInSelect            ( true )
            ,bUseSchemaInSelect             ( true )
            ,bUseIndexDirectionKeyword      ( true )
            ,bUseDOSLineEnds                ( true )
            ,bBooleanComparisonMode         ( true )
            ,bFormsCheckRequiredFields      ( true )
            ,bIgnoreCurrency                ( false )
            ,bAutoIncrementIsPrimaryKey     ( false )
            ,bEscapeDateTime                ( false )
        {
        }

        /** determines whether there is support for any of the settings found on the "Special Settings"
            UI
        */
        inline bool    supportsAnySpecialSetting() const;
    };

    //--------------------------------------------------------------------
    inline bool AdvancedSettingsSupport::supportsAnySpecialSetting() const
    {
        return  ( bUseSQL92NamingConstraints    == true )
            ||  ( bAppendTableAliasInSelect     == true )
            ||  ( bUseKeywordAsBeforeAlias      == true )
            ||  ( bUseBracketedOuterJoinSyntax  == true )
            ||  ( bIgnoreDriverPrivileges       == true )
            ||  ( bParameterNameSubstitution    == true )
            ||  ( bDisplayVersionColumns        == true )
            ||  ( bUseCatalogInSelect           == true )
            ||  ( bUseSchemaInSelect            == true )
            ||  ( bUseIndexDirectionKeyword     == true )
            ||  ( bUseDOSLineEnds               == true )
            ||  ( bBooleanComparisonMode        == true )
            ||  ( bFormsCheckRequiredFields     == true )
            ||  ( bIgnoreCurrency               == true )
            ||  ( bAutoIncrementIsPrimaryKey    == true )
            ||  ( bEscapeDateTime               == true )
            ;
    }

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_DSMETA_HXX
