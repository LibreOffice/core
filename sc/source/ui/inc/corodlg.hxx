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

#ifndef SC_CORODLG_HXX
#define SC_CORODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>

#include "sc.hrc"
#include "scresid.hxx"

//------------------------------------------------------------------------

class ScColRowLabelDlg : public ModalDialog
{
public:
            ScColRowLabelDlg( Window* pParent,
                              sal_Bool bCol = false,
                              sal_Bool bRow = false )
                : ModalDialog( pParent, ScResId( RID_SCDLG_CHARTCOLROW ) ),
                  aFlColRow  ( this, ScResId(6) ),
                  aBtnRow    ( this, ScResId(2) ),
                  aBtnCol    ( this, ScResId(1) ),
                  aBtnOk     ( this, ScResId(3) ),
                  aBtnCancel ( this, ScResId(4) ),
                  aBtnHelp   ( this, ScResId(5) )
                {
                    FreeResource();
                    aBtnCol.Check( bCol );
                    aBtnRow.Check( bRow );
                }

    sal_Bool IsCol() { return aBtnCol.IsChecked(); }
    sal_Bool IsRow() { return aBtnRow.IsChecked(); }

private:
    FixedLine       aFlColRow;
    CheckBox        aBtnRow;
    CheckBox        aBtnCol;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
};


#endif



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
