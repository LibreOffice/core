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

#ifndef INCLUDED_XMLOFF_XMLPAGEEXPORT_HXX
#define INCLUDED_XMLOFF_XMLPAGEEXPORT_HXX

#include <sal/config.h>
#include <xmloff/dllapi.h>
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <vector>
#include <xmloff/attrlist.hxx>
#include <rtl/ref.hxx>
#include <salhelper/simplereferenceobject.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

namespace com { namespace sun { namespace star {
    namespace style { class XStyle; }
    namespace container { class XIndexReplace; class XNameAccess;}
    namespace beans { class XPropertySet; }
} } }

class SvXMLExport;
class XMLPropertyHandlerFactory;
class XMLPropertySetMapper;
class SvXMLExportPropertyMapper;



struct XMLPageExportNameEntry
{
    OUString         sPageMasterName;
    OUString         sStyleName;
};



class XMLOFF_DLLPUBLIC XMLPageExport : public salhelper::SimpleReferenceObject
{
    SvXMLExport& rExport;

    const OUString sIsPhysical;
    const OUString sFollowStyle;

    css::uno::Reference< css::container::XNameAccess > xPageStyles;

    ::std::vector< XMLPageExportNameEntry > aNameVector;
    SAL_DLLPRIVATE bool findPageMasterName( const OUString& rStyleName, OUString& rPMName ) const;

    rtl::Reference < XMLPropertyHandlerFactory > xPageMasterPropHdlFactory;
    rtl::Reference < XMLPropertySetMapper > xPageMasterPropSetMapper;
    rtl::Reference < SvXMLExportPropertyMapper > xPageMasterExportPropMapper;

protected:

    SvXMLExport& GetExport() { return rExport; }

    void collectPageMasterAutoStyle(
                const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
                OUString& rPageMasterName );

    virtual void exportMasterPageContent(
                const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
                 bool bAutoStyles );

    bool exportStyle(
                const css::uno::Reference< css::style::XStyle >& rStyle,
                bool bAutoStyles );

    void exportStyles( bool bUsed, bool bAutoStyles );

public:
    XMLPageExport( SvXMLExport& rExp );
    virtual ~XMLPageExport();

    void    collectAutoStyles( bool bUsed )     { exportStyles( bUsed, true ); }
    void    exportAutoStyles();
    void    exportMasterStyles( bool bUsed )    { exportStyles( bUsed, false ); }

    //text grid enhancement for better CJK support
    void exportDefaultStyle();
};

#endif // INCLUDED_XMLOFF_XMLPAGEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
