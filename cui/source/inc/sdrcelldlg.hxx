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

#ifndef INCLUDED_CUI_SOURCE_INC_SDRCELLDLG_HXX
#define INCLUDED_CUI_SOURCE_INC_SDRCELLDLG_HXX


#include <sfx2/tabdlg.hxx>
#include <svx/xtable.hxx>

class SdrModel;
class SvxFormatCellsDialog : public SfxTabDialog
{
private:
    const SfxItemSet&   mrOutAttrs;

    XColorListRef       mpColorTab;
    XGradientListRef    mpGradientList;
    XHatchListRef       mpHatchingList;
    XBitmapListRef      mpBitmapList;

    sal_uInt16          m_nAreaPageId;
    sal_uInt16          m_nBorderPageId;

public:
    SvxFormatCellsDialog( vcl::Window* pParent, const SfxItemSet* pAttr, SdrModel* pModel );

    virtual void PageCreated( sal_uInt16 nId, SfxTabPage &rPage ) override;

};

#endif // INCLUDED_CUI_SOURCE_INC_SDRCELLDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
