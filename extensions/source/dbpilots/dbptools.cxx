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
#include "precompiled_extensions.hxx"
#include "dbptools.hxx"
#include <tools/debug.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;

    //---------------------------------------------------------------------
    void disambiguateName(const Reference< XNameAccess >& _rxContainer, ::rtl::OUString& _rElementsName)
    {
        DBG_ASSERT(_rxContainer.is(), "::dbp::disambiguateName: invalid container!");
        if (!_rxContainer.is())
            return;

        try
        {
            ::rtl::OUString sBase(_rElementsName);
            for (sal_Int32 i=1; i<0x7FFFFFFF; ++i)
            {
                _rElementsName = sBase;
                _rElementsName += ::rtl::OUString::valueOf((sal_Int32)i);
                if (!_rxContainer->hasByName(_rElementsName))
                    return;
            }
            // can't do anything ... no free names
            _rElementsName = sBase;
        }
        catch(Exception&)
        {
            OSL_FAIL("::dbp::disambiguateName: something went (strangely) wrong!");
        }
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
