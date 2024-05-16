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

#include "text.hxx"

#include <osl/diagnose.h>

using namespace css::uno;
using namespace css::xml::dom;
using namespace css::xml::sax;

namespace DOM
{
    CText::CText(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            NodeType const& reNodeType, xmlNodePtr const& rpNode)
        : CText_Base(rDocument, rMutex, reNodeType, rpNode)
    {
    }

    CText::CText(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNodePtr const pNode)
        : CText_Base(rDocument, rMutex, NodeType_TEXT_NODE, pNode)
    {
    }

    void CText::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        i_xHandler->characters(getData());
    }

    void CText::fastSaxify( Context& io_rContext )
    {
        if (io_rContext.mxCurrentHandler.is())
        {
            try
            {
                io_rContext.mxCurrentHandler->characters( getData() );
            }
            catch( Exception& )
            {}
        }
    }

    OUString SAL_CALL CText::getNodeName()
    {
        return u"#text"_ustr;
    }

    Reference< XText > SAL_CALL CText::splitText(sal_Int32 /*offset*/)
    {
        OSL_FAIL("CText::splitText: not implemented (#i113683#)");
        return Reference< XText >(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
