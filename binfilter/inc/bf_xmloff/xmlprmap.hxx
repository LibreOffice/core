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

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#define _XMLOFF_PROPERTYSETMAPPER_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/uno/Any.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#include <bf_xmloff/uniref.hxx>

#include <bf_xmloff/maptype.hxx>

#include <bf_xmloff/xmltypes.hxx>

#include <bf_xmloff/prhdlfac.hxx>
namespace binfilter {

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
};

class XMLPropertySetMapper : public UniRefBase
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
    sal_Int32	GetEntryCount() const { return aMapEntries.size(); }

    /** Returns the flags of an entry */
    sal_uInt32 GetEntryFlags( sal_Int32 nIndex ) const
    {
        return aMapEntries[nIndex].nType & ~MID_FLAG_MASK;
    }

    /** Returns the type of an entry */
    sal_uInt32 GetEntryType( sal_Int32 nIndex,
                                   sal_Bool bWithFlags = sal_True ) const
    {
        sal_uInt32 nType = aMapEntries[nIndex].nType;
        if( !bWithFlags )
            nType = nType & MID_FLAG_MASK;
        return nType;
    }

    /** Returns the namespace-key of an entry */
    sal_uInt16 GetEntryNameSpace( sal_Int32 nIndex ) const
    {
        return aMapEntries[nIndex].nXMLNameSpace;
    }

    /** Returns the 'local' XML-name of the entry */
    const ::rtl::OUString& GetEntryXMLName( sal_Int32 nIndex ) const
    {
        return aMapEntries[nIndex].sXMLAttributeName;
    }

    /** Returns the entry API name */
    const ::rtl::OUString& GetEntryAPIName( sal_Int32 nIndex ) const
    {
        return aMapEntries[nIndex].sAPIPropertyName;
    }

    /** returns the entry context id */
    sal_Int16 GetEntryContextId( sal_Int32 nIndex ) const
    {
        return nIndex == -1 ? 0 : aMapEntries[nIndex].nContextId;
    }

    /** Returns the index of an entry with the given XML-name and namespace
        If there is no matching entry the method returns -1 */
    sal_Int32 GetEntryIndex( sal_uInt16 nNamespace,
                                   const ::rtl::OUString& rStrName,
                                   sal_Int32 nStartAt = -1 ) const;

    /** Retrieves a PropertyHandler for that property wich placed at nIndex in the XMLPropertyMapEntry-array */
    const XMLPropertyHandler* GetPropertyHandler( sal_Int32 nIndex ) const
    {
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

}//end of namespace binfilter
#endif // _XMLOFF_PROPERTYSETMAPPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
