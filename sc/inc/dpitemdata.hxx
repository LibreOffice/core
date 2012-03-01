/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 *   Copyright (C) 2012 Kohei Yoshida <kohei.yoshida@suse.com>
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef __SC_DPITEMDATA_HXX__
#define __SC_DPITEMDATA_HXX__

#include "scdllapi.h"
#include "address.hxx"

#include "sal/types.h"
#include "tools/solar.h"
#include "rtl/ustring.hxx"

#include <vector>

#include <boost/unordered_map.hpp>

class ScDocument;

class SC_DLLPUBLIC ScDPItemData
{
    friend class ScDPCache;
public:
    enum {
        MK_VAL      = 0x01,
        MK_DATA     = 0x02,
        MK_ERR      = 0x04,
        MK_DATE     = 0x08
    };

    static bool isDate( sal_uLong nNumType );

private:
    rtl::OUString maString;
    double mfValue;
    sal_uInt8 mbFlag;

public:
    ScDPItemData();
    ScDPItemData(const rtl::OUString& rS, double fV, sal_uInt8 bF);
    ScDPItemData(const rtl::OUString& rS, double fV = 0.0, bool bHV = false, bool bData = true);

    void Set(const rtl::OUString& rS, double fVal, sal_uInt8 nFlag);
    void SetString(const rtl::OUString& rS);
    void SetErrorString(const rtl::OUString& rS);
    bool IsCaseInsEqual(const ScDPItemData& r) const;

    size_t Hash() const;

    // exact equality
    bool operator==( const ScDPItemData& r ) const;
    // case insensitive equality
    static sal_Int32 Compare( const ScDPItemData& rA, const ScDPItemData& rB );

public:
    bool IsHasData() const ;
    bool IsHasErr() const ;
    bool IsValue() const;
    const rtl::OUString& GetString() const;
    double GetValue() const ;
    bool HasStringData() const ;

    sal_uInt8 GetType() const;
};

class SC_DLLPUBLIC ScDPItemDataPool
{
public:
    ScDPItemDataPool();
    ScDPItemDataPool(const ScDPItemDataPool& r);

    virtual ~ScDPItemDataPool();
    virtual const ScDPItemData* getData( sal_Int32 nId  );
    virtual sal_Int32 getDataId( const ScDPItemData& aData );
    virtual sal_Int32 insertData( const ScDPItemData& aData );
protected:
    struct DataHashFunc : public std::unary_function< const ScDPItemData &, size_t >
    {
        size_t operator() (const ScDPItemData &rData) const { return rData.Hash(); }
    };

    typedef ::boost::unordered_multimap< ScDPItemData, sal_Int32, DataHashFunc > DataHash;

    ::std::vector< ScDPItemData > maItems;
    DataHash  maItemIds;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
