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

#ifndef INCLUDED_SVX_SDGCPITM_HXX
#define INCLUDED_SVX_SDGCPITM_HXX

#include <svx/grfcrop.hxx>
#include <svx/svddef.hxx>


// SdrGrafCropItem -


class SVX_DLLPUBLIC SdrGrafCropItem : public SvxGrfCrop
{
public:

                            TYPEINFO_OVERRIDE();

                            SdrGrafCropItem( sal_Int32 nLeftCrop = 0, sal_Int32 nTopCrop = 0,
                                             sal_Int32 nRightCrop = 0, sal_Int32 nBottomCrop = 0,
                                             sal_uInt16 nWhichId = SDRATTR_GRAFCROP ) :
                                SvxGrfCrop( nLeftCrop, nRightCrop, nTopCrop, nBottomCrop, nWhichId ) {}

    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = NULL ) const override;
    virtual SfxPoolItem*    Create( SvStream& rIn, sal_uInt16 nVer ) const override;
    virtual sal_uInt16          GetVersion( sal_uInt16 nFileVersion ) const override;
};

#endif // INCLUDED_SVX_SDGCPITM_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
