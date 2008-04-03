/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsntypes.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 16:49:33 $
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

#ifndef _DBAUI_DSNTYPES_HXX_
#define _DBAUI_DSNTYPES_HXX_

#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <svtools/poolitem.hxx>

#include <comphelper/stl_types.hxx>
#include "dbaccessdllapi.h"

//.........................................................................
namespace dbaui
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

//=========================================================================
//= ODsnTypeCollection
//=========================================================================
class DBACCESS_DLLPUBLIC ODsnTypeCollection
{
protected:
    DECLARE_STL_VECTOR(String, StringVector);
    DECLARE_STL_VECTOR(DATASOURCE_TYPE, TypeVector);
    typedef ::std::map<DATASOURCE_TYPE,DATASOURCE_TYPE> TRelatedTypes;

    StringVector    m_aDsnTypesDisplayNames;    /// user readable names for the datasource types
    StringVector    m_aDsnPrefixes;             /// DSN prefixes which determine the type of a datasource
    TypeVector      m_aDsnTypes;                /// types of datasources we know
    StringVector    m_aUserExtensions;          /// extensions of user defined types

#ifdef DBG_UTIL
    sal_Int32       m_nLivingIterators;         /// just for debugging reasons, counts the living iterators
#endif

public:
    class TypeIterator;
    friend class ODsnTypeCollection::TypeIterator;

    ODsnTypeCollection();
    ~ODsnTypeCollection();

    /// get the datasource type from a DSN string
    DATASOURCE_TYPE getType(const String& _rDsn) const;

    /// get the datasource type display name from a DSN string
    String getTypeDisplayName(DATASOURCE_TYPE _eType) const;

    /// the the DSN prefix associated with a given type
    String getDatasourcePrefix(DATASOURCE_TYPE _eType) const;

    /// returns the extension of the user defined type
    String getTypeExtension(DATASOURCE_TYPE _eType) const;

    /// on a given string, cut the type prefix and return the result
    String cutPrefix(const String& _rDsn) const;

    /// returns the media type of a file based database
    String getMediaType(DATASOURCE_TYPE _eType) const;

    /// returns the dsn prefix for a given media type
    static String getDatasourcePrefixFromMediaType(const String& _sMediaType,const String& _sExtension = String() );

    void extractHostNamePort(const String& _rDsn,String& _sDatabaseName,String& _rHostname,sal_Int32& _nPortNumber) const;

    /// check if the given data source allows creation of tables
    sal_Bool supportsTableCreation(DATASOURCE_TYPE _eType);

    // check if a Browse button may be shown to insert connection url
    sal_Bool supportsBrowsing(DATASOURCE_TYPE _eType);

    /// check if the given data source tyoe is based on the file system - i.e. the URL is a prefix plus a file URL
    sal_Bool isFileSystemBased(DATASOURCE_TYPE _eType) const;

    /** returns default settings for newly created databases of the given type.

        Currently implemented (and used) for DST_EMBEDDED_HSQLDB only
    */
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>
            getDefaultDBSettings( DATASOURCE_TYPE _eType ) const;

    /// get access to the first element of the types collection
    TypeIterator    begin() const;
    /// get access to the (last + 1st) element of the types collection
    TypeIterator    end() const;

    /** read all user defined driver types.
        @param  _rxORB
            The service factory
    */
    void initUserDriverTypes(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB);

protected:
    /// return the connection type a DSN string represents
    DATASOURCE_TYPE implDetermineType(const String& _rDsn) const;

    /// return the index within the internal structures for the connection type given
    sal_Int32 implDetermineTypeIndex(DATASOURCE_TYPE _eType) const;
};

//-------------------------------------------------------------------------
//- ODsnTypeCollection::TypeIterator
//-------------------------------------------------------------------------
class ODsnTypeCollection::TypeIterator
{
    friend class ODsnTypeCollection;

    friend bool operator==(const TypeIterator& lhs, const TypeIterator& rhs);
    friend bool operator!=(const TypeIterator& lhs, const TypeIterator& rhs) { return !(lhs == rhs); }

protected:
    const ODsnTypeCollection*   m_pContainer;
    sal_Int32                   m_nPosition;

public:
    TypeIterator(const TypeIterator& _rSource);
    ~TypeIterator();

    DATASOURCE_TYPE getType() const;
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

//=========================================================================
//= DbuTypeCollectionItem
//=========================================================================
/** allows an ODsnTypeCollection to be transported in an SfxItemSet
*/
class DbuTypeCollectionItem : public SfxPoolItem
{
    ODsnTypeCollection* m_pCollection;

public:
    TYPEINFO();
    DbuTypeCollectionItem(sal_Int16 nWhich = 0, ODsnTypeCollection* _pCollection = NULL);
    DbuTypeCollectionItem(const DbuTypeCollectionItem& _rSource);

    virtual int              operator==(const SfxPoolItem& _rItem) const;
    virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const;

    ODsnTypeCollection* getCollection() const { return m_pCollection; }
};


//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DSNTYPES_HXX_

