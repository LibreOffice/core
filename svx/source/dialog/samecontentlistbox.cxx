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

#include <svx/dialmgr.hxx>
#include <svx/samecontentlistbox.hxx>
#include <samecontent.hrc>

namespace SameContentListBox
{
    void Fill(weld::ComboBox& rComboBox)
    {
        rComboBox.clear();
        for (size_t i = 0; i < SAL_N_ELEMENTS(RID_SVXSTRARY_SAMECONTENT); ++i)
        {
            OUString aStr = SvxResId(RID_SVXSTRARY_SAMECONTENT[i].first);
            sal_uInt32 nData = RID_SVXSTRARY_SAMECONTENT[i].second;
            rComboBox.append(OUString::number(nData), aStr);
        }
        rComboBox.set_active(0);
        rComboBox.set_size_request(150, -1);
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
