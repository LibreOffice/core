/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_connectivity.hxx"
#include "charset_s.hxx"

//........................................................................
namespace connectivity
{
//........................................................................

    using namespace ::dbtools;

    //====================================================================
    //= ODataAccessCharSet
    //====================================================================
    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessCharSet::acquire()
    {
        return ORefBase::acquire();
    }

    //----------------------------------------------------------------
    oslInterlockedCount SAL_CALL ODataAccessCharSet::release()
    {
        return ORefBase::release();
    }

    //--------------------------------------------------------------------
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

//........................................................................
}   // namespace connectivity
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
