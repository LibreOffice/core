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

#ifndef INCLUDED_SVX_TBXCOLORUPDATE_HXX
#define INCLUDED_SVX_TBXCOLORUPDATE_HXX

#include <svx/svxdllapi.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;
class VirtualDevice;


namespace svx
{



    //= ToolboxButtonColorUpdater

    /** helper class to update a color in a toolbox button image

        formerly known as SvxTbxButtonColorUpdater_Impl, residing in svx/source/tbxctrls/colorwindow.hxx.
    */
    class SVX_DLLPUBLIC ToolboxButtonColorUpdater
    {
    public:
                    ToolboxButtonColorUpdater( sal_uInt16   nSlotId,
                                               sal_uInt16   nTbxBtnId,
                                               ToolBox*     ptrTbx);
                    ~ToolboxButtonColorUpdater();

        void        Update( const Color& rColor );

    private:
        sal_uInt16      mnBtnId;
        sal_uInt16      mnSlotId;
        VclPtr<ToolBox> mpTbx;
        Color       maCurColor;
        Rectangle   maUpdRect;
        Size        maBmpSize;
        bool        mbWasHiContrastMode;
    };


}


#endif // INCLUDED_SVX_TBXCOLORUPDATE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
