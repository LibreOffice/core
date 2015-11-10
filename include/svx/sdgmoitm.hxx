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

#ifndef INCLUDED_SVX_SDGMOITM_HXX
#define INCLUDED_SVX_SDGMOITM_HXX

#include <svtools/grfmgr.hxx>
#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>


// SdrGrafModeItem -


class SVX_DLLPUBLIC SdrGrafModeItem : public SfxEnumItem
{
public:

                            TYPEINFO_OVERRIDE();

                            SdrGrafModeItem( GraphicDrawMode eMode = GRAPHICDRAWMODE_STANDARD ) : SfxEnumItem( SDRATTR_GRAFMODE, (sal_uInt16)eMode ) {}
                            SdrGrafModeItem( SvStream& rIn ) : SfxEnumItem( SDRATTR_GRAFMODE, rIn ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = nullptr ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
    virtual sal_uInt16          GetValueCount() const override;
    GraphicDrawMode         GetValue() const { return (GraphicDrawMode) SfxEnumItem::GetValue(); }

    virtual OUString   GetValueTextByPos( sal_uInt16 nPos ) const override;
    virtual bool GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreMetric,
                                                 SfxMapUnit ePresMetric,
                                                 OUString& rText, const IntlWrapper * = nullptr) const override;

};

#endif // INCLUDED_SVX_SDGMOITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
