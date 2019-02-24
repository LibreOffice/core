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

#ifndef INCLUDED_DBACCESS_SOURCE_INC_DSNTYPES_HXX
#define INCLUDED_DBACCESS_SOURCE_INC_DSNTYPES_HXX

#include <sal/config.h>

#include <vector>

#include <dbadllapi.hxx>
#include <connectivity/DriversConfig.hxx>

namespace dbaccess
{

// DATASOURCE_TYPE
/// known datasource types
enum DATASOURCE_TYPE
{
    DST_MSACCESS            =  1,
    DST_MYSQL_ODBC          =  2,
    DST_MYSQL_JDBC          =  3,
    DST_ORACLE_JDBC         =  4,
    //5 was DST_ADABAS
    DST_CALC                =  6,
    DST_DBASE               =  7,
    DST_FLAT                =  8,
    DST_JDBC                =  9,
    DST_ODBC                = 10,
    DST_ADO                 = 11,
    DST_MOZILLA             = 12,
    DST_THUNDERBIRD         = 13,
    DST_LDAP                = 14,
    DST_OUTLOOK             = 15,
    DST_OUTLOOKEXP          = 16,
    DST_EVOLUTION           = 17,
    DST_EVOLUTION_GROUPWISE = 18,
    DST_EVOLUTION_LDAP      = 19,
    DST_KAB                 = 20,
    DST_MACAB               = 21,
    DST_MSACCESS_2007       = 22,
    DST_EMBEDDED_HSQLDB     = 23,
    DST_MYSQL_NATIVE        = 24,
    DST_MYSQL_NATIVE_DIRECT = 25,
    DST_FIREBIRD            = 26,
    DST_EMBEDDED_FIREBIRD   = 27,
    DST_POSTGRES            = 28,
    DST_WRITER              = 29,

    DST_USERDEFINE1,    /// first user defined driver
    DST_USERDEFINE2,
    DST_USERDEFINE3,
    DST_USERDEFINE4,
    DST_USERDEFINE5,
    DST_USERDEFINE6,
    DST_USERDEFINE7,
    DST_USERDEFINE8,
    DST_USERDEFINE9,
    DST_USERDEFINE10,

    DST_UNKNOWN         /// unrecognized type
};

#define PAGE_DBSETUPWIZARD_INTRO                     0
#define PAGE_DBSETUPWIZARD_DBASE                     1
#define PAGE_DBSETUPWIZARD_TEXT                      2
#define PAGE_DBSETUPWIZARD_MSACCESS                  3
#define PAGE_DBSETUPWIZARD_LDAP                      4
//5 was PAGE_DBSETUPWIZARD_ADABAS
#define PAGE_DBSETUPWIZARD_MYSQL_INTRO               6
#define PAGE_DBSETUPWIZARD_MYSQL_JDBC                7
#define PAGE_DBSETUPWIZARD_MYSQL_ODBC                8
#define PAGE_DBSETUPWIZARD_ORACLE                    9
#define PAGE_DBSETUPWIZARD_JDBC                      10
#define PAGE_DBSETUPWIZARD_ADO                       11
#define PAGE_DBSETUPWIZARD_ODBC                      12
#define PAGE_DBSETUPWIZARD_DOCUMENT_OR_SPREADSHEET   13
#define PAGE_DBSETUPWIZARD_AUTHENTIFICATION          14
#define PAGE_DBSETUPWIZARD_FINAL                     16
#define PAGE_DBSETUPWIZARD_USERDEFINED               17
#define PAGE_DBSETUPWIZARD_MYSQL_NATIVE              18

// ODsnTypeCollection
class OOO_DLLPUBLIC_DBA ODsnTypeCollection final
{

    std::vector<OUString> m_aDsnTypesDisplayNames;    /// user readable names for the datasource types
    std::vector<OUString> m_aDsnPrefixes;             /// DSN prefixes which determine the type of a datasource
    ::connectivity::DriversConfig m_aDriverConfig;

#if OSL_DEBUG_LEVEL > 0
    sal_Int32       m_nLivingIterators;         /// just for debugging reasons, counts the living iterators
#endif

public:
    class TypeIterator;
    friend class ODsnTypeCollection::TypeIterator;

