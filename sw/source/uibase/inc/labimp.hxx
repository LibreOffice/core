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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_LABIMP_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_LABIMP_HXX

#include <sfx2/tabdlg.hxx>
#include "label.hxx"
#include "labimg.hxx"

#define GETFLDVAL(rField)         (rField).Denormalize((rField).GetValue(FieldUnit::TWIP))
#define SETFLDVAL(rField, lValue) (rField).SetValue((rField).Normalize(lValue), FieldUnit::TWIP)

inline int getfldval(const weld::MetricSpinButton& rField)
{
    return rField.denormalize(rField.get_value(FieldUnit::TWIP));
}

inline void setfldval(weld::MetricSpinButton& rField, int lValue)
{
    rField.set_value(rField.normalize(lValue), FieldUnit::TWIP);
}

#endif // INCLUDED_SW_SOURCE_UIBASE_INC_LABIMP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
