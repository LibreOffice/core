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

#ifndef _SVX_XSFLCLIT_HXX
#define _SVX_XSFLCLIT_HXX

#include <svx/xcolit.hxx>

//------------------------------
// class XSecondaryFillColorItem
//------------------------------
class SVX_DLLPUBLIC XSecondaryFillColorItem : public XColorItem
{
public:
            TYPEINFO();
            SVX_DLLPRIVATE XSecondaryFillColorItem() {}
            XSecondaryFillColorItem(const OUString& rName, const Color& rTheColor);
            SVX_DLLPRIVATE XSecondaryFillColorItem(SvStream& rIn);

    SVX_DLLPRIVATE virtual SfxPoolItem*    Clone(SfxItemPool* pPool = 0) const;
    SVX_DLLPRIVATE virtual SfxPoolItem*    Create(SvStream& rIn, sal_uInt16 nVer) const;

    SVX_DLLPRIVATE virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit eCoreMetric,
                                    SfxMapUnit ePresMetric,
                                    OUString &rText, const IntlWrapper * = 0 ) const;

    SVX_DLLPRIVATE virtual sal_uInt16 GetVersion( sal_uInt16 nFileFormatVersion ) const;

};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
