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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_LABREC_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_LABREC_HXX

#include <rtl/ustring.hxx>
#include <tools/long.hxx>

#include <memory>
#include <vector>


class SwLabItem;

class SwLabRec
{
public:
    SwLabRec(): m_nHDist(0), m_nVDist(0), m_nWidth(0), m_nHeight(0), m_nLeft(0), m_nUpper(0), m_nPWidth(0), m_nPHeight(0), m_nCols(0), m_nRows(0), m_bCont(false) {}

    void SetFromItem( const SwLabItem& rItem );
    void FillItem( SwLabItem& rItem ) const;

    OUString        m_aMake;
    OUString        m_aType;
    tools::Long            m_nHDist;
    tools::Long            m_nVDist;
    tools::Long            m_nWidth;
    tools::Long            m_nHeight;
    tools::Long            m_nLeft;
    tools::Long            m_nUpper;
    tools::Long            m_nPWidth;
    tools::Long            m_nPHeight;
    sal_Int32       m_nCols;
    sal_Int32       m_nRows;
    bool        m_bCont;
};

typedef std::vector<std::unique_ptr<SwLabRec>> SwLabRecs;

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
