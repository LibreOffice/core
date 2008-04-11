/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: brkdlg.hxx,v $
 * $Revision: 1.5 $
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

#ifndef _BRKDLG_HXX
#define _BRKDLG_HXX

#include <svheader.hxx>
#include <bastypes.hxx>
#include <vcl/dialog.hxx>

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>

class BreakPointDialog : public ModalDialog
{
private:
    ComboBox        aComboBox;
    OKButton        aOKButton;
    CancelButton    aCancelButton;
    PushButton      aNewButton;
    PushButton      aDelButton;
//  PushButton      aShowButton;
    CheckBox        aCheckBox;
    FixedText       aBrkText;
    FixedText       aPassText;
    NumericField    aNumericField;

    BreakPointList & m_rOriginalBreakPointList;
    BreakPointList m_aModifiedBreakPointList;

protected:
    void            CheckButtons();
    DECL_LINK( CheckBoxHdl, CheckBox * );
    DECL_LINK( ComboBoxHighlightHdl, ComboBox * );
    DECL_LINK( EditModifyHdl, Edit * );
    DECL_LINK( ButtonHdl, Button * );
    void            UpdateFields( BreakPoint* pBrk );
    BreakPoint*     GetSelectedBreakPoint();


public:
            BreakPointDialog( Window* pParent, BreakPointList& rBrkList );

    void    SetCurrentBreakPoint( BreakPoint* pBrk );
};

#endif  // _BRKDLG_HXX
