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

#ifndef SVX_SQLPARSERCLIENT_HXX
#define SVX_SQLPARSERCLIENT_HXX

#include "svx/dbtoolsclient.hxx"
#include "svx/ParseContext.hxx"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

//........................................................................
namespace svxform
{
//........................................................................

    //====================================================================
    //= OSQLParserClient
    //====================================================================
    class SVX_DLLPUBLIC OSQLParserClient : public ODbtoolsClient
                            ,public ::svxform::OParseContextClient
    {
    private:

        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > m_xContext;

    protected:
        mutable ::rtl::Reference< ::connectivity::simple::ISQLParser >  m_xParser;

    protected:
        OSQLParserClient(
            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext);
        virtual bool ensureLoaded() const;

    protected:
        inline ::rtl::Reference< ::connectivity::simple::ISQLParseNode > predicateTree(
                OUString& _rErrorMessage,
                const OUString& _rStatement,
                const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxFormatter,
                const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >& _rxField
            ) const
        {
            ::rtl::Reference< ::connectivity::simple::ISQLParseNode > xReturn;
            if ( ensureLoaded() )
                xReturn = m_xParser->predicateTree(_rErrorMessage, _rStatement, _rxFormatter, _rxField);
            return xReturn;
        }
    };

//........................................................................
}   // namespace svxform
//........................................................................

#endif // SVX_SQLPARSERCLIENT_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
