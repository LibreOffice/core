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

#ifndef CONNECTIVITY_PARSENODE_SIMPLE_HXX
#define CONNECTIVITY_PARSENODE_SIMPLE_HXX

#include <connectivity/virtualdbtools.hxx>
#include "refbase.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    class OSQLParseNode;
    //================================================================
    //= OSimpleParseNode
    //================================================================
    class OSimpleParseNode
            :public simple::ISQLParseNode
            ,public ORefBase
    {
    protected:
        const OSQLParseNode*    m_pFullNode;
        sal_Bool                m_bOwner;

    public:
        OSimpleParseNode(const OSQLParseNode* _pNode, sal_Bool _bTakeOwnership = sal_True);
        ~OSimpleParseNode();

        // ISQLParseNode
        virtual void parseNodeToStr(::rtl::OUString& _rString,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const IParseContext* _pContext
        ) const;

        virtual void parseNodeToPredicateStr(::rtl::OUString& _rString,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
            const ::com::sun::star::lang::Locale& _rIntl,
            const sal_Char _cDecSeparator,
            const IParseContext* _pContext
        ) const;

        // disambiguate IReference
        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();
    };

//........................................................................
}   // namespace connectivity
//........................................................................

#endif // CONNECTIVITY_PARSENODE_SIMPLE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
