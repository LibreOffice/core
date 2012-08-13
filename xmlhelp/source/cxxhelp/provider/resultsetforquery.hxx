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
#ifndef _RESULTSETFORQUERY_HXX
#define _RESULTSETFORQUERY_HXX

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/ucb/XContentProvider.hpp>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/ucb/NumberedSortingInfo.hpp>

#include "resultsetbase.hxx"
#include "urlparameter.hxx"

namespace chelp {

    class Databases;

    class ResultSetForQuery
        : public ResultSetBase
    {
    public:

        ResultSetForQuery( const com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory>&  xMSF,
                           const com::sun::star::uno::Reference<com::sun::star::ucb::XContentProvider>&  xProvider,
                           sal_Int32 nOpenMode,
                           const com::sun::star::uno::Sequence< com::sun::star::beans::Property >& seq,
                           const com::sun::star::uno::Sequence< com::sun::star::ucb::NumberedSortingInfo >& seqSort,
                           URLParameter& aURLParameter,
                           Databases* pDatabases );


    private:

        URLParameter m_aURLParameter;
    };

}


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
