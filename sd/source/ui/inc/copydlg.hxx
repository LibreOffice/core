/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: copydlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:41:18 $
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

#ifndef SD_COPY_DLG_HXX
#define SD_COPY_DLG_HXX

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

class XColorTable;

namespace sd {

class View;

/*************************************************************************
|*
|* Dialog zum Einstellen des Bildschirms
|*
\************************************************************************/
class CopyDlg
    : public SfxModalDialog
{
public:
    CopyDlg( ::Window* pWindow, const SfxItemSet& rInAttrs,
        XColorTable* pColTab, ::sd::View* pView );
    ~CopyDlg();

    void    GetAttr( SfxItemSet& rOutAttrs );
    DECL_LINK( Reset, void* );

private:
    FixedText           maFtCopies;
    NumericField        maNumFldCopies;
    ImageButton         maBtnSetViewData;

    FixedText           maFtMoveX;
    MetricField         maMtrFldMoveX;
    FixedText           maFtMoveY;
    MetricField         maMtrFldMoveY;
    FixedText           maFtAngle;
    MetricField         maMtrFldAngle;
    FixedLine           maGrpMovement;

    FixedText           maFtWidth;
    MetricField         maMtrFldWidth;
    FixedText           maFtHeight;
    MetricField         maMtrFldHeight;
    FixedLine           maGrpEnlargement;

    FixedText           maFtStartColor;
    ColorLB             maLbStartColor;
    FixedText           maFtEndColor;
    ColorLB             maLbEndColor;
    FixedLine           maGrpColor;

    OKButton            maBtnOK;
    CancelButton        maBtnCancel;
    HelpButton          maBtnHelp;
    PushButton          maBtnSetDefault;

    const SfxItemSet&   mrOutAttrs;
    XColorTable*        mpColorTab;
    Fraction            maUIScale;
    ::sd::View*         mpView;

    DECL_LINK( SelectColorHdl, void * );
    DECL_LINK( SetViewData, void * );
    DECL_LINK( SetDefault, void * );
};

} // end of namespace sd

#endif

