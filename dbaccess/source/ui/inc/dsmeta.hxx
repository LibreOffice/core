/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "dsitems.hxx"

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
    class FeatureSet;
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
        DataSourceMetaData( const ::rtl::OUString& _sURL );
        ~DataSourceMetaData();

        /// returns a struct describing this data source type's support for our known advanced settings
        const FeatureSet&   getFeatureSet() const;

        /// determines whether or not the data source requires authentication
        static  AuthenticationMode  getAuthentication( const ::rtl::OUString& _sURL );

    private:
        ::boost::shared_ptr< DataSourceMetaData_Impl >  m_pImpl;
    };

    //====================================================================
    //= FeatureSet
    //====================================================================
    /** can be used to ask for (UI) support for certain advanced features
    */
    class FeatureSet
    {
    public:
        typedef ::std::set< ItemID >::const_iterator    const_iterator;

    public:
        inline FeatureSet() { }

        inline void put( const ItemID _id )         { m_aContent.insert( _id ); }
        inline bool has( const ItemID _id ) const   { return m_aContent.find( _id ) != m_aContent.end(); }

        inline  bool    supportsAnySpecialSetting() const;
        inline  bool    supportsGeneratedValues() const;

        inline  const_iterator begin() const    { return m_aContent.begin(); }
        inline  const_iterator end() const      { return m_aContent.end(); }

    private:
        ::std::set< ItemID >    m_aContent;
    };

    //--------------------------------------------------------------------
    inline  bool FeatureSet::supportsGeneratedValues() const
    {
        return has( DSID_AUTORETRIEVEENABLED );
    }

    //--------------------------------------------------------------------
    inline bool FeatureSet::supportsAnySpecialSetting() const
    {
        return  has( DSID_SQL92CHECK )
            ||  has( DSID_APPEND_TABLE_ALIAS )
            ||  has( DSID_AS_BEFORE_CORRNAME )
            ||  has( DSID_ENABLEOUTERJOIN )
            ||  has( DSID_IGNOREDRIVER_PRIV )
            ||  has( DSID_PARAMETERNAMESUBST )
            ||  has( DSID_SUPPRESSVERSIONCL )
            ||  has( DSID_CATALOG )
            ||  has( DSID_SCHEMA )
            ||  has( DSID_INDEXAPPENDIX )
            ||  has( DSID_DOSLINEENDS )
            ||  has( DSID_BOOLEANCOMPARISON )
            ||  has( DSID_CHECK_REQUIRED_FIELDS )
            ||  has( DSID_IGNORECURRENCY )
            ||  has( DSID_ESCAPE_DATETIME )
            ||  has( DSID_PRIMARY_KEY_SUPPORT )
            ||  has( DSID_MAX_ROW_SCAN )
            ||  has( DSID_RESPECTRESULTSETTYPE )
            ;
    }

//........................................................................
} // namespace dbaui
//........................................................................

#endif // DBACCESS_DSMETA_HXX
