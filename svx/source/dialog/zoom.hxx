/*************************************************************************
 *
 *  $RCSfile: zoom.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-02-03 19:02:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SVX_ZOOM_HXX
#define _SVX_ZOOM_HXX

// include ---------------------------------------------------------------

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

class SvxZoomDialog : public SfxModalDialog
{
private:
    FixedLine           aZoomFl;
    RadioButton         aWholePageBtn;
    RadioButton         aPageWidthBtn;
    RadioButton         aOptimalBtn;
    RadioButton         a200Btn;
    RadioButton         a150Btn;
    RadioButton         a100Btn;
    RadioButton         a75Btn;
    RadioButton         a50Btn;
    RadioButton         aUserBtn;
    MetricField         aUserEdit;
    OKButton            aOKBtn;
    CancelButton        aCancelBtn;
    HelpButton          aHelpBtn;

    const SfxItemSet&   rSet;
    SfxItemSet*         pOutSet;
    BOOL                bModified;

#ifdef _SVX_ZOOM_CXX
    DECL_LINK( UserHdl, RadioButton* );
    DECL_LINK( SpinHdl, MetricField* );
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


#endif

