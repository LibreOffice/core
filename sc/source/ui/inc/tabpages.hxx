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

                                    // aktueller Status:
    sal_Bool        bTriEnabled;        //  wenn vorher Dont-Care
    sal_Bool        bDontCare;          //  alles auf TriState
    sal_Bool        bProtect;           //  einzelne Einstellungen ueber TriState sichern
    sal_Bool        bHideForm;
    sal_Bool        bHideCell;
    sal_Bool        bHidePrint;

    // Handler:
    DECL_LINK( ButtonClickHdl, TriStateBox* pBox );
    void        UpdateButtons();
};



#endif // SC_TABPAGES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
