/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsmeta.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 17:03:01 $
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
        DataSourceMetaData( DATASOURCE_TYPE _eType );
        ~DataSourceMetaData();

        /// returns a struct describing this data source type's support for our known advanced settings
        const AdvancedSettingsSupport&  getAdvancedSettingsSupport() const;

        /// determines whether or not the data source requires authentication
        AuthenticationMode  getAuthentication() const;

        static  AuthenticationMode  getAuthentication( DATASOURCE_TYPE _eType );

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
            ||  ( bIgnoreCurrency               == true );
    }

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_DSMETA_HXX
