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

#ifndef INCLUDED_XMLOFF_INC_XMLIMAGEMAPEXPORT_HXX
#define INCLUDED_XMLOFF_INC_XMLIMAGEMAPEXPORT_HXX

#include <rtl/ustring.hxx>

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
    const OUString msBoundary;
    const OUString msCenter;
    const OUString msDescription;
    const OUString msImageMap;
    const OUString msIsActive;
    const OUString msName;
    const OUString msPolygon;
    const OUString msRadius;
    const OUString msTarget;
    const OUString msURL;
    const OUString msTitle;

    SvXMLExport& mrExport;

public:
    XMLImageMapExport(SvXMLExport& rExport);

    ~XMLImageMapExport();

    /**
     * Get the ImageMap object from the "ImageMap" property and subsequently
     * export the map (if present).
     */
    void Export(
        /// the property set containing the ImageMap property
        const css::uno::Reference< css::beans::XPropertySet> & rPropertySet);
    /**
     * Export an ImageMap (XIndexContainer).
     */
    void Export(
        /// the container containing the image map elements
        const css::uno::Reference< css::container::XIndexContainer> & rContainer);

private:

    /**
     * Export a single, named map entry.
     * (as given by com.sun.star.image.ImageMapObject)
     * Calls methods for specific image map entries.
     */
    void ExportMapEntry(
        const css::uno::Reference< css::beans::XPropertySet> & rPropertySet);

    /**
     * Export the specifics of a rectangular image map entry.
     * To be called by ExportMapEntry.
     */
    void ExportRectangle(
        const css::uno::Reference< css::beans::XPropertySet> & rPropertySet);

    /**
     * Export the specifics of a circular image map entry.
     * To be called by ExportMapEntry.
     */
    void ExportCircle(
        const css::uno::Reference< css::beans::XPropertySet> & rPropertySet);

    /**
     * Export the specifics of a polygonal image map entry;
     * To be called by ExportMapEntry.
     */
    void ExportPolygon(
        const css::uno::Reference< css::beans::XPropertySet> & rPropertySet);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
