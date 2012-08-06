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

#ifndef _SD_PRNTOPTS_HXX
#define _SD_PRNTOPTS_HXX

#include <vcl/group.hxx>

#include <vcl/button.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

class SdModule;
class SdPrintOptions : public SfxTabPage
{
 friend class SdModule;

private:
    FixedLine           aGrpPrint;
    CheckBox            aCbxDraw;
    CheckBox            aCbxNotes;
    CheckBox            aCbxHandout;
    CheckBox            aCbxOutline;

    FixedLine           aSeparator1FL;
    FixedLine           aGrpOutput;
    RadioButton         aRbtColor;
    RadioButton         aRbtGrayscale;
    RadioButton         aRbtBlackWhite;

    FixedLine           aGrpPrintExt;
    CheckBox            aCbxPagename;
    CheckBox            aCbxDate;
    CheckBox            aCbxTime;
    CheckBox            aCbxHiddenPages;

    FixedLine           aSeparator2FL;
    FixedLine           aGrpPageoptions;
    RadioButton         aRbtDefault;
    RadioButton         aRbtPagesize;
    RadioButton         aRbtPagetile;
    RadioButton         aRbtBooklet;
    CheckBox            aCbxFront;
    CheckBox            aCbxBack;

    CheckBox            aCbxPaperbin;

    const SfxItemSet&   rOutAttrs;

    DECL_LINK( ClickCheckboxHdl, CheckBox * );
    DECL_LINK( ClickBookletHdl, void * );

    void updateControls();

    using OutputDevice::SetDrawMode;
public:
            SdPrintOptions( Window* pParent, const SfxItemSet& rInAttrs);
            ~SdPrintOptions();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );

    virtual sal_Bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    void    SetDrawMode();
    virtual void        PageCreated (SfxAllItemSet aSet);
};



#endif // _SD_PRNTOPTS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
