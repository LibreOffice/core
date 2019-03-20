/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_ITEM_SIMPLE_CNTINT16_HXX
#define INCLUDED_ITEM_SIMPLE_CNTINT16_HXX

#include <item/base/IBaseStaticHelper.hxx>

///////////////////////////////////////////////////////////////////////////////

namespace Item
{
    // example for SfxInt16Item
    // It uses IAdministrator_set to provide an Administrator using
    // a sorted list for fast accesses. This requires ::operator< to be
    // implemented.
    class CntInt16;
    typedef IBaseStaticHelper<CntInt16, IAdministrator_set> CntInt16StaticHelper;

    class ITEM_DLLPUBLIC CntInt16 : public CntInt16StaticHelper
    {
    private:
        sal_Int16 m_nValue;

    protected:
        friend CntInt16StaticHelper;

        CntInt16(sal_Int16 nValue = 0);

    public:
        virtual ~CntInt16();

        static CntInt16::SharedPtr Create(sal_Int16 nValue);
        virtual bool operator==(const IBase& rCandidate) const override;
        virtual bool operator<(const IBase& rCandidate) const override;
        sal_Int16 GetValue() const;
    };
} // end of namespace Item

///////////////////////////////////////////////////////////////////////////////

#endif // INCLUDED_ITEM_SIMPLE_CNTINT16_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
