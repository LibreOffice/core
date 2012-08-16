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

#ifndef SC_TABPAGES_HXX
#define SC_TABPAGES_HXX

#include <vcl/group.hxx>
#include <svtools/stdctrl.hxx>
#include <sfx2/tabdlg.hxx>

//========================================================================

class ScTabPageProtection : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rAttrSet );
    static  sal_uInt16*     GetRanges       ();
    virtual sal_Bool        FillItemSet     ( SfxItemSet& rCoreAttrs );
    virtual void        Reset           ( const SfxItemSet& );

protected:
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
                ScTabPageProtection( Window*            pParent,
                                     const SfxItemSet&  rCoreAttrs );
                ~ScTabPageProtection();

private:
    FixedLine   aFlProtect;
    TriStateBox aBtnHideCell;
    TriStateBox aBtnProtect;
    TriStateBox aBtnHideFormula;
    FixedInfo   aTxtHint;

    FixedLine   aFlPrint;
    TriStateBox aBtnHidePrint;
    FixedInfo   aTxtHint2;

                                    // current status:
    sal_Bool        bTriEnabled;        //  if before - DontCare
    sal_Bool        bDontCare;          //  all in  TriState
    sal_Bool        bProtect;           //  secure individual settings for TriState
    sal_Bool        bHideForm;
    sal_Bool        bHideCell;
    sal_Bool        bHidePrint;

    // Handler:
    DECL_LINK( ButtonClickHdl, TriStateBox* pBox );
    void        UpdateButtons();
};



#endif // SC_TABPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
