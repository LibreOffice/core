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

#include <sqlparserclient.hxx>
#include <svx/ParseContext.hxx>

#include <connectivity/dbtools.hxx>
#include <connectivity/sqlparse.hxx>

using namespace ::dbtools;
using namespace ::connectivity;

namespace svxform
{
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;

    OSQLParserClient::OSQLParserClient(const Reference< XComponentContext >& rxContext)
        : m_pParser(new OSQLParser(rxContext, getParseContext()))
    {
    }

    std::unique_ptr< ::connectivity::OSQLParseNode > OSQLParserClient::predicateTree(
            OUString& _rErrorMessage,
            const OUString& _rStatement,
            const css::uno::Reference< css::util::XNumberFormatter >& _rxFormatter,
            const css::uno::Reference< css::beans::XPropertySet >& _rxField
        ) const
    {
        return m_pParser->predicateTree(_rErrorMessage, _rStatement, _rxFormatter, _rxField);
    }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
