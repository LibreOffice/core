/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: brkdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 20:02:38 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _BRKDLG_HXX
#define _BRKDLG_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif
#include <bastypes.hxx>

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

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
