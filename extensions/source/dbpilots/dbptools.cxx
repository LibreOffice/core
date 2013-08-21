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

#include "dbptools.hxx"
#include <tools/debug.hxx>

//.........................................................................
namespace dbp
{
//.........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::container;

    //---------------------------------------------------------------------
    void disambiguateName(const Reference< XNameAccess >& _rxContainer, OUString& _rElementsName)
    {
        DBG_ASSERT(_rxContainer.is(), "::dbp::disambiguateName: invalid container!");
        if (!_rxContainer.is())
            return;

        try
        {
            OUString sBase(_rElementsName);
            for (sal_Int32 i=1; i<0x7FFFFFFF; ++i)
            {
                _rElementsName = sBase;
                _rElementsName += OUString::number(i);
                if (!_rxContainer->hasByName(_rElementsName))
                    return;
            }
            // can't do anything ... no free names
            _rElementsName = sBase;
        }
        catch(const Exception&)
        {
            OSL_FAIL("::dbp::disambiguateName: something went (strangely) wrong!");
        }
    }

//.........................................................................
}   // namespace dbp
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
