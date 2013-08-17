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

#ifndef DBACCESS_DSMETA_HXX
#define DBACCESS_DSMETA_HXX

#include "dsntypes.hxx"
#include "dsitems.hxx"

#include <boost/shared_ptr.hpp>

namespace dbaui
{

    // AutheticationMode
    enum AuthenticationMode
    {
        AuthNone,
        AuthUserPwd,
        AuthPwd
    };

    // DataSourceMetaData
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
        DataSourceMetaData( const OUString& _sURL );
        ~DataSourceMetaData();

        /// returns a struct describing this data source type's support for our known advanced settings
        const FeatureSet&   getFeatureSet() const;

        /// determines whether or not the data source requires authentication
        static  AuthenticationMode  getAuthentication( const OUString& _sURL );

    private:
        ::boost::shared_ptr< DataSourceMetaData_Impl >  m_pImpl;
    };

    // FeatureSet
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

    inline  bool FeatureSet::supportsGeneratedValues() const
    {
        return has( DSID_AUTORETRIEVEENABLED );
    }

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

} // namespace dbaui

#endif // DBACCESS_DSMETA_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
