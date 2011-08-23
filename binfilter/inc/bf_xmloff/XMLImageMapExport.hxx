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

#ifndef _XMLOFF_XMLIMAGEMAPEXPORT_HXX_
#define _XMLOFF_XMLIMAGEMAPEXPORT_HXX_

#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace beans { class XPropertySet; }
    namespace container { class XIndexContainer; }
} } }
namespace binfilter {
class SvXMLExport;


/**
 * Export an ImageMap as defined by service com.sun.star.image.ImageMap to XML.
 */
class XMLImageMapExport
{
    const ::rtl::OUString sBoundary;
    const ::rtl::OUString sCenter;
    const ::rtl::OUString sDescription;
    const ::rtl::OUString sImageMap;
    const ::rtl::OUString sIsActive;
    const ::rtl::OUString sName;
    const ::rtl::OUString sPolygon;
    const ::rtl::OUString sRadius;
    const ::rtl::OUString sTarget;
    const ::rtl::OUString sURL;

    SvXMLExport& rExport;

    sal_Bool bWhiteSpace;	/// use whitespace between image map elements?

public:
    XMLImageMapExport(SvXMLExport& rExport);

    ~XMLImageMapExport();

    /**
     * Get the ImageMap object from the "ImageMap" property and subsequently 
     * export the map (if present).
     */
    void Export( 
        /// the property set containing the ImageMap property
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
    /**
     * Export an ImageMap (XIndexContainer).
     */
    void Export(
        /// the container containing the image map elements
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XIndexContainer> & rContainer);

protected:

    /**
     * Export a single, named map entry.
     * (as given by com.sun.star.image.ImageMapObject)
     * Calls methods for specific image map entries.
     */
    void ExportMapEntry(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);

    /**
     * Export the specifics of a rectangular image map entry.
     * To be called by ExportMapEntry.
     */
    void ExportRectangle(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);

    /**
     * Export the specifics of a circular image map entry.
     * To be called by ExportMapEntry.
     */
    void ExportCircle(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);

    /**
     * Export the specifics of a polygonal image map entry; 
     * To be called by ExportMapEntry.
     */
    void ExportPolygon(
        const ::com::sun::star::uno::Reference<
            ::com::sun::star::beans::XPropertySet> & rPropertySet);
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
