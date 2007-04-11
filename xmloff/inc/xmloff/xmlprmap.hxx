/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlprmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:35:22 $
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

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#define _XMLOFF_PROPERTYSETMAPPER_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

#ifndef _XMLOFF_PROPMAPPINGTYPES_HXX
#include <xmloff/maptype.hxx>
#endif

#ifndef _XMLOFF_XMLTYPES_HXX
#include <xmloff/xmltypes.hxx>
#endif

#ifndef _XMLOFF_PROPERTYHANDLERFACTORY_HXX
#include <xmloff/prhdlfac.hxx>
#endif

#include <tools/debug.hxx>

class SvXMLUnitConverter;
class XMLPropertyHandler;

///////////////////////////////////////////////////////////////////////////
//
/** Helper-class for XML-im/export:
    - Holds a pointer to a given array of XMLPropertyMapEntry
    - Provides several methods to access data from this array
    - Holds a Sequence of XML-names (for properties)
    - The filter takes all properties of the XPropertySet which are also
      in the XMLPropertyMapEntry and which are have not a default value
      and put them into a vector of XMLPropertyStae
    - this class knows how to compare, im/export properties

    Attention: At all methods, which get an index as parameter, there is no
               range validation to save runtime !!
*/
struct XMLPropertySetMapperEntry_Impl
{
    ::rtl::OUString sXMLAttributeName;
    ::rtl::OUString sAPIPropertyName;
    sal_uInt16 nXMLNameSpace;
    sal_Int32  nType;
    sal_Int16  nContextId;
    const XMLPropertyHandler *pHdl;

    XMLPropertySetMapperEntry_Impl(
        const XMLPropertyMapEntry& rMapEntry,
        const UniReference< XMLPropertyHandlerFactory >& rFactory );

    XMLPropertySetMapperEntry_Impl(
        const XMLPropertySetMapperEntry_Impl& rEntry );

    sal_uInt32 GetPropType() const { return nType & XML_TYPE_PROP_MASK; }
};

class XMLOFF_DLLPUBLIC XMLPropertySetMapper : public UniRefBase
{
    ::std::vector< XMLPropertySetMapperEntry_Impl > aMapEntries;
    ::std::vector< UniReference < XMLPropertyHandlerFactory > > aHdlFactories;

public:
    /** The last element of the XMLPropertyMapEntry-array must contain NULL-values */
    XMLPropertySetMapper(
            const XMLPropertyMapEntry* pEntries,
            const UniReference< XMLPropertyHandlerFactory >& rFactory );
    virtual ~XMLPropertySetMapper();

    void AddMapperEntry( const UniReference < XMLPropertySetMapper >& rMapper );

    /** Return number of entries in input-array */
    const sal_Int32 GetEntryCount() const { return aMapEntries.size(); }

    /** Returns the flags of an entry */
    const sal_uInt32 GetEntryFlags( sal_Int32 nIndex ) const
    {
        DBG_ASSERT( (nIndex >= 0) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        return aMapEntries[nIndex].nType & ~MID_FLAG_MASK;
    }

    /** Returns the type of an entry */
    const sal_uInt32 GetEntryType( sal_Int32 nIndex,
                                   sal_Bool bWithFlags = sal_True ) const
    {
        DBG_ASSERT( (nIndex >= 0) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        sal_uInt32 nType = aMapEntries[nIndex].nType;
        if( !bWithFlags )
            nType = nType & MID_FLAG_MASK;
        return nType;
    }

    /** Returns the namespace-key of an entry */
    const sal_uInt16 GetEntryNameSpace( sal_Int32 nIndex ) const
    {
        DBG_ASSERT( (nIndex >= 0) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        return aMapEntries[nIndex].nXMLNameSpace;
    }

    /** Returns the 'local' XML-name of the entry */
    const ::rtl::OUString& GetEntryXMLName( sal_Int32 nIndex ) const
    {
        DBG_ASSERT( (nIndex >= 0) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        return aMapEntries[nIndex].sXMLAttributeName;
    }

    /** Returns the entry API name */
    const ::rtl::OUString& GetEntryAPIName( sal_Int32 nIndex ) const
    {
        DBG_ASSERT( (nIndex >= 0) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        return aMapEntries[nIndex].sAPIPropertyName;
    }

    /** returns the entry context id. -1 is a valid index here. */
    const sal_Int16 GetEntryContextId( sal_Int32 nIndex ) const
    {
        DBG_ASSERT( (nIndex >= -1) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        return nIndex == -1 ? 0 : aMapEntries[nIndex].nContextId;
    }

    /** Returns the index of an entry with the given XML-name and namespace
        If there is no matching entry the method returns -1 */
    const sal_Int32 GetEntryIndex( sal_uInt16 nNamespace,
                                   const ::rtl::OUString& rStrName,
                                   sal_uInt32 nPropType,
                                   sal_Int32 nStartAt = -1 ) const;

    /** Retrieves a PropertyHandler for that property wich placed at nIndex in the XMLPropertyMapEntry-array */
    const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nIndex ) const
    {
        DBG_ASSERT( (nIndex >= 0) && (nIndex < (sal_Int32)aMapEntries.size() ), "illegal access to invalid entry!" );
        return aMapEntries[nIndex].pHdl;
    }

    /** import/export
        This methods calls the respective im/export-method of the respective PropertyHandler. */
    virtual sal_Bool exportXML( ::rtl::OUString& rStrExpValue,
                                const XMLPropertyState& rProperty,
                                const SvXMLUnitConverter& rUnitConverter ) const;
    virtual sal_Bool importXML( const ::rtl::OUString& rStrImpValue,
                                XMLPropertyState& rProperty,
                                const SvXMLUnitConverter& rUnitConverter ) const;

    /** searches for an entry that matches the given api name, namespace and local name or -1 if nothing found */
    sal_Int32 FindEntryIndex( const sal_Char* sApiName,
                              sal_uInt16 nNameSpace,
                              const ::rtl::OUString& sXMLName ) const;

    /** searches for an entry that matches the given ContextId or gives -1 if nothing found */
    sal_Int32 FindEntryIndex( const sal_Int16 nContextId ) const;
};

#endif // _XMLOFF_PROPERTYSETMAPPER_HXX
