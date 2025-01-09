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
#ifndef INCLUDED_EDITENG_ORPHITEM_HXX
#define INCLUDED_EDITENG_ORPHITEM_HXX

#include <svl/intitem.hxx>
#include <editeng/editengdllapi.h>

/// An orphan line is a paragraph-starting line that is still on the previous page or column (has
/// future, but no past).
///
/// nL specifies the minimum number of lines, to prevent orphans.
class EDITENG_DLLPUBLIC SvxOrphansItem final : public SfxByteItem
{
public:
    static SfxPoolItem* CreateDefault();
    DECLARE_ITEM_TYPE_FUNCTION(SvxOrphansItem)
    SvxOrphansItem( const sal_uInt8 nL /*= 0*/, const sal_uInt16 nId  );

    // "pure virtual Methods" from SfxPoolItem
    virtual SvxOrphansItem* Clone( SfxItemPool *pPool = nullptr ) const override;

    virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString &rText, const IntlWrapper& ) const override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
