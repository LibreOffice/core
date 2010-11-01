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
#include "parser_s.hxx"
#include "parsenode_s.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;

//........................................................................
namespace connectivity
{
//........................................................................

    //================================================================
    //= OSimpleSQLParser
    //================================================================
    //----------------------------------------------------------------
    OSimpleSQLParser::OSimpleSQLParser(const Reference< XMultiServiceFactory >& _rxServiceFactory,const IParseContext* _pContext)
        :m_aFullParser(_rxServiceFactory,_pContext)
    {
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleSQLParser::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL OSimpleSQLParser::release()
    {
        return ORefBase::release();
    }

    //----------------------------------------------------------------
    const IParseContext& OSimpleSQLParser::getContext() const
    {
        return m_aFullParser.getContext();
    }

    //----------------------------------------------------------------
    ::rtl::Reference< simple::ISQLParseNode > OSimpleSQLParser::predicateTree(::rtl::OUString& rErrorMessage, const ::rtl::OUString& rStatement,
        const Reference< XNumberFormatter >& _rxFormatter, const Reference< XPropertySet >& _rxField) const
    {
        OSimpleParseNode* pReturn = NULL;
        OSQLParseNode* pFullNode = const_cast<OSimpleSQLParser*>(this)->m_aFullParser.predicateTree(rErrorMessage, rStatement, _rxFormatter, _rxField);
        if (pFullNode)
            pReturn = new OSimpleParseNode(pFullNode, sal_True);
        return pReturn;
    }

//........................................................................
}   // namespace connectivity
//........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
