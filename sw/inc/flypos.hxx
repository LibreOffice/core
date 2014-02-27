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
#ifndef INCLUDED_SW_INC_FLYPOS_HXX
#define INCLUDED_SW_INC_FLYPOS_HXX

#include <swdllapi.h>
#include <boost/shared_ptr.hpp>
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

    const SwFrmFmt& GetFmt() const { return *pFrmFmt; }
    const SwNodeIndex& GetNdIndex() const { return *pNdIdx; }
    sal_uInt32 GetOrdNum() const { return nOrdNum; }
};

// define needed classes to safely handle an array of allocated SwPosFlyFrm(s).
// SwPosFlyFrms can be handled by value (as return value), only refcounts to
// contained SwPosFlyFrm* will be copied. When releasing the last SwPosFlyFrmPtr
// instance the allocated instance will be freed. The array is sorted by
// GetNdIndex by using a ::std::set container.
typedef ::boost::shared_ptr< SwPosFlyFrm > SwPosFlyFrmPtr;
struct SwPosFlyFrmCmp { bool operator()(const SwPosFlyFrmPtr& rA, const SwPosFlyFrmPtr& rB) const; };
typedef ::std::set< SwPosFlyFrmPtr, SwPosFlyFrmCmp > SwPosFlyFrms;

#endif // INCLUDED_SW_INC_FLYPOS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
