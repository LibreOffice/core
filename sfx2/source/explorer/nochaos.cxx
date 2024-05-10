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

#include <sal/config.h>

#include <svl/itempool.hxx>
#include <svl/poolitem.hxx>
#include <svl/stritem.hxx>
#include <nochaos.hxx>
#include <memory>


#define WID_CHAOS_START TypedWhichId<SfxStringItem>(500)


namespace {
class CntItemPool: public SfxItemPool
{
    static CntItemPool* _pThePool;
    sal_uInt16              _nRefs;

protected:
             CntItemPool();
    virtual ~CntItemPool() override;

public:
    static CntItemPool* Acquire();
    static sal_uInt16       Release();
};

}

// static
SfxItemPool* NoChaos::GetItemPool()
{
    // Get and hold CHAOS item pool.
    return CntItemPool::Acquire();
}


// static
sal_uInt16 NoChaos::ReleaseItemPool()
{
    // Release CHAOS item pool.
    return CntItemPool::Release();
}


//  CntItemPool implementation
static ItemInfoPackage& getItemInfoPackageNoChaos()
{
    class ItemInfoPackageNoChaos : public ItemInfoPackage
    {
        typedef std::array<ItemInfoStatic, 1> ItemInfoArrayNoChaos;
        ItemInfoArrayNoChaos maItemInfos {{
            // m_nWhich, m_pItem, m_nSlotID, m_nItemInfoFlags
            { WID_CHAOS_START, new SfxStringItem(WID_CHAOS_START, OUString()), 0, SFX_ITEMINFOFLAG_NONE }
        }};

        virtual const ItemInfoStatic& getItemInfoStatic(size_t nIndex) const override { return maItemInfos[nIndex]; }

    public:
        virtual size_t size() const override { return maItemInfos.size(); }
        virtual const ItemInfo& getItemInfo(size_t nIndex, SfxItemPool& /*rPool*/) override { return maItemInfos[nIndex]; }
    };

    static std::unique_ptr<ItemInfoPackageNoChaos> g_aItemInfoPackageNoChaos;
    if (!g_aItemInfoPackageNoChaos)
        g_aItemInfoPackageNoChaos.reset(new ItemInfoPackageNoChaos);
    return *g_aItemInfoPackageNoChaos;
}


// static CntStaticPoolDefaults_Impl* pPoolDefs_Impl = nullptr;

// static member!
CntItemPool* CntItemPool::_pThePool = nullptr;

CntItemPool::CntItemPool()
: SfxItemPool(u"chaos"_ustr)
, _nRefs(0)
{
    registerItemInfoPackage(getItemInfoPackageNoChaos());
}


//virtual
CntItemPool::~CntItemPool()
{
}


// static
CntItemPool* CntItemPool::Acquire()
{
    if ( !_pThePool )
        _pThePool = new CntItemPool;

    _pThePool->_nRefs++;

    return _pThePool;
}


// static
sal_uInt16 CntItemPool::Release()
{
    if ( !_pThePool )
        return 0;

    sal_uInt16& nRefs = _pThePool->_nRefs;

    if ( nRefs )
        --nRefs;

    if ( !nRefs )
    {
        delete _pThePool;
        _pThePool = nullptr;
        return 0;
    }

    return nRefs;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
