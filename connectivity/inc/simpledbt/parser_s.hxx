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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_SIMPLEDBT_PARSER_S_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_SIMPLEDBT_PARSER_S_HXX

#include <connectivity/virtualdbtools.hxx>
#include <connectivity/sqlparse.hxx>


namespace connectivity
{



    //= OSimpleSQLParser

    class OSimpleSQLParser : public simple::ISQLParser
    {
    protected:
        OSQLParser      m_aFullParser;

    public:
        OSimpleSQLParser(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext, const IParseContext* _pContext);

        // ISQLParser
        virtual ::rtl::Reference< simple::ISQLParseNode > predicateTree(
            OUString& rErrorMessage,
            const OUString& rStatement,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField
        ) const SAL_OVERRIDE;

        virtual const IParseContext& getContext() const SAL_OVERRIDE;
    };


}   // namespace connectivity


#endif // INCLUDED_CONNECTIVITY_SOURCE_SIMPLEDBT_PARSER_S_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
