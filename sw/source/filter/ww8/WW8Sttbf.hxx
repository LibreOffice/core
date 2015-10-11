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

#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8STTBF_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8STTBF_HXX

#include <memory>
#include <vector>
#include <boost/shared_array.hpp>
#include <tools/solar.h>
#include <rtl/ustring.hxx>
#include <IDocumentExternalData.hxx>

class SvStream;

namespace ww8
{

    class WW8Struct : public ::sw::ExternalData
    {
        boost::shared_array<sal_uInt8> mp_data;
        sal_uInt32 mn_offset;
        sal_uInt32 mn_size;

    public:
        WW8Struct(SvStream& rSt, sal_uInt32 nPos, sal_uInt32 nSize);
        WW8Struct(WW8Struct * pStruct, sal_uInt32 nPos, sal_uInt32 nSize);
        virtual ~WW8Struct();

        sal_uInt8 getU8(sal_uInt32 nOffset);

        sal_uInt16 getU16(sal_uInt32 nOffset)
        { return getU8(nOffset) + (getU8(nOffset + 1) << 8); }

        OUString getUString(sal_uInt32 nOffset, sal_uInt32 nCount);
    };

    template <class T>
    class WW8Sttb : public WW8Struct
    {
        typedef std::shared_ptr< void > ExtraPointer_t;
        bool                            bDoubleByteCharacters;
        std::vector<OUString>           m_Strings;
        std::vector< ExtraPointer_t >   m_Extras;

    public:
        WW8Sttb(SvStream& rSt, sal_Int32 nPos, sal_uInt32 nSize);
        virtual ~WW8Sttb();

        std::vector<OUString> & getStrings()
        {
            return m_Strings;
        }
    };

    template <class T>
    WW8Sttb<T>::WW8Sttb(SvStream& rSt, sal_Int32 nPos, sal_uInt32 nSize)
    : WW8Struct(rSt, nPos, nSize), bDoubleByteCharacters(false)
    {
        sal_uInt32 nOffset = 0;

        if (getU16(nOffset) == 0xffff)
        {
            bDoubleByteCharacters = true;
            nOffset += 2;
        }

        sal_uInt16 nCount = getU16(nOffset);
        sal_uInt16 ncbExtra = getU16(nOffset + 2);

        nOffset += 4;
        for (sal_uInt16 i = 0; i < nCount; i++)
        {
            if (bDoubleByteCharacters)
            {
                sal_uInt16 nStrLen = getU16(nOffset);

                m_Strings.push_back(getUString(nOffset +2, nStrLen));

                nOffset += 2 + 2 * nStrLen;
            }
            else
            {
                sal_uInt8 nStrLen = getU8(nOffset);

                m_Strings.push_back(getUString(nOffset, nStrLen));

                nOffset += 1 + nStrLen;
            }

            if (ncbExtra > 0)
            {
                ExtraPointer_t pExtra(new T(this, nOffset, ncbExtra));
                m_Extras.push_back(pExtra);

                nOffset += ncbExtra;
            }
        }
    }

    template <class T>
    WW8Sttb<T>::~WW8Sttb()
    {
    }
}

#endif // INCLUDED_SW_SOURCE_FILTER_WW8_WW8STTBF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
