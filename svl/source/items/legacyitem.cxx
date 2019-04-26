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

#include <svl/legacyitem.hxx>
#include <tools/stream.hxx>
#include <svl/eitem.hxx>
#include <svl/cintitem.hxx>

namespace legacy
{
    namespace SfxBool
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(SfxBoolItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            bool tmp(false);
            rStrm.ReadCharAsBool(tmp);
            rItem.SetValue(tmp);
        }

        SvStream& Store(const SfxBoolItem& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteBool(rItem.GetValue()); // not bool for serialization!
            return rStrm;
        }
    }
    namespace CntInt32
    {
        sal_uInt16 GetVersion(sal_uInt16)
        {
            return 0;
        }

        void Create(CntInt32Item& rItem, SvStream& rStrm, sal_uInt16)
        {
            sal_Int32 tmp(0);
            rStrm.ReadInt32(tmp);
            rItem.SetValue(tmp);
        }

        SvStream& Store(const CntInt32Item& rItem, SvStream& rStrm, sal_uInt16)
        {
            rStrm.WriteInt32(rItem.GetValue());
            return rStrm;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
