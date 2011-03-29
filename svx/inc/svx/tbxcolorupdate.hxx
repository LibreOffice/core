/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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

#define TBX_UPDATER_MODE_NONE               0x00
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

    protected:
        void        DrawChar(VirtualDevice&, const Color&);

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
