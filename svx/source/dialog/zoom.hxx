/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: zoom.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 15:17:12 $
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
#ifndef _SVX_ZOOM_HXX
#define _SVX_ZOOM_HXX

// include ---------------------------------------------------------------

#include <layout/layout.hxx>
#ifndef _BASEDLGS_HXX //autogen wg. SfxModalDialog
#include <sfx2/basedlgs.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen wg. Radio-/OK-/Cancel-/HelpButton
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen wg. MetricField
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

// define ----------------------------------------------------------------
/* CHINA001
#define ZOOMBTN_OPTIMAL     ((USHORT)0x0001)
#define ZOOMBTN_PAGEWIDTH   ((USHORT)0x0002)
#define ZOOMBTN_WHOLEPAGE   ((USHORT)0x0004)
*/
#ifndef _SVX_ZOOM_HXX
#include "zoom_def.hxx"
#endif
// class SvxZoomDialog ---------------------------------------------------
/*
    {k:\svx\prototyp\dialog\zoom.bmp}

    [Beschreibung]
    Mit diesem Dialog wird ein Zoom-Faktor eingestellt.

    [Items]
    SvxZoomItem <SID_ATTR_ZOOM>
*/

#include <layout/layout-pre.hxx>

class SvxZoomDialog : public SfxModalDialog
{
private:
    FixedLine           aZoomFl;
    RadioButton         aOptimalBtn;
    RadioButton         aWholePageBtn;
    RadioButton         aPageWidthBtn;
    RadioButton         a100Btn;
    RadioButton         aUserBtn;
    MetricField         aUserEdit;

    FixedLine           aViewLayoutFl;
    RadioButton         aAutomaticBtn;
    RadioButton         aSingleBtn;
    RadioButton         aColumnsBtn;
    MetricField         aColumnsEdit;
    CheckBox            aBookModeChk;

    FixedLine           aBottomFl;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    const SfxItemSet&   rSet;
    SfxItemSet*         pOutSet;
    BOOL                bModified;

#ifdef _SVX_ZOOM_CXX
    DECL_LINK( UserHdl, RadioButton* );
    DECL_LINK( SpinHdl, MetricField* );
    DECL_LINK( ViewLayoutUserHdl, RadioButton* );
    DECL_LINK( ViewLayoutSpinHdl, MetricField* );
    DECL_LINK( ViewLayoutCheckHdl, CheckBox* );
    DECL_LINK( OKHdl, Button* );
#endif

public:
    SvxZoomDialog( Window* pParent, const SfxItemSet& rCoreSet );
    ~SvxZoomDialog();

    static USHORT*      GetRanges();
    const SfxItemSet*   GetOutputItemSet() const { return pOutSet; }

    USHORT              GetFactor() const;
    void                SetFactor( USHORT nNewFactor, USHORT nBtnId = 0 );

    void                SetButtonText( USHORT nBtnId, const String& aNewTxt );
    void                HideButton( USHORT nBtnId );
    void                SetLimits( USHORT nMin, USHORT nMax );
    void                SetSpinSize( USHORT nNewSpin );
};

#include <layout/layout-post.hxx>

#endif

