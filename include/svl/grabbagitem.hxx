/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SFXGRABBAGITEM_HXX
#define _SFXGRABBAGITEM_HXX

#include <map>

#include "svl/svldllapi.h"
#include <tools/rtti.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/uno/Any.hxx>

/// Grab bag item provides a string-any map for interim interop purposes.
class SVL_DLLPUBLIC SfxGrabBagItem : public SfxPoolItem
{
private:
    std::map<OUString, com::sun::star::uno::Any> m_aMap;

public:
    TYPEINFO();

    SfxGrabBagItem();
    SfxGrabBagItem(sal_uInt16 nWhich, const std::map<OUString, com::sun::star::uno::Any> *pMap = 0);
    SfxGrabBagItem(const SfxGrabBagItem& rItem);
    ~SfxGrabBagItem();

    void SetGrabBag(const std::map<OUString, com::sun::star::uno::Any>& rMap);
    const std::map<OUString, com::sun::star::uno::Any>& GetGrabBag() const;

    virtual int operator==(const SfxPoolItem&) const;
    virtual SfxPoolItem* Clone(SfxItemPool *pPool = 0) const;

    virtual bool PutValue(const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0);
    virtual bool QueryValue(com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0) const;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
