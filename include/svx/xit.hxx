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
#include <svx/svxdllapi.h>
#include <unotools/resmgr.hxx>

/************************************************************************/

class SfxItemPool;
class NameOrIndex;

typedef bool (*SvxCompareValueFunc)( const NameOrIndex* p1, const NameOrIndex* p2 );



class SVXCORE_DLLPUBLIC NameOrIndex : public SfxStringItem
{
    sal_Int32    m_nPalIndex;

protected:
    void    Detach()    { m_nPalIndex = -1; }

public:
            DECLARE_ITEM_TYPE_FUNCTION(NameOrIndex)
            NameOrIndex() : SfxStringItem(0) { m_nPalIndex = -1; }
            NameOrIndex(TypedWhichId<NameOrIndex> nWhich, sal_Int32 nIndex);
            NameOrIndex(TypedWhichId<NameOrIndex> nWhich, const OUString& rName);
            NameOrIndex(const NameOrIndex& rNameOrIndex);

    virtual bool         operator==(const SfxPoolItem& rItem) const override;
    virtual NameOrIndex* Clone(SfxItemPool* pPool = nullptr) const override;
    // Marked as false since the SfxStringItem superclass supports hashing, but
    // this class has not been checked for safety under hashing yet.
    virtual bool         supportsHashCode() const override { return false; }

            OUString const & GetName() const              { return GetValue();   }
            void         SetName(const OUString& rName) { SetValue(rName);     }
            bool         IsIndex() const          { return (m_nPalIndex >= 0); }
            sal_Int32    GetPalIndex() const { return m_nPalIndex; }

    /** this checks if the given NameOrIndex item has a unique name for its value.
        The returned String is a unique name for an item with this value in both given pools.
        If returned string equals NameOrIndex->GetName(), the name was already unique.
    */
    OUString CheckNamedItem(const sal_uInt16 nWhich, const SfxItemPool* pPool1, SvxCompareValueFunc pCompareValueFunc, TranslateId pPrefixResId, const XPropertyListRef &pDefaults) const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
