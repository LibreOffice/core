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

#include "charset_s.hxx"


namespace connectivity
{


    using namespace ::dbtools;


    //= ODataAccessCharSet


    oslInterlockedCount SAL_CALL ODataAccessCharSet::acquire()
    {
        return ORefBase::acquire();
    }


    oslInterlockedCount SAL_CALL ODataAccessCharSet::release()
    {
        return ORefBase::release();
    }


    sal_Int32 ODataAccessCharSet::getSupportedTextEncodings( ::std::vector< rtl_TextEncoding >& _rEncs ) const
    {
        _rEncs.clear();

        OCharsetMap::const_iterator aLoop = m_aCharsetInfo.begin();
        OCharsetMap::const_iterator aLoopEnd = m_aCharsetInfo.end();
        while (aLoop != aLoopEnd)
        {
            _rEncs.push_back( (*aLoop).getEncoding() );
            ++aLoop;
        }

        return _rEncs.size();
    }


}   // namespace connectivity


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
