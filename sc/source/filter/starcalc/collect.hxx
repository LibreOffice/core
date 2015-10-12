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

#ifndef INCLUDED_SC_SOURCE_FILTER_STARCALC_COLLECT_HXX
#define INCLUDED_SC_SOURCE_FILTER_STARCALC_COLLECT_HXX

#include <sal/types.h>

class ScDataObject
{
public:
               ScDataObject() {}
    virtual    ~ScDataObject();
    virtual    ScDataObject*       Clone() const = 0;
};

class ScCollection : public ScDataObject
{
protected:
    sal_uInt16          nCount;
    sal_uInt16          nLimit;
    sal_uInt16          nDelta;
    ScDataObject**     pItems;
public:
    ScCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4);
    ScCollection(const ScCollection& rCollection);
    virtual                 ~ScCollection();

    virtual ScDataObject*   Clone() const override;

    bool                    AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject);
    bool                    Insert(ScDataObject* pScDataObject);

    ScDataObject*           At(sal_uInt16 nIndex) const;
    sal_uInt16              GetCount() const { return nCount; }

    ScDataObject*  operator[]( const sal_uInt16 nIndex) const { return At(nIndex); }
    ScCollection&  operator= ( const ScCollection& rCol );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
