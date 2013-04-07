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

#ifndef _XMLTEXTE_HXX
#define _XMLTEXTE_HXX

#include <xmloff/txtparae.hxx>
#include <tools/globname.hxx>

class SwXMLExport;
class SvXMLAutoStylePoolP;
class SwNoTxtNode;

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

    SwNoTxtNode *GetNoTxtNode(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet ) const;

protected:
    virtual void exportStyleContent(
            const ::com::sun::star::uno::Reference<
                ::com::sun::star::style::XStyle > & rStyle );

    virtual void _collectTextEmbeddedAutoStyles(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet );
    virtual void _exportTextEmbedded(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySet > & rPropSet,
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::beans::XPropertySetInfo > & rPropSetInfo );

    virtual void exportTable(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContent,
        sal_Bool bAutoStyles, sal_Bool bProgress );

public:
    SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& rAutoStylePool );
    ~SwXMLTextParagraphExport();

    virtual void setTextEmbeddedGraphicURL(
        const ::com::sun::star::uno::Reference <
                ::com::sun::star::beans::XPropertySet >& rPropSet,
        OUString& rStreamName ) const;
};


#endif  //  _XMLTEXTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
