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
#ifndef _FLYPOS_HXX
#define _FLYPOS_HXX


#include <swdllapi.h>
#include <set>

class SwFrmFmt;
class SwNodeIndex;


/// For querying current flys in document.
class SW_DLLPUBLIC SwPosFlyFrm
{
    const SwFrmFmt* pFrmFmt;    ///< FlyFrmFmt
    SwNodeIndex* pNdIdx;        ///< Index for node is sufficient.
    sal_uInt32 nOrdNum;
public:
    SwPosFlyFrm( const SwNodeIndex& , const SwFrmFmt*, sal_uInt16 nArrPos );
    virtual ~SwPosFlyFrm(); ///< Virtual for Writer (DLL !!)

    /// Operators for sort array.
    sal_Bool operator==( const SwPosFlyFrm& );
    sal_Bool operator<( const SwPosFlyFrm& );

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }
    sal_uInt32 GetOrdNum() const { return nOrdNum; }
};

class SwPosFlyFrms : public std::multiset<SwPosFlyFrm*> {};

#endif // _FLYPOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
