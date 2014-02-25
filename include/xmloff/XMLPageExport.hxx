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
#include <xmloff/uniref.hxx>
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



class XMLOFF_DLLPUBLIC XMLPageExport : public UniRefBase
{
    SvXMLExport& rExport;

    const OUString sIsPhysical;
    const OUString sFollowStyle;

    ::com::sun::star::uno::Reference<
        ::com::sun::star::container::XNameAccess > xPageStyles;

    ::std::vector< XMLPageExportNameEntry > aNameVector;
    SAL_DLLPRIVATE sal_Bool findPageMasterName( const OUString& rStyleName, OUString& rPMName ) const;

    UniReference < XMLPropertyHandlerFactory > xPageMasterPropHdlFactory;
    UniReference < XMLPropertySetMapper > xPageMasterPropSetMapper;
    UniReference < SvXMLExportPropertyMapper > xPageMasterExportPropMapper;

protected:

    SvXMLExport& GetExport() { return rExport; }

    virtual void collectPageMasterAutoStyle(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                OUString& rPageMasterName );

    virtual void exportMasterPageContent(
                const ::com::sun::star::uno::Reference <
                    ::com::sun::star::beans::XPropertySet > & rPropSet,
                 bool bAutoStyles );

    sal_Bool exportStyle(
                const ::com::sun::star::uno::Reference<
                    ::com::sun::star::style::XStyle >& rStyle,
                sal_Bool bAutoStyles );

    void exportStyles( sal_Bool bUsed, sal_Bool bAutoStyles );

public:
    XMLPageExport( SvXMLExport& rExp );
    ~XMLPageExport();

    void    collectAutoStyles( sal_Bool bUsed )     { exportStyles( bUsed, sal_True ); }
    void    exportAutoStyles();
    void    exportMasterStyles( sal_Bool bUsed )    { exportStyles( bUsed, sal_False ); }

    //text grid enhancement for better CJK support
    void exportDefaultStyle();
};

#endif // INCLUDED_XMLOFF_XMLPAGEEXPORT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
