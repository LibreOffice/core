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

#include <vcl/GraphicAttributes.hxx>
#include <svl/eitem.hxx>
#include <svx/svddef.hxx>
#include <svx/svxdllapi.h>

// MSVC hack:
class SAL_DLLPUBLIC_RTTI SdrGrafModeItem_Base: public SfxEnumItem<GraphicDrawMode> {
protected:
    SdrGrafModeItem_Base(GraphicDrawMode eMode):
        SfxEnumItem(SDRATTR_GRAFMODE, eMode) {}
};

class SVXCORE_DLLPUBLIC SdrGrafModeItem final : public SdrGrafModeItem_Base
{
public:
                            SdrGrafModeItem( GraphicDrawMode eMode = GraphicDrawMode::Standard ) : SdrGrafModeItem_Base( eMode ) {}

    SAL_DLLPRIVATE virtual SdrGrafModeItem* Clone( SfxItemPool* pPool = nullptr ) const override;
    SAL_DLLPRIVATE virtual sal_uInt16      GetValueCount() const override;

    SAL_DLLPRIVATE static OUString         GetValueTextByPos( sal_uInt16 nPos );
    SAL_DLLPRIVATE virtual bool GetPresentation( SfxItemPresentation ePres,
                                  MapUnit eCoreMetric,
                                  MapUnit ePresMetric,
                                  OUString& rText, const IntlWrapper& ) const override;
};

#endif // INCLUDED_SVX_SDGMOITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
