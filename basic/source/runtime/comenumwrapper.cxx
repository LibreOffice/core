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

#include "comenumwrapper.hxx"

using namespace ::com::sun::star;

sal_Bool SAL_CALL ComEnumerationWrapper::hasMoreElements()
{
    bool bResult = false;

    try
    {
        if ( m_xInvocation.is() )
        {
            sal_Int32 nLength = 0;
            bResult = ( ( m_xInvocation->getValue( u"length"_ustr ) >>= nLength ) && nLength > m_nCurInd );
        }
    }
    catch(const uno::Exception& )
    {}

    return bResult;
}

uno::Any SAL_CALL ComEnumerationWrapper::nextElement()
{
    try
    {
        if ( m_xInvocation.is() )
        {
            uno::Sequence< sal_Int16 > aNamedParamIndex;
            uno::Sequence< uno::Any > aNamedParam;
            uno::Sequence< uno::Any > aArgs{ uno::Any(m_nCurInd++) };

            return m_xInvocation->invoke( u"item"_ustr,
                                          aArgs,
                                          aNamedParamIndex,
                                          aNamedParam );
        }
    }
    catch(const uno::Exception& )
    {}

    throw container::NoSuchElementException();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
