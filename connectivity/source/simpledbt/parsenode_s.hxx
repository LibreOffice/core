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

#ifndef CONNECTIVITY_PARSENODE_SIMPLE_HXX
#define CONNECTIVITY_PARSENODE_SIMPLE_HXX

#include <connectivity/virtualdbtools.hxx>
#include "refbase.hxx"


namespace connectivity
{


    class OSQLParseNode;

    //= OSimpleParseNode

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
        virtual void parseNodeToStr(OUString& _rString,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const IParseContext* _pContext
        ) const SAL_OVERRIDE;

        virtual void parseNodeToPredicateStr(OUString& _rString,
            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection >& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField,
            const OUString &_sPredicateTableAlias,
            const ::com::sun::star::lang::Locale& _rIntl,
            const sal_Char _cDecSeparator,
            const IParseContext* _pContext
        ) const SAL_OVERRIDE;

        // disambiguate IReference
        virtual oslInterlockedCount SAL_CALL acquire() SAL_OVERRIDE;
        virtual oslInterlockedCount SAL_CALL release() SAL_OVERRIDE;
    };


}   // namespace connectivity


#endif // CONNECTIVITY_PARSENODE_SIMPLE_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
