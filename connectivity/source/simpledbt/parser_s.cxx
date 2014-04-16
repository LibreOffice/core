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

#include <connectivity/virtualdbtools.hxx>
#include "parser_s.hxx"
#include "parsenode_s.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;


namespace connectivity
{



    //= OSimpleSQLParser


    OSimpleSQLParser::OSimpleSQLParser(const Reference< XComponentContext >& rxContext, const IParseContext* _pContext)
        :m_aFullParser(rxContext, _pContext)
    {
    }


    oslInterlockedCount SAL_CALL OSimpleSQLParser::acquire()
    {
        return ORefBase::acquire();
    }


    oslInterlockedCount SAL_CALL OSimpleSQLParser::release()
    {
        return ORefBase::release();
    }


    const IParseContext& OSimpleSQLParser::getContext() const
    {
        return m_aFullParser.getContext();
    }


    ::rtl::Reference< simple::ISQLParseNode > OSimpleSQLParser::predicateTree(OUString& rErrorMessage, const OUString& rStatement,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField) const
    {
        OSimpleParseNode* pReturn = NULL;
        OSQLParseNode* pFullNode = const_cast<OSimpleSQLParser*>(this)->m_aFullParser.predicateTree(rErrorMessage, rStatement, _rxFormatter, _rxField);
        if (pFullNode)
            pReturn = new OSimpleParseNode(pFullNode, true);
        return pReturn;
    }


}   // namespace connectivity



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
