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

#ifndef SC_COLLECT_HXX
#define SC_COLLECT_HXX

#include "sal/types.h"

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
    ScDataObject**  pItems;
public:
    ScCollection(sal_uInt16 nLim = 4, sal_uInt16 nDel = 4);
    ScCollection(const ScCollection& rCollection);
    virtual             ~ScCollection();

    virtual ScDataObject*   Clone() const;

    sal_Bool        AtInsert(sal_uInt16 nIndex, ScDataObject* pScDataObject);
    virtual sal_Bool        Insert(ScDataObject* pScDataObject);

    ScDataObject*   At(sal_uInt16 nIndex) const;
    virtual sal_uInt16      IndexOf(ScDataObject* pScDataObject) const;
    sal_uInt16 GetCount() const;

            ScDataObject* operator[]( const sal_uInt16 nIndex) const {return At(nIndex);}
            ScCollection&   operator=( const ScCollection& rCol );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
