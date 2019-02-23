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
#ifndef INCLUDED_CONNECTIVITY_SQLSCAN_HXX
#define INCLUDED_CONNECTIVITY_SQLSCAN_HXX

#include <connectivity/IParseContext.hxx>
#include <connectivity/dbtoolsdllapi.hxx>

namespace connectivity
{

    //= OSQLScanner

    /** Scanner for SQL92
    */
    class OOO_DLLPUBLIC_DBTOOLS OSQLScanner
    {
        const IParseContext*    m_pContext;                 // context for parse, knows all international stuff
        OString          m_sStatement;               // statement to parse
        OUString         m_sErrorMessage;

        sal_Int32               m_nCurrentPos;             // next position to read from the statement
        bool                    m_bInternational;          // do we have a statement which may uses
        sal_Int32               m_nRule;                   // rule to be set

    public:
        OSQLScanner();
        virtual ~OSQLScanner();

        sal_Int32 SQLyygetc();
        void SQLyyerror(char const *fmt);
        IParseContext::InternationalKeyCode getInternationalTokenID(const char* sToken) const;

        // setting the new information before scanning
        void prepareScan(const OUString & rNewStatement, const IParseContext* pContext, bool bInternational);
        const OUString& getErrorMessage() const {return m_sErrorMessage;}
        const OString& getStatement() const { return m_sStatement; }

        static sal_Int32 SQLlex();
        // set this as scanner for flex
        void setScanner(bool _bNull=false);
        // rules settings
        void SetRule(sal_Int32 nRule) {m_nRule = nRule;}
        static sal_Int32 GetGERRule();
        static sal_Int32 GetENGRule();
        static sal_Int32 GetSQLRule();
        static sal_Int32 GetDATERule();
        static sal_Int32 GetSTRINGRule();
        sal_Int32 GetCurrentPos() const { return m_nCurrentPos; }
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
