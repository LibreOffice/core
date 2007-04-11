/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlexppr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:32:22 $
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

#ifndef _XMLOFF_XMLEXPPR_HXX
#define _XMLOFF_XMLEXPPR_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _XMLOFF_PROPERTYSETMAPPER_HXX
#include <xmloff/xmlprmap.hxx>
#endif
#ifndef _UNIVERSALL_REFERENCE_HXX
#include <xmloff/uniref.hxx>
#endif

namespace rtl { class OUString; }

class SvXMLUnitConverter;
class SvXMLAttributeList;
class SvXMLNamespaceMap;
class SvUShorts;
class FilterPropertiesInfos_Impl;
class SvXMLExport;

#define XML_EXPORT_FLAG_DEFAULTS    0x0001      // export also default items
#define XML_EXPORT_FLAG_DEEP        0x0002      // export also items from
                                                // parent item sets
#define XML_EXPORT_FLAG_EMPTY       0x0004      // export attribs element
                                                // even if its empty
#define XML_EXPORT_FLAG_IGN_WS      0x0008

class XMLOFF_DLLPUBLIC SvXMLExportPropertyMapper : public UniRefBase
{
    UniReference< SvXMLExportPropertyMapper> mxNextMapper;

    FilterPropertiesInfos_Impl *pCache;

protected:
    UniReference< XMLPropertySetMapper > maPropMapper;

    /** Filter all properties we don't want to export:
        Take all properties of the XPropertySet which are also found in the
        XMLPropertyMapEntry-array and which are not set directly (so, the value isn't
        default and isn't inherited, apart from bDefault is true)
        After this process It'll called 'Contextfilter' for application-specific
        filter-processes. */
    ::std::vector< XMLPropertyState > _Filter(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet,
            const sal_Bool bDefault ) const;

    /** Application-specific filter. By default do nothing. */
    virtual void ContextFilter(
            ::std::vector< XMLPropertyState >& rProperties,
            ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const;

    /** fills the given attribute list with the items in the given set */
    void _exportXML( sal_uInt16 nPropType, sal_uInt16& rPropTypeFlags,
                     SvXMLAttributeList& rAttrList,
                     const ::std::vector< XMLPropertyState >& rProperties,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     sal_uInt16 nFlags,
                     SvUShorts* pIndexArray,
                       sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx ) const;

    void _exportXML( SvXMLAttributeList& rAttrList,
                     const XMLPropertyState& rProperty,
                     const SvXMLUnitConverter& rUnitConverter,
                     const SvXMLNamespaceMap& rNamespaceMap,
                     sal_uInt16 nFlags,
                     const ::std::vector< XMLPropertyState > *pProperties = 0,
                     sal_uInt32 nIdx = 0 ) const;

    void exportElementItems(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_uInt16 nFlags,
            const SvUShorts& rIndexArray ) const;

public:

    SvXMLExportPropertyMapper(
            const UniReference< XMLPropertySetMapper >& rMapper );
    virtual ~SvXMLExportPropertyMapper();

    // Add a ExportPropertyMapper at the end of the import mapper chain.
    // The added mapper MUST not be used outside the Mapper chain any longer,
    // because its PropertyMapper will be replaced.
    void ChainExportMapper(
        const UniReference< SvXMLExportPropertyMapper>& rMapper );

    /** Filter all properties we don't want to export:
        Take all properties of the XPropertySet which are also found in the
        XMLPropertyMapEntry-array and which are not set directly (so, the value isn't
        default and isn't inherited)
        After this process It'll called 'Contextfilter' for application-specific
        filter-processes. */
    ::std::vector< XMLPropertyState > Filter(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const
                    { return _Filter(rPropSet, sal_False); }

    /** Like Filter(), excepti that:
      * - only properties that have the map flag MID_FLAG_DEFAULT_ITEM_EXPORT
      *   set are exported,
      * - instead of the property's value, its defualt value is exported.
      */
    ::std::vector< XMLPropertyState > FilterDefaults(
            const ::com::sun::star::uno::Reference<
                    ::com::sun::star::beans::XPropertySet > rPropSet ) const
                    { return _Filter(rPropSet, sal_True); }

    /** Compare to arrays of XMLPropertyState */
    sal_Bool Equals( const ::std::vector< XMLPropertyState >& aProperties1,
                     const ::std::vector< XMLPropertyState >& aProperties2 ) const;

    /** fills the given attribute list with the items in the given set */
    void exportXML(
           SvXMLAttributeList& rAttrList,
           const ::std::vector< XMLPropertyState >& rProperties,
           const SvXMLUnitConverter& rUnitConverter,
           const SvXMLNamespaceMap& rNamespaceMap,
           sal_uInt16 nFlags = 0 ) const;
    /** like above but only properties whose property map index is within the
        specified range are exported */
    void exportXML(
           SvXMLAttributeList& rAttrList,
           const ::std::vector< XMLPropertyState >& rProperties,
           const SvXMLUnitConverter& rUnitConverter,
           const SvXMLNamespaceMap& rNamespaceMap,
           sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
           sal_uInt16 nFlags = 0 ) const;

    /** fills the given attribute list with the representation of one item */
    void exportXML(
           SvXMLAttributeList& rAttrList,
           const XMLPropertyState& rProperty,
           const SvXMLUnitConverter& rUnitConverter,
           const SvXMLNamespaceMap& rNamespaceMap,
           sal_uInt16 nFlags = 0 ) const;

    void exportXML(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_uInt16 nFlags = 0 ) const;

    /** like above but only properties whose property map index is within the
        specified range are exported */
    void exportXML(
            SvXMLExport& rExport,
            const ::std::vector< XMLPropertyState >& rProperties,
            sal_Int32 nPropMapStartIdx, sal_Int32 nPropMapEndIdx,
            sal_uInt16 nFlags = 0 ) const;

    /** this method is called for every item that has the
        MID_FLAG_ELEMENT_EXPORT flag set */
    virtual void handleElementItem(
            SvXMLExport& rExport,
            const XMLPropertyState& rProperty,
            sal_uInt16 nFlags,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;

    /** this method is called for every item that has the
        MID_FLAG_SPECIAL_ITEM_EXPORT flag set */
    virtual void handleSpecialItem(
            SvXMLAttributeList& rAttrList,
            const XMLPropertyState& rProperty,
            const SvXMLUnitConverter& rUnitConverter,
            const SvXMLNamespaceMap& rNamespaceMap,
            const ::std::vector< XMLPropertyState > *pProperties = 0,
            sal_uInt32 nIdx = 0 ) const;

    inline const UniReference< XMLPropertySetMapper >&
        getPropertySetMapper() const { return maPropMapper; }

};

#endif  //  _XMLOFF_XMLEXPPR_HXX
