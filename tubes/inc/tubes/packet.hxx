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
 * Copyright (C) 2012 Red Hat, Inc., Eike Rathke <erack@redhat.com>
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

#ifndef INCLUDED_TUBES_PACKET_HXX
#define INCLUDED_TUBES_PACKET_HXX

#include <sal/config.h>
#include <rtl/string.hxx>
#include <rtl/byteseq.hxx>


class TelePacket
{
public:

    TelePacket( const char* pSender, const char* pData, int nSize )
        :
            maSender( pSender),
            maData( reinterpret_cast<const sal_Int8*>(pData), static_cast<sal_Int32>(nSize) )
    {
    }

    explicit TelePacket( const TelePacket& r )
        :
            maSender( r.maSender),
            maData( r.maData)
    {
    }

    TelePacket() {}

    /** Underlying getArray() ensures reference count is exactly one, hence
        this method is non-const! */
    const char*         getData() { return reinterpret_cast<const char*>( maData.getArray()); }
    sal_Int32           getSize() const { return maData.getLength(); }

    bool                operator==( const TelePacket& r ) const
                            { return maSender == r.maSender && maData == r.maData; }

    TelePacket&         operator=( const TelePacket& r )
                            {
                                if (this == &r)
                                    return *this;
                                maSender = r.maSender;
                                maData = r.maData;
                                return *this;
                            }

private:

    rtl::OString        maSender;
    rtl::ByteSequence   maData;

};

#endif // INCLUDED_TUBES_PACKET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
