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

#ifndef SC_STYLEDLG_HXX
#define SC_STYLEDLG_HXX

#include <sfx2/styledlg.hxx>

//==================================================================

class SfxStyleSheetBase;

class ScStyleDlg : public SfxStyleDialog
{
public:
    ScStyleDlg( Window*             pParent,
                SfxStyleSheetBase&  rStyleBase,
                sal_uInt16          nRscId );

protected:
    virtual void                PageCreated( sal_uInt16 nPageId, SfxTabPage& rTabPage );
    virtual const SfxItemSet*   GetRefreshedSet();

private:
    sal_uInt16 nDlgRsc;

    sal_uInt16 m_nNumberId;
    sal_uInt16 m_nFontId;
    sal_uInt16 m_nFontEffectId;
    sal_uInt16 m_nAlignmentId;
    sal_uInt16 m_nAsianId;
    sal_uInt16 m_nBorderId;
    sal_uInt16 m_nBackgroundId;
    sal_uInt16 m_nProtectId;
    sal_uInt16 m_nPageId;
    sal_uInt16 m_nHeaderId;
    sal_uInt16 m_nFooterId;
    sal_uInt16 m_nSheetId;
};


#endif // SC_STYLEDLG_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
