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

#include <editeng/eeitem.hxx>

#include <editeng/measfld.hxx>
#include <svx/svdotext.hxx>
#include <svx/svdfield.hxx>

static bool bInit = false;

// Do not remove this, it is still used in src536a!
void SdrRegisterFieldClasses()
{
    if ( !bInit )
    {
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SdrMeasureField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxHeaderField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxFooterField);
        SvxFieldItem::GetClassManager().SV_CLASS_REGISTER(SvxDateTimeField);
        bInit = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////// */

bool SdrTextObj::CalcFieldValue(const SvxFieldItem& /*rField*/, sal_Int32 /*nPara*/, sal_uInt16 /*nPos*/,
    bool /*bEdit*/, Color*& /*rpTxtColor*/, Color*& /*rpFldColor*/, OUString& /*rRet*/) const
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
