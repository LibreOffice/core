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
#ifndef _XMLOFF_XMLIMPPR_HXX
#define _XMLOFF_XMLIMPPR_HXX

#include <tools/solar.h>

#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>

#ifndef __SGI_STL_VECTOR
#include <vector>
#endif

#include <bf_xmloff/uniref.hxx>
namespace rtl { class OUString; }
namespace binfilter {

struct XMLPropertyState;
class XMLPropertySetMapper;


class SvXMLUnitConverter;
class SvXMLNamespaceMap;
class SvXMLImport;

/** This struct is used as an optional parameter to the static
 * _FillPropertySet() methods.
 *
 * It should not be used in any other context.
 */
struct _ContextID_Index_Pair 
{
    sal_Int16 nContextID;
    sal_Int32 nIndex;
};

class SvXMLImportPropertyMapper : public UniRefBase
{
    UniReference< SvXMLImportPropertyMapper> mxNextMapper;

    SvXMLImport& rImport;   // access to error handling

protected:

    UniReference< XMLPropertySetMapper > maPropMapper;

public:

    SvXMLImportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper,
            SvXMLImport& rImport);
    virtual ~SvXMLImportPropertyMapper();

    // Add a ImportPropertyMapper at the end of the import mapper chain.
    // The added mapper MUST not be used outside the Mapper chain any longer,
    // because its PropertyMapper will be replaced.
    void ChainImportMapper(
        const UniReference< SvXMLImportPropertyMapper>& rMapper );

    /** like above, except that the mart is only serached within the range
      *  [nStartIdx, nEndIdx[
      */
    void importXML(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::xml::sax::XAttributeList > xAttrList,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            sal_Int32 nStartIdx, sal_Int32 nEndIdx ) const;

    /** this method is called for every item that has the MID_FLAG_SPECIAL_ITEM_IMPORT flag set */
    virtual sal_Bool handleSpecialItem(
            XMLPropertyState& rProperty,
            ::std::vector< XMLPropertyState >& rProperties,
            const ::rtl::OUString& rValue,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap ) const;

    /** This method is called when all attributes have benn processed. It may be used to remove items that are incomplete */
    virtual void finished(
            ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nStartIndex, sal_Int32 nEndIndex ) const;

    sal_Bool FillPropertySet(
            const ::std::vector< XMLPropertyState >& aProperties,
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const;

    inline const UniReference< XMLPropertySetMapper >&
        getPropertySetMapper() const;

    /** implementation helper for FillPropertySet: fill an XPropertySet.
     * Exceptions will be asserted. */
    static sal_Bool _FillPropertySet(
        const ::std::vector<XMLPropertyState> & rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropSet,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo> & rPropSetInfo,
        const UniReference<XMLPropertySetMapper> & rPropMapper,
        SvXMLImport& rImport,

        // parameter for use by txtstyli.cxx; allows efficient
        // catching the combined characters property
        struct _ContextID_Index_Pair* pSpecialContextIds = NULL );

    /** implementation helper for FillPropertySet: fill an XMultiPropertySet.
     * If unsuccessul, set return value. */
    static sal_Bool _FillMultiPropertySet(
        const ::std::vector<XMLPropertyState> & rProperties,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XMultiPropertySet> & rMultiPropSet,
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySetInfo> & rPropSetInfo,
        const UniReference<XMLPropertySetMapper> & rPropMapper,

        // parameters for use by txtstyli.cxx; allows efficient
        // catching the combined characters property
        struct _ContextID_Index_Pair* pSpecialContextIds = NULL );

};

inline const UniReference< XMLPropertySetMapper >&
    SvXMLImportPropertyMapper::getPropertySetMapper() const
{
    return maPropMapper;
}

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLIMPPR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
