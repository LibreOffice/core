/*************************************************************************
 *
 *  $RCSfile: copydlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:37 $
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


#ifndef _SD_COPYDLG_HXX_
#define _SD_COPYDLG_HXX_

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SVX_DLG_CTRL_HXX //autogen
#include <svx/dlgctrl.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

class SdView;
class XColorTable;

/*************************************************************************
|*
|* Dialog zum Einstellen des Bildschirms
|*
\************************************************************************/
class SdCopyDlg : public SfxModalDialog
{
private:
    FixedText           aFtCopies;
    NumericField        aNumFldCopies;
    FixedText           aFtMoveX;
    MetricField         aMtrFldMoveX;
    FixedText           aFtMoveY;
    MetricField         aMtrFldMoveY;
    FixedText           aFtAngle;
    MetricField         aMtrFldAngle;
    GroupBox            aGrpMovement;

    FixedText           aFtWidth;
    MetricField         aMtrFldWidth;
    FixedText           aFtHeight;
    MetricField         aMtrFldHeight;
    GroupBox            aGrpEnlargement;

    FixedText           aFtStartColor;
    ColorLB             aLbStartColor;
    FixedText           aFtEndColor;
    ColorLB             aLbEndColor;
    GroupBox            aGrpColor;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    ImageButton         aBtnSetViewData;
    PushButton          aBtnSetDefault;

    const SfxItemSet&   rOutAttrs;
    XColorTable*        pColorTab;
    FieldUnit           eUIUnit;
    Fraction            aUIScale;
    SdView*             pView;

    DECL_LINK( SelectColorHdl, void * );
    DECL_LINK( SetViewData, void * );
    DECL_LINK( SetDefault, void * );

public:
            SdCopyDlg( Window* pWindow, const SfxItemSet& rInAttrs,
                        XColorTable* pColTab, SdView* pView );
            ~SdCopyDlg();

    void    GetAttr( SfxItemSet& rOutAttrs );
    DECL_LINK( Reset, void* );
};

#endif // _SD_COPYDLG_HXX_

