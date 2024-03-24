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

#include <svx/svxdllapi.h>
#include <svl/itempool.hxx>

class XLineAttrSetItem;
class XFillAttrSetItem;

class SVXCORE_DLLPUBLIC SdrItemPool final : public SfxItemPool
{
public:
    SdrItemPool(SfxItemPool* pMaster = nullptr);
    SAL_DLLPRIVATE SdrItemPool(const SdrItemPool& rPool);

private:
    SAL_DLLPRIVATE virtual ~SdrItemPool() override;

public:
    SAL_DLLPRIVATE virtual rtl::Reference<SfxItemPool> Clone() const override;
    SAL_DLLPRIVATE virtual bool GetPresentation(const SfxPoolItem& rItem,
                                 MapUnit ePresentationMetric,
                                 OUString& rText,
                                 const IntlWrapper& rIntlWrapper) const override;

    SAL_DLLPRIVATE static OUString GetItemName(sal_uInt16 nWhich);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
