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
#ifndef INCLUDED_REPORTDESIGN_SOURCE_INC_GROUPPROPERTIES_HXX
#define INCLUDED_REPORTDESIGN_SOURCE_INC_GROUPPROPERTIES_HXX

namespace rptshared
{
    /** struct for the group properties
    */
    struct GroupProperties
    {
        sal_Int32                                                                 m_nGroupInterval;
        OUString                                                                  m_sExpression;
        sal_Int16                                                                 m_nGroupOn;
        sal_Int16                                                                 m_nKeepTogether;
        bool                                                                      m_eSortAscending;
        bool                                                                      m_bStartNewColumn;
        bool                                                                      m_bResetPageNumber;

        GroupProperties()
        :m_nGroupInterval(1)
        ,m_nGroupOn(0)
        ,m_nKeepTogether(0)
        ,m_eSortAscending(true)
        ,m_bStartNewColumn(false)
        ,m_bResetPageNumber(false)
        {}
    };
}
#endif // INCLUDED_REPORTDESIGN_SOURCE_INC_GROUPPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
