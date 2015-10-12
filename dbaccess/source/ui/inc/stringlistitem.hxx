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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_STRINGLISTITEM_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_STRINGLISTITEM_HXX

#include <svl/poolitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <rtl/ustring.hxx>

namespace dbaui
{

// OStringListItem
/** <type>SfxPoolItem</type> which transports a sequence of <type scope="rtl">OUString</type>'s
*/
class OStringListItem : public SfxPoolItem
{
    css::uno::Sequence< OUString >      m_aList;

public:
    TYPEINFO_OVERRIDE();
    OStringListItem(sal_Int16 nWhich, const css::uno::Sequence< OUString >& _rList);
    OStringListItem(const OStringListItem& _rSource);

    virtual bool             operator==(const SfxPoolItem& _rItem) const override;
    virtual SfxPoolItem*     Clone(SfxItemPool* _pPool = NULL) const override;

    css::uno::Sequence< OUString >  getList() const { return m_aList; }
};

}   // namespace dbaui

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_STRINGLISTITEM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
