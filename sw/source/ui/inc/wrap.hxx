/*************************************************************************
 *
 *  $RCSfile: wrap.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:43 $
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
#ifndef _WRAP_HXX
#define _WRAP_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _IMAGEBTN_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

class Window;
class SfxItemSet;
class SwWrtShell;

class SwWrapDlg : public SfxSingleTabDialog
{
    SwWrtShell*         pWrtShell;

public:
     SwWrapDlg(Window* pParent, SfxItemSet& rSet, SwWrtShell* pSh, BOOL bDrawMode);
    ~SwWrapDlg();

    inline SwWrtShell*  GetWrtShell()   { return pWrtShell; }
};


/*--------------------------------------------------------------------
    Beschreibung:   Umlauf-TabPage
 --------------------------------------------------------------------*/

class SwWrapTabPage: public SfxTabPage
{
    // WRAPPING
    ImageRadioButton    aNoWrapRB;
    ImageRadioButton    aWrapLeftRB;
    ImageRadioButton    aWrapRightRB;
    ImageRadioButton    aWrapParallelRB;
    ImageRadioButton    aWrapThroughRB;
    ImageRadioButton    aIdealWrapRB;
    GroupBox            aWrapBoxC;

    CheckBox            aWrapAnchorOnlyCB;
    CheckBox            aWrapTransparentCB;
    CheckBox            aWrapOutlineCB;
    CheckBox            aWrapOutsideCB;
    GroupBox            aOptionsGB;

    // MARGIN
    FixedText           aLeftMarginFT;
    MetricField         aLeftMarginED;
    FixedText           aRightMarginFT;
    MetricField         aRightMarginED;
    FixedText           aTopMarginFT;
    MetricField         aTopMarginED;
    FixedText           aBottomMarginFT;
    MetricField         aBottomMarginED;
    GroupBox            aMarginGB;

    ImageList           aWrapIL;

    USHORT              nOldLeftMargin;
    USHORT              nOldRightMargin;
    USHORT              nOldUpperMargin;
    USHORT              nOldLowerMargin;

    USHORT              nAnchorId;
    USHORT              nHtmlMode;

    Size aFrmSize;
    SwWrtShell*         pWrtSh;

    BOOL bFormat;
    BOOL bNew;
    BOOL bHtmlMode;
    BOOL bDrawMode;
    BOOL bContourImage;

    SwWrapTabPage(Window *pParent, const SfxItemSet &rSet);
    ~SwWrapTabPage();

    void            EnableModes(const SfxItemSet& rSet);
    virtual void    ActivatePage(const SfxItemSet& rSet);
    virtual int     DeactivatePage(SfxItemSet *pSet);

    DECL_LINK( RangeModifyHdl, MetricField * );
    DECL_LINK( WrapTypeHdl, ImageRadioButton * );
    DECL_LINK( ContourHdl, CheckBox * );

public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);

    virtual BOOL    FillItemSet(SfxItemSet &rSet);
    virtual void    Reset(const SfxItemSet &rSet);

    static USHORT*  GetRanges();
    inline void     SetNewFrame(BOOL bNewFrame) { bNew = bNewFrame; }
    inline void     SetFormatUsed(BOOL bFmt, BOOL bDrw) { bFormat = bFmt;
                                                            bDrawMode = bDrw; }
    inline void     SetShell(SwWrtShell* pSh) { pWrtSh = pSh; }
};

#endif


