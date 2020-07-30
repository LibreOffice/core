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

#include <numberingtypelistbox.hxx>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/text/DefaultNumberingProvider.hpp>
#include <com/sun/star/text/XDefaultNumberingProvider.hpp>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/XNumberingTypeInfo.hpp>
#include <editeng/numitem.hxx>
#include <svx/strarray.hxx>
#include <osl/diagnose.h>

using namespace com::sun::star;

struct SwNumberingTypeListBox_Impl
{
    uno::Reference<text::XNumberingTypeInfo> xInfo;
};

SwNumberingTypeListBox::SwNumberingTypeListBox(std::unique_ptr<weld::ComboBox> pWidget)
    : m_xWidget(std::move(pWidget))
    , m_xImpl(new SwNumberingTypeListBox_Impl)
{
    uno::Reference<uno::XComponentContext>          xContext( ::comphelper::getProcessComponentContext() );
    uno::Reference<text::XDefaultNumberingProvider> xDefNum = text::DefaultNumberingProvider::create(xContext);
    m_xImpl->xInfo.set(xDefNum, uno::UNO_QUERY);
}

SwNumberingTypeListBox::~SwNumberingTypeListBox()
{
}

void SwNumberingTypeListBox::Reload(SwInsertNumTypes nTypeFlags)
{
    m_xWidget->clear();
    uno::Sequence<sal_Int16> aTypes;
    if (nTypeFlags & SwInsertNumTypes::Extended)
    {
        if (m_xImpl->xInfo.is())
            aTypes = m_xImpl->xInfo->getSupportedNumberingTypes();
    }

    for(size_t i = 0; i < SvxNumberingTypeTable::Count(); i++)
    {
        sal_IntPtr nValue = SvxNumberingTypeTable::GetValue(i);
        bool bInsert = true;
        int nPos = -1;
        switch(nValue)
        {
            case  style::NumberingType::NUMBER_NONE:
                bInsert = bool(nTypeFlags & SwInsertNumTypes::NoNumbering);
                nPos = 0;

                break;
            case  style::NumberingType::CHAR_SPECIAL:
                bInsert = false;

                break;
            case  style::NumberingType::PAGE_DESCRIPTOR:
                bInsert = false;

                break;
            case  style::NumberingType::BITMAP:
                bInsert = false;

                break;
            case  style::NumberingType::BITMAP | LINK_TOKEN:
                bInsert = false;

                break;
            default:
                if (nValue >  style::NumberingType::CHARS_LOWER_LETTER_N)
                {
                    // Insert only if offered by i18n framework per configuration.
                    bInsert = std::find(aTypes.begin(), aTypes.end(), nValue) != aTypes.end();
                }
        }
        if (bInsert)
        {
            OUString sId(OUString::number(nValue));
            m_xWidget->insert(nPos, SvxNumberingTypeTable::GetString(i), &sId, nullptr, nullptr);
        }
    }
    if (!(nTypeFlags & SwInsertNumTypes::Extended))
        return;

    for (sal_Int16 nCurrent : aTypes)
    {
        if (nCurrent > style::NumberingType::CHARS_LOWER_LETTER_N)
        {
            if (m_xWidget->find_id(OUString::number(nCurrent)) == -1)
            {
                m_xWidget->append(OUString::number(nCurrent), m_xImpl->xInfo->getNumberingIdentifier(nCurrent));
            }
        }
    }
    m_xWidget->set_active(0);
}

SvxNumType SwNumberingTypeListBox::GetSelectedNumberingType() const
{
    SvxNumType nRet = SVX_NUM_CHARS_UPPER_LETTER;
    int nSelPos = m_xWidget->get_active();
    if (nSelPos != -1)
        nRet = static_cast<SvxNumType>(m_xWidget->get_id(nSelPos).toInt32());
#if OSL_DEBUG_LEVEL > 0
    else
        OSL_FAIL("NumberingTypeListBox not selected");
#endif
    return nRet;
}

bool SwNumberingTypeListBox::SelectNumberingType(SvxNumType nType)
{
    int nPos = m_xWidget->find_id(OUString::number(nType));
    m_xWidget->set_active(nPos);
    return nPos != -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
