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
#pragma once

#include <tools/fontenum.hxx>
#include <svl/eitem.hxx>
#include <editeng/editengdllapi.h>
#include <ostream>
#include <i18nutil/scriptchangescanner.hxx>

class EDITENG_DLLPUBLIC SvxScriptHintItem final : public SfxEnumItem<i18nutil::ScriptHintType>
{
protected:
    virtual ItemInstanceManager* getItemInstanceManager() const override;

public:
    static SfxPoolItem* CreateDefault();

    DECLARE_ITEM_TYPE_FUNCTION(SvxScriptHintItem)
    SvxScriptHintItem(const sal_uInt16 nId);
    SvxScriptHintItem(i18nutil::ScriptHintType eType, const sal_uInt16 nId);

    // "pure virtual Methods" from SfxPoolItem + SfxEnumItem
    virtual bool GetPresentation(SfxItemPresentation ePres, MapUnit eCoreMetric,
                                 MapUnit ePresMetric, OUString& rText,
                                 const IntlWrapper&) const override;

    virtual SvxScriptHintItem* Clone(SfxItemPool* pPool = nullptr) const override;
    static OUString GetValueText(i18nutil::ScriptHintType eValue);

    virtual bool QueryValue(css::uno::Any& rVal, sal_uInt8 nMemberId = 0) const override;
    virtual bool PutValue(const css::uno::Any& rVal, sal_uInt8 nMemberId) override;

    // enum cast
    i18nutil::ScriptHintType GetScriptHintValue() const { return GetValue(); }

    void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
