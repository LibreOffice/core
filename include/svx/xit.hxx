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

#ifndef INCLUDED_SVX_XIT_HXX
#define INCLUDED_SVX_XIT_HXX

#include <svl/stritem.hxx>
#include <svx/xtable.hxx>
#include <svx/xdef.hxx>
#include <svx/svxdllapi.h>

/************************************************************************/

class SfxItemPool;
class NameOrIndex;

typedef bool (*SvxCompareValueFunc)( const NameOrIndex* p1, const NameOrIndex* p2 );


// class NameOrIndex

class SVX_DLLPUBLIC NameOrIndex : public SfxStringItem
{
    sal_Int32    nPalIndex;

protected:
    void    Detach()    { nPalIndex = -1; }

public:
            NameOrIndex() { nPalIndex = -1; }
            NameOrIndex(sal_uInt16 nWhich, sal_Int32 nIndex);
            NameOrIndex(sal_uInt16 nWhich, const OUString& rName);
            NameOrIndex(const NameOrIndex& rNameOrIndex);

    virtual bool         operator==(const SfxPoolItem& rItem) const override;
    virtual SfxPoolItem* Clone(SfxItemPool* pPool = nullptr) const override;

            OUString     GetName() const              { return GetValue();   }
            void         SetName(const OUString& rName) { SetValue(rName);     }
            bool         IsIndex() const          { return (nPalIndex >= 0); }

    /** this static checks if the given NameOrIndex item has a unique name for its value.
        The returned String is a unique name for an item with this value in both given pools.
        Argument pPool2 can be null.
        If returned string equals NameOrIndex->GetName(), the name was already unique.
    */
    static OUString CheckNamedItem( const NameOrIndex* pCheckItem, const sal_uInt16 nWhich, const SfxItemPool* pPool1, SvxCompareValueFunc pCompareValueFunc, const char* pPrefixResId, const XPropertyListRef &pDefaults );

    void dumpAsXml(struct _xmlTextWriter* pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
