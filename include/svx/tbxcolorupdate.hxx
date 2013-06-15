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

#ifndef SVX_TBXCOLORUPDATE_HXX
#define SVX_TBXCOLORUPDATE_HXX

#include <svx/svxdllapi.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>

class ToolBox;
class VirtualDevice;

//........................................................................
namespace svx
{
//........................................................................

#define TBX_UPDATER_MODE_CHAR_COLOR         0x01
#define TBX_UPDATER_MODE_CHAR_BACKGROUND    0x02
#define TBX_UPDATER_MODE_CHAR_COLOR_NEW     0x03

    //====================================================================
    //= ToolboxButtonColorUpdater
    //====================================================================
    /** helper class to update a color in a toolbox button image

        formerly known as SvxTbxButtonColorUpdater_Impl, residing in svx/source/tbxctrls/colorwindow.hxx.
    */
    class SVX_DLLPUBLIC ToolboxButtonColorUpdater
    {
    public:
                    ToolboxButtonColorUpdater( sal_uInt16   nSlotId,
                                                sal_uInt16   nTbxBtnId,
                                                ToolBox* ptrTbx,
                                                sal_uInt16   nMode = 0 );
                    ~ToolboxButtonColorUpdater();

        void        Update( const Color& rColor );

    private:
        sal_uInt16      mnDrawMode;
        sal_uInt16      mnBtnId;
        sal_uInt16      mnSlotId;
        ToolBox*    mpTbx;
        Color       maCurColor;
        Rectangle   maUpdRect;
        Size        maBmpSize;
        sal_Bool        mbWasHiContrastMode;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_TBXCOLORUPDATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
