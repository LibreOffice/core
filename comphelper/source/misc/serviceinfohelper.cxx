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


#include <comphelper/serviceinfohelper.hxx>

// #####################################################################

namespace comphelper
{

/** returns an empty UString(). most times sufficient */
OUString SAL_CALL ServiceInfoHelper::getImplementationName() throw( css::uno::RuntimeException, std::exception )
{
    return OUString();
}

/** the base implementation has no supported services */
css::uno::Sequence< OUString > ServiceInfoHelper::getSupportedServiceNames() throw( css::uno::RuntimeException, std::exception )
{
    css::uno::Sequence< OUString> aSeq(0);
    return aSeq;
}

/** this method adds a variable number of OUString to a given Sequence
 */
void ServiceInfoHelper::addToSequence( css::uno::Sequence< OUString >& rSeq, std::initializer_list<OUString> services ) throw()
{
    sal_uInt32 nCount = rSeq.getLength();

    rSeq.realloc( nCount + services.size() );
    OUString* pStrings = rSeq.getArray();

    for( auto const & s: services )
        pStrings[nCount++] = s;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
