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

#ifndef INCLUDED_SVX_SOURCE_INC_SQLPARSERCLIENT_HXX
#define INCLUDED_SVX_SOURCE_INC_SQLPARSERCLIENT_HXX

#include <svx/ParseContext.hxx>

namespace com { namespace sun { namespace star {
namespace util {
    class XNumberFormatter;
}
namespace beans {
    class XPropertySet;
} } } }

namespace connectivity {
    class OSQLParser;
    class OSQLParseNode;
}

namespace svxform
{
    //= OSQLParserClient

    class SVX_DLLPUBLIC OSQLParserClient : public ::svxform::OParseContextClient
    {
    protected:
        mutable std::shared_ptr< ::connectivity::OSQLParser > m_pParser;

        OSQLParserClient(
            const css::uno::Reference< css::uno::XComponentContext >& rxContext);

        std::unique_ptr< ::connectivity::OSQLParseNode > predicateTree(
                OUString& _rErrorMessage,
                const OUString& _rStatement,
                const css::uno::Reference< css::util::XNumberFormatter >& _rxFormatter,
                const css::uno::Reference< css::beans::XPropertySet >& _rxField
            ) const;
    };


}


#endif // INCLUDED_SVX_SOURCE_INC_SQLPARSERCLIENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
