/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _DBACCESS_DSNTYPES_HXX_
#define _DBACCESS_DSNTYPES_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <comphelper/stl_types.hxx>
#include "dbadllapi.hxx"
#include <tools/string.hxx>
#include <connectivity/DriversConfig.hxx>

//.........................................................................
namespace dbaccess
{
//.........................................................................

//=========================================================================
//= DATASOURCE_TYPE
//=========================================================================
/// known datasource types
enum DATASOURCE_TYPE
{
    DST_MSACCESS            =  1,
    DST_MYSQL_ODBC          =  2,
    DST_MYSQL_JDBC          =  3,
    DST_ORACLE_JDBC         =  4,
    DST_ADABAS              =  5,
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
     DST_EMBEDDED_HSQLDB    = 23,
    DST_MYSQL_NATIVE        = 24,
    DST_MYSQL_NATIVE_DIRECT = 25,

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
#define PAGE_DBSETUPWIZARD_ADABAS                    5
#define PAGE_DBSETUPWIZARD_MYSQL_INTRO               6
#define PAGE_DBSETUPWIZARD_MYSQL_JDBC                7
#define PAGE_DBSETUPWIZARD_MYSQL_ODBC                8
#define PAGE_DBSETUPWIZARD_ORACLE                    9
#define PAGE_DBSETUPWIZARD_JDBC                      10
#define PAGE_DBSETUPWIZARD_ADO                       11
#define PAGE_DBSETUPWIZARD_ODBC                      12
#define PAGE_DBSETUPWIZARD_SPREADSHEET               13
#define PAGE_DBSETUPWIZARD_AUTHENTIFICATION          14
#define PAGE_DBSETUPWIZARD_MOZILLA                   15
#define PAGE_DBSETUPWIZARD_FINAL                     16
#define PAGE_DBSETUPWIZARD_USERDEFINED               17
#define PAGE_DBSETUPWIZARD_MYSQL_NATIVE              18


//=========================================================================
//= ODsnTypeCollection
//=========================================================================
class OOO_DLLPUBLIC_DBA ODsnTypeCollection
{
protected:
    DECLARE_STL_VECTOR(String, StringVector);

    StringVector    m_aDsnTypesDisplayNames;    /// user readable names for the datasource types
    StringVector    m_aDsnPrefixes;             /// DSN prefixes which determine the type of a datasource
    ::connectivity::DriversConfig m_aDriverConfig;
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > m_xFactory;

#ifdef DBG_UTIL
    sal_Int32       m_nLivingIterators;         /// just for debugging reasons, counts the living iterators
#endif

public:
    class TypeIterator;
    friend class ODsnTypeCollection::TypeIterator;

    ODsnTypeCollection(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _xFactory);
    ~ODsnTypeCollection();

    /// get the datasource type display name from a DSN string
    String getTypeDisplayName(const ::rtl::OUString& _sURL) const;

    /// on a given string, cut the type prefix and return the result
    String cutPrefix(const ::rtl::OUString& _sURL) const;

    /// on a given string, return the type prefix
    String getPrefix(const ::rtl::OUString& _sURL) const;

    /// determines whether there is a driver for the given URL prefix/pattern
    bool    hasDriver( const sal_Char* _pAsciiPattern ) const;

    /// on a given string, return the Java Driver Class
    String getJavaDriverClass(const ::rtl::OUString& _sURL) const;

    /// returns the media type of a file based database
    String getMediaType(const ::rtl::OUString& _sURL) const;

    /// returns the dsn prefix for a given media type
    String getDatasourcePrefixFromMediaType(const ::rtl::OUString& _sMediaType,const ::rtl::OUString& _sExtension = ::rtl::OUString() );

    void extractHostNamePort(const ::rtl::OUString& _rDsn,String& _sDatabaseName,String& _rHostname,sal_Int32& _nPortNumber) const;

    /// check if the given data source allows creation of tables
    sal_Bool supportsTableCreation(const ::rtl::OUString& _sURL) const;

    /// check if the given data source allows to show column description.
    sal_Bool supportsColumnDescription(const ::rtl::OUString& _sURL) const;

    // check if a Browse button may be shown to insert connection url
    sal_Bool supportsBrowsing(const ::rtl::OUString& _sURL) const;

    /// check if the given data source tyoe is based on the file system - i.e. the URL is a prefix plus a file URL
    sal_Bool isFileSystemBased(const ::rtl::OUString& _sURL) const;

    bool isConnectionUrlRequired(const ::rtl::OUString& _sURL) const;

    /// checks if the given data source type embeds its data into the database document
    bool isEmbeddedDatabase( const ::rtl::OUString& _sURL ) const;

    ::rtl::OUString getEmbeddedDatabase() const;

    // returns true when the properties dialog can be shown, otherwise false.
    bool isShowPropertiesEnabled( const ::rtl::OUString& _sURL ) const;

    /** returns default settings for newly created databases of the given type.
    */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
            getDefaultDBSettings( const ::rtl::OUString& _sURL ) const;

    /// get access to the first element of the types collection
    TypeIterator    begin() const;
    /// get access to the (last + 1st) element of the types collection
    TypeIterator    end() const;

    void fillPageIds(const ::rtl::OUString& _sURL,::std::vector<sal_Int16>& _rOutPathIds) const;

    DATASOURCE_TYPE determineType(const String& _rDsn) const;

    bool needsJVM(const String& _rDsn) const;

    sal_Int32 getIndexOf(const ::rtl::OUString& _sURL) const;
    sal_Int32 size() const;
    ::rtl::OUString getType(const ::rtl::OUString& _sURL) const;
};

//-------------------------------------------------------------------------
//- ODsnTypeCollection::TypeIterator
//-------------------------------------------------------------------------
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

    ::rtl::OUString getURLPrefix() const;
    String          getDisplayName() const;

    /// prefix increment
    const TypeIterator& operator++();
    /// postfix increment
    const TypeIterator  operator++(int) { TypeIterator hold(*this); ++*this; return hold; }

    /// prefix decrement
    const TypeIterator& operator--();
    /// postfix decrement
    const TypeIterator  operator--(int) { TypeIterator hold(*this); --*this; return hold; }

protected:
    TypeIterator(const ODsnTypeCollection* _pContainer, sal_Int32 _nInitialPos = 0);
};

//.........................................................................
}   // namespace dbaccess
//.........................................................................

#endif // _DBACCESS_DSNTYPES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
