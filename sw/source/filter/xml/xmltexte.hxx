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

#include <optional>
#include <unordered_map>

#define XML_EMBEDDEDOBJECTGRAPHIC_URL_BASE "vnd.sun.star.GraphicObject:"

class SwXMLExport;
class SvXMLAutoStylePoolP;
class SwNoTextNode;
class SwTableNode;

class SwTableLines;
namespace com { namespace sun { namespace star { namespace style {
                class XStyle; } } } }

class SwXMLTextParagraphExport : public XMLTextParagraphExport
{
    const SvGlobalName aAppletClassId;
    const SvGlobalName aPluginClassId;
    const SvGlobalName aIFrameClassId;

    // Collected autostyles for use in exportTextAutoStyles
    std::vector<const SwTableNode*> maTableNodes;
public:
    typedef ::std::unordered_map<SwFrameFormat const*, ::std::optional<OUString>> FormatMap;
private:
    ::std::unordered_map<SwTableNode const*, ::std::pair<FormatMap, FormatMap>> m_TableFormats;

    static SwNoTextNode *GetNoTextNode(
        const css::uno::Reference < css::beans::XPropertySet >& rPropSet );

    void CollectTableLinesAutoStyles(const SwTableLines& rLines, SwFrameFormat& rFormat,
                                     bool bProgress);

protected:
    virtual void _collectTextEmbeddedAutoStyles(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet ) override;
    virtual void _exportTextEmbedded(
        const css::uno::Reference< css::beans::XPropertySet > & rPropSet,
        const css::uno::Reference< css::beans::XPropertySetInfo > & rPropSetInfo ) override;

    virtual void exportTable(
        const css::uno::Reference< css::text::XTextContent > & rTextContent,
        bool bAutoStyles, bool bProgress ) override;

    virtual void exportTableAutoStyles() override;

public:
    SwXMLTextParagraphExport(
        SwXMLExport& rExp,
         SvXMLAutoStylePoolP& rAutoStylePool );
    virtual ~SwXMLTextParagraphExport() override;

    ::std::unordered_map<SwTableNode const*, ::std::pair<FormatMap, FormatMap>> const&
        GetTableFormats() const { return m_TableFormats; }
    ::std::unordered_map<SwTableNode const*, ::std::pair<FormatMap, FormatMap>> &
        GetTableFormats()       { return m_TableFormats; }
};

#endif // INCLUDED_SW_SOURCE_FILTER_XML_XMLTEXTE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
