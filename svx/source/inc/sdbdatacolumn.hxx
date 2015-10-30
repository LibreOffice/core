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

#ifndef INCLUDED_SVX_SOURCE_INC_SDBDATACOLUMN_HXX
#define INCLUDED_SVX_SOURCE_INC_SDBDATACOLUMN_HXX

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/sdb/XColumn.hpp>
#include <com/sun/star/sdb/XColumnUpdate.hpp>
#include <osl/diagnose.h>


namespace svxform
{



    //= DataColumn - a class wrapping an object implementing a sdb::DataColumn service

    class DataColumn
    {
        // interfaces needed for sddb::Column
        css::uno::Reference< css::beans::XPropertySet>    m_xPropertySet;
        // interfaces needed for sdb::DataColumn
        css::uno::Reference< css::sdb::XColumn>           m_xColumn;
        css::uno::Reference< css::sdb::XColumnUpdate>     m_xColumnUpdate;

    public:
        DataColumn() { };
        DataColumn(const css::uno::Reference< css::beans::XPropertySet>& _rxIFace);
        // if the object behind _rxIFace doesn't fully support the DataColumn service,
        // (which is checked via the supported interfaces) _all_ members will be set to
        // void !, even if the object has some of the needed interfaces.

        DataColumn* operator ->() { return this; }
        operator css::uno::Reference< css::uno::XInterface> () const{ return m_xColumn.get(); }

        // 'conversions'
        inline const css::uno::Reference< css::sdb::XColumn>& getColumn() const
        {
            return m_xColumn;
        }
    };

#endif // INCLUDED_SVX_SOURCE_INC_SDBDATACOLUMN_HXX


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
