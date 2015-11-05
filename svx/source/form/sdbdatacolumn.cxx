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

#include "sdbdatacolumn.hxx"


namespace svxform
{


    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::sdbc;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::io;
    using namespace ::com::sun::star::container;


    //= DataColumn - a class wrapping an object implementing a sdb::DataColumn service

    DataColumn::DataColumn(const Reference< css::beans::XPropertySet>& _rxIFace)
    {
        m_xPropertySet = _rxIFace;
        m_xColumn.set(_rxIFace, UNO_QUERY);
        m_xColumnUpdate.set(_rxIFace, UNO_QUERY);

        if (!m_xPropertySet.is() || !m_xColumn.is())
        {
            m_xPropertySet = NULL;
            m_xColumn = NULL;
            m_xColumnUpdate = NULL;
        }
    }

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
