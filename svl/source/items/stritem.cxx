/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <svl/stritem.hxx>

//

//

TYPEINIT1_AUTOFACTORY(SfxStringItem, CntUnencodedStringItem)


SfxStringItem::SfxStringItem(sal_uInt16 which, SvStream & rStream):
    CntUnencodedStringItem(which)
{
    SetValue(readByteString(rStream));
}



SfxPoolItem * SfxStringItem::Create(SvStream & rStream, sal_uInt16) const
{
    return new SfxStringItem(Which(), rStream);
}


SvStream & SfxStringItem::Store(SvStream & rStream, sal_uInt16) const
{
    writeByteString(rStream, GetValue());
    return rStream;
}


SfxPoolItem * SfxStringItem::Clone(SfxItemPool *) const
{
    return new SfxStringItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
