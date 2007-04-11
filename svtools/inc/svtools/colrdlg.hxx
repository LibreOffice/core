/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: colrdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:14:33 $
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

#ifndef _SV_COLRDLG_HXX
#define _SV_COLRDLG_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_COLCTRL_HXX
#include <svtools/colctrl.hxx>
#endif

// ---------------
// - ColorDialog -
// ---------------

class SVT_DLLPUBLIC SvColorDialog : public ModalDialog
{
private:
    ColorMixingControl  maColMixCtrl;
    PushButton      maBtn1;
    PushButton      maBtn2;
    //PushButton        maBtn3;
    //PushButton        maBtn4;
    //FixedText     maFtRGB;
    SvColorControl  maCtlColor;

    // CMYK controls.
    FixedText       maFtCyan;
    MetricField     maNumCyan;
    FixedText       maFtMagenta;
    MetricField     maNumMagenta;
    FixedText       maFtYellow;
    MetricField     maNumYellow;
    FixedText       maFtKey;
    MetricField     maNumKey;

    // RGB controls.
    FixedText       maFtRed;
    NumericField    maNumRed;
    FixedText       maFtGreen;
    NumericField    maNumGreen;
    FixedText       maFtBlue;
    NumericField    maNumBlue;

    // HSL controls.
    FixedText       maFtHue;
    NumericField    maNumHue;
    FixedText       maFtSaturation;
    NumericField    maNumSaturation;
    FixedText       maFtLuminance;
    NumericField    maNumLuminance;

    ColorPreviewControl maCtlPreview;
    ColorPreviewControl maCtlPreviewOld;

    OKButton        maBtnOK;
    CancelButton    maBtnCancel;
    HelpButton      maBtnHelp;

    Color           maColor;

    SVT_DLLPRIVATE void         Initialize();

    DECL_DLLPRIVATE_LINK( ColorModifyHdl, void * );
    DECL_DLLPRIVATE_LINK( ClickBtnHdl, void * );
    DECL_DLLPRIVATE_LINK( ClickMixCtrlHdl, void * );
    DECL_DLLPRIVATE_LINK( SelectMixCtrlHdl, void * );

public:
                    SvColorDialog( Window* pParent );
                    ~SvColorDialog();

    void            SetColor( const Color& rColor );
    const Color&    GetColor() const;

    virtual short   Execute();
};

#endif  // _SV_COLRDLG_HXX
