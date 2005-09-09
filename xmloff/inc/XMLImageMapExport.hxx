/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XMLImageMapExport.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 12:51:47 $
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

#ifndef _XMLOFF_XMLIMAGEMAPEXPORT_HXX_
#define _XMLOFF_XMLIMAGEMAPEXPORT_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace beans { class XPropertySet; }
    namespace container { class XIndexContainer; }
} } }
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

    sal_Bool bWhiteSpace;   /// use whitespace between image map elements?

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

#endif
