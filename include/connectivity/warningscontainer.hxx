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

#ifndef INCLUDED_CONNECTIVITY_WARNINGSCONTAINER_HXX
#define INCLUDED_CONNECTIVITY_WARNINGSCONTAINER_HXX

#include <com/sun/star/sdbc/XWarningsSupplier.hpp>
#include <com/sun/star/sdb/SQLContext.hpp>

#include <connectivity/dbtoolsdllapi.hxx>


namespace dbtools
{

    /** helper class for implementing XWarningsSupplier, which mixes own warnings with
        warnings obtained from an external instance
    */
    class OOO_DLLPUBLIC_DBTOOLS WarningsContainer
    {
    private:
        css::uno::Reference< css::sdbc::XWarningsSupplier >   m_xExternalWarnings;
        css::uno::Any                                         m_aOwnWarnings;

    public:
        WarningsContainer() {}
        WarningsContainer( const css::uno::Reference< css::sdbc::XWarningsSupplier >& _rxExternalWarnings )
            :m_xExternalWarnings( _rxExternalWarnings ) {}

        void setExternalWarnings( const css::uno::Reference< css::sdbc::XWarningsSupplier >& _rxExternalWarnings )
        {
            m_xExternalWarnings = _rxExternalWarnings;
        }

        // convenience
        /** appends an SQLWarning instance to the chain
            @param  _rWarning
                the warning message
            @param  _pAsciiSQLState
                the SQLState of the warning
            @param  _rxContext
                the context of the warning
        */
        void appendWarning(
            const OUString& _rWarning,
            const sal_Char* _pAsciiSQLState,
            const css::uno::Reference< css::uno::XInterface >& _rxContext );

        void appendWarning(const css::sdbc::SQLException& _rWarning);
        void appendWarning(const css::sdbc::SQLWarning& _rWarning);
        void appendWarning(const css::sdb::SQLContext& _rContext);

        // XWarningsSupplier equivalents
        css::uno::Any SAL_CALL getWarnings(  ) const;
        void SAL_CALL clearWarnings(  );
    };


}   // namespace dbtools


#endif // INCLUDED_CONNECTIVITY_WARNINGSCONTAINER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
