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

#include "documentfragment.hxx"

using namespace css::uno;
using namespace css::xml::dom;

namespace DOM
{
    CDocumentFragment::CDocumentFragment(
            CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNodePtr const pNode)
        : CDocumentFragment_Base(rDocument, rMutex,
                NodeType_DOCUMENT_FRAGMENT_NODE, pNode)
    {
    }

    bool CDocumentFragment::IsChildTypeAllowed(NodeType const nodeType, NodeType const*const)
    {
        switch (nodeType) {
            case NodeType_ELEMENT_NODE:
            case NodeType_PROCESSING_INSTRUCTION_NODE:
            case NodeType_COMMENT_NODE:
            case NodeType_TEXT_NODE:
            case NodeType_CDATA_SECTION_NODE:
            case NodeType_ENTITY_REFERENCE_NODE:
                return true;
            default:
                return false;
        }
    }

    OUString SAL_CALL CDocumentFragment::getNodeName()
    {
        return u"#document-fragment"_ustr;
    }
    OUString SAL_CALL CDocumentFragment::getNodeValue()
    {
        return OUString();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
