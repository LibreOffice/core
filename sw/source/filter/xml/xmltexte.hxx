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

#ifndef INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTE_HXX
#define INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTE_HXX

#include <xmloff/txtparae.hxx>
#include <tools/globname.hxx>

class SwXMLExport;
class SvXMLAutoStylePoolP;
class SwNoTextNode;

namespace com { namespace sun { namespace star { namespace style {
                class XStyle; } } } }

class SwXMLTextParagraphExport : public XMLTextParagraphExport
{
    const OUString sTextTable;
    const OUString sEmbeddedObjectProtocol;
    const OUString sGraphicObjectProtocol;

    const SvGlobalName aAppletClassId;
    const SvGlobalName aPluginClassId;
    const SvGlobalName aIFrameClassId;
    const SvGlobalName aOutplaceClassId;

    static SwNoTextNode *GetNoTextNode(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet );

protected:
    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle ) SAL_OVERRIDE;

    virtual void _collectTextEmbeddedAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet ) SAL_OVERRIDE;
    virtual void _exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo ) SAL_OVERRIDE;

    virtual void exportTable(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        bool bAutoStyles, bool bProgress ) SAL_OVERRIDE;

public:
    SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& rAutoStylePool );
    virtual ~SwXMLTextParagraphExport();
};

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
