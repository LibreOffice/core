/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/propertyvalue.hxx>
#include <svl/memberid.h>
#include <svx/statusitem.hxx>

constexpr OUString STATUS_PARAM_VALUE = u"Value"_ustr;
constexpr OUString STATUS_PARAM_TYPE = u"Type"_ustr;
constexpr int STATUS_PARAMS = 2;

SvxStatusItem::SvxStatusItem(TypedWhichId<SvxStatusItem> nWhich, const OUString& rString,
                             StatusCategory eCategory)
    : SfxStringItem(nWhich, rString)
    , m_eCategory(eCategory)
{
}

bool SvxStatusItem::operator==(const SfxPoolItem& rItem) const
{
    return SfxStringItem::operator==(rItem)
           && static_cast<const SvxStatusItem&>(rItem).m_eCategory == m_eCategory;
}

bool SvxStatusItem::QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId) const
{
    nMemberId &= ~CONVERT_TWIPS;

    switch (nMemberId)
    {
        case 0:
        {
            css::uno::Sequence<css::beans::PropertyValue> aSeq{
                comphelper::makePropertyValue(STATUS_PARAM_VALUE, GetValue()),
                comphelper::makePropertyValue(STATUS_PARAM_TYPE,
                                              static_cast<sal_Int16>(m_eCategory))
            };
            assert(aSeq.getLength() == STATUS_PARAMS);
            rVal <<= aSeq;
            break;
        }
        case MID_VALUE:
            rVal <<= GetValue();
            break;
        case MID_TYPE:
            rVal <<= static_cast<sal_Int16>(m_eCategory);
            break;
        default:
            return false;
    }

    return true;
}

bool SvxStatusItem::PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId)
{
    nMemberId &= ~CONVERT_TWIPS;
    bool bRet;
    switch (nMemberId)
    {
        case 0:
        {
            css::uno::Sequence<css::beans::PropertyValue> aSeq;
            if ((rVal >>= aSeq) && (aSeq.getLength() == STATUS_PARAMS))
            {
                OUString sValueTmp;
                sal_Int16 nTypeTmp(0);
                bool bAllConverted(true);
                sal_Int16 nConvertedCount(0);
                for (const auto& rProp : aSeq)
                {
                    if (rProp.Name == STATUS_PARAM_VALUE)
                    {
                        bAllConverted &= (rProp.Value >>= sValueTmp);
                        ++nConvertedCount;
                    }
                    else if (rProp.Name == STATUS_PARAM_TYPE)
                    {
                        bAllConverted &= (rProp.Value >>= nTypeTmp);
                        ++nConvertedCount;
                    }
                }

                if (bAllConverted && nConvertedCount == STATUS_PARAMS)
                {
                    SetValue(sValueTmp);
                    m_eCategory = static_cast<StatusCategory>(nTypeTmp);
                    return true;
                }
            }
            return false;
        }
        case MID_TYPE:
        {
            sal_Int16 nCategory;
            bRet = (rVal >>= nCategory);
            if (bRet)
                m_eCategory = static_cast<StatusCategory>(nCategory);
            break;
        }
        case MID_VALUE:
        {
            OUString aStr;
            bRet = (rVal >>= aStr);
            if (bRet)
                SetValue(aStr);
            break;
        }
        default:
            return false;
    }

    return bRet;
}

SvxStatusItem* SvxStatusItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SvxStatusItem(*this);
}

SfxPoolItem* SvxStatusItem::CreateDefault()
{
    return new SvxStatusItem(TypedWhichId<SvxStatusItem>(0), OUString(), StatusCategory::NONE);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