    ODsnTypeCollection(const css::uno::Reference< css::uno::XComponentContext >& _xContext);
    ~ODsnTypeCollection();

    /// get the datasource type display name from a DSN string
    OUString getTypeDisplayName(const OUString& _sURL) const;

    /// on a given string, cut the type prefix and return the result
    OUString cutPrefix(const OUString& _sURL) const;

    /// on a given string, return the type prefix
    OUString getPrefix(const OUString& _sURL) const;

    /// determines whether there is a driver for the given URL prefix/pattern
    bool    hasDriver( const sal_Char* _pAsciiPattern ) const;

    /// on a given string, return the Java Driver Class
    OUString getJavaDriverClass(const OUString& _sURL) const;

    /// returns the media type of a file based database
    OUString getMediaType(const OUString& _sURL) const;

    /// returns the dsn prefix for a given media type
    OUString getDatasourcePrefixFromMediaType(const OUString& _sMediaType, const OUString& _sExtension );

    void extractHostNamePort(const OUString& _rDsn,OUString& _sDatabaseName,OUString& _rHostname,sal_Int32& _nPortNumber) const;

    /// check if the given data source allows creation of tables
    bool supportsTableCreation(const OUString& _sURL) const;

    /// check if the given data source allows to show column description.
    bool supportsColumnDescription(const OUString& _sURL) const;

    // check if a Browse button may be shown to insert connection url
    bool supportsBrowsing(const OUString& _sURL) const;

    // check if a Create New Database button may be shown to insert connection url
    bool supportsDBCreation(const OUString& _sURL) const;

    /// check if the given data source type is based on the file system - i.e. the URL is a prefix plus a file URL
    bool isFileSystemBased(const OUString& _sURL) const;

    bool isConnectionUrlRequired(const OUString& _sURL) const;

    /// checks if the given data source type embeds its data into the database document
    static bool isEmbeddedDatabase( const OUString& _sURL );

    static OUString getEmbeddedDatabase();

    // returns true when the properties dialog can be shown, otherwise false.
    static bool isShowPropertiesEnabled( const OUString& _sURL );

    /** returns default settings for newly created databases of the given type.
    */
    css::uno::Sequence< css::beans::PropertyValue>
            getDefaultDBSettings( const OUString& _sURL ) const;

    /// get access to the first element of the types collection
    inline TypeIterator    begin() const;
    /// get access to the (last + 1st) element of the types collection
    inline TypeIterator    end() const;

    void fillPageIds(const OUString& _sURL,std::vector<sal_Int16>& _rOutPathIds) const;

    DATASOURCE_TYPE determineType(const OUString& _rDsn) const;

    sal_Int32 getIndexOf(const OUString& _sURL) const;
    sal_Int32 size() const;
    OUString getType(const OUString& _sURL) const;
};

//- ODsnTypeCollection::TypeIterator
class OOO_DLLPUBLIC_DBA ODsnTypeCollection::TypeIterator
{
    friend class ODsnTypeCollection;

    friend bool OOO_DLLPUBLIC_DBA operator==(const TypeIterator& lhs, const TypeIterator& rhs);
    friend bool OOO_DLLPUBLIC_DBA operator!=(const TypeIterator& lhs, const TypeIterator& rhs) { return !(lhs == rhs); }

protected:
    const ODsnTypeCollection*   m_pContainer;
    sal_Int32                   m_nPosition;

public:
    TypeIterator(const TypeIterator& _rSource);
    ~TypeIterator();

    OUString const & getURLPrefix() const;
    OUString const & getDisplayName() const;

    /// prefix increment
    const TypeIterator& operator++();

protected:
    TypeIterator(const ODsnTypeCollection* _pContainer, sal_Int32 _nInitialPos);
};


inline ODsnTypeCollection::TypeIterator ODsnTypeCollection::begin() const { return ODsnTypeCollection::TypeIterator(this, 0);}
inline ODsnTypeCollection::TypeIterator ODsnTypeCollection::end() const { return ODsnTypeCollection::TypeIterator(this, m_aDsnTypesDisplayNames.size());}

}   // namespace dbaccess

#endif // INCLUDED_DBACCESS_SOURCE_INC_DSNTYPES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
