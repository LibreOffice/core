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
#include "parsenode_s.hxx"
#include <connectivity/sqlnode.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= OSimpleParseNode
    //================================================================
    //----------------------------------------------------------------
    OSimpleParseNode::OSimpleParseNode(const OSQLParseNode* _pNode, sal_Bool _bTakeOwnership)
        :m_pFullNode(_pNode)
        ,m_bOwner(_bTakeOwnership)
    {
        OSL_ENSURE(m_pFullNode, "OSimpleParseNode::OSimpleParseNode: invalid node given!");
    }

    //----------------------------------------------------------------
    OSimpleParseNode::~OSimpleParseNode()
    {
        if (m_bOwner)
            delete m_pFullNode;
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleParseNode::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleParseNode::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToStr(OUString& _rString, const Reference< XConnection >& _rxConnection,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToStr( _rString, _rxConnection, _pContext );
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToPredicateStr(OUString& _rString, const Reference< XConnection >& _rxConnection,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField,
        const Locale& _rIntl, const sal_Char _cDecSeparator,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToPredicateStr( _rString, _rxConnection, _rxFormatter, _rxField, _rIntl, _cDecSeparator, _pContext );
    }

//........................................................................
}   // namespace connectivity
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
