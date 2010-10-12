/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
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
    void OSimpleParseNode::parseNodeToStr(::rtl::OUString& _rString, const Reference< XConnection >& _rxConnection,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToStr( _rString, _rxConnection, _pContext );
    }

    //----------------------------------------------------------------
    void OSimpleParseNode::parseNodeToPredicateStr(::rtl::OUString& _rString, const Reference< XConnection >& _rxConnection,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField,
        const Locale& _rIntl, const sal_Char _cDecSeparator,const IParseContext* _pContext) const
    {
        m_pFullNode->parseNodeToPredicateStr( _rString, _rxConnection, _rxFormatter, _rxField, _rIntl, _cDecSeparator, _pContext );
    }

//........................................................................
}   // namespace connectivity
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
