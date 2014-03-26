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

#ifndef INCLUDED_SD_SOURCE_UI_INC_PRNTOPTS_HXX
#define INCLUDED_SD_SOURCE_UI_INC_PRNTOPTS_HXX

#include <vcl/group.hxx>

#include <vcl/button.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>

class SdModule;
class SdPrintOptions : public SfxTabPage
{
 friend class SdModule;

private:
    VclFrame            *m_pFrmContent;
    CheckBox            *m_pCbxDraw;
    CheckBox            *m_pCbxNotes;
    CheckBox            *m_pCbxHandout;
    CheckBox            *m_pCbxOutline;
    RadioButton         *m_pRbtColor;
    RadioButton         *m_pRbtGrayscale;
    RadioButton         *m_pRbtBlackWhite;
    CheckBox            *m_pCbxPagename;
    CheckBox            *m_pCbxDate;
    CheckBox            *m_pCbxTime;
    CheckBox            *m_pCbxHiddenPages;
    RadioButton         *m_pRbtDefault;
    RadioButton         *m_pRbtPagesize;
    RadioButton         *m_pRbtPagetile;
    RadioButton         *m_pRbtBooklet;
    CheckBox            *m_pCbxFront;
    CheckBox            *m_pCbxBack;
    CheckBox            *m_pCbxPaperbin;

    const SfxItemSet&   rOutAttrs;

    DECL_LINK( ClickCheckboxHdl, CheckBox * );
    DECL_LINK( ClickBookletHdl, void * );

    void updateControls();

    using OutputDevice::SetDrawMode;
public:
            SdPrintOptions( Window* pParent, const SfxItemSet& rInAttrs);
            ~SdPrintOptions();

    static  SfxTabPage* Create( Window*, const SfxItemSet& );

    virtual bool FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    void    SetDrawMode();
    virtual void        PageCreated (SfxAllItemSet aSet);
};



#endif // INCLUDED_SD_SOURCE_UI_INC_PRNTOPTS_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
