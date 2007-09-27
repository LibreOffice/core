/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tautofmt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-27 12:12:20 $
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
#ifndef SW_TAUTOFMT_HXX
#define SW_TAUTOFMT_HXX
#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _MOREBTN_HXX //autogen
#include <vcl/morebtn.hxx>
#endif

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

class SwTableAutoFmt;
class AutoFmtPreview;
class SwTableAutoFmtTbl;
class SwWrtShell;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

class SwAutoFormatDlg : public SfxModalDialog
{
    FixedLine       aFlFormat;
    ListBox         aLbFormat;
    CheckBox        aBtnNumFormat;
    CheckBox        aBtnBorder;
    CheckBox        aBtnFont;
    CheckBox        aBtnPattern;
    CheckBox        aBtnAlignment;
    FixedLine       aFlFormats;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    PushButton      aBtnRename;
    MoreButton      aBtnMore;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRenameTitle;
    String          aStrInvalidFmt;
    AutoFmtPreview* pWndPreview;

    //------------------------
    SwWrtShell*             pShell;
    SwTableAutoFmtTbl*      pTableTbl;
    BYTE                    nIndex;
    BYTE                    nDfltStylePos;
    BOOL                    bCoreDataChanged : 1;
    BOOL                    bSetAutoFmt : 1;


    void Init( const SwTableAutoFmt* pSelFmt );
    void UpdateChecks( const SwTableAutoFmt&, BOOL bEnableBtn );
    //------------------------
    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( OkHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( RenameHdl, void * );
    DECL_LINK( SelFmtHdl, void * );

public:
    SwAutoFormatDlg( Window* pParent, SwWrtShell* pShell,
                        BOOL bSetAutoFmt = TRUE,
                        const SwTableAutoFmt* pSelFmt = 0 );
    virtual ~SwAutoFormatDlg();

    void FillAutoFmtOfIndex( SwTableAutoFmt*& rToFill ) const;
};


#endif // SW_AUTOFMT_HXX

