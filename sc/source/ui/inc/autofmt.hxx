/*************************************************************************
 *
 *  $RCSfile: autofmt.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:44:57 $
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

#ifndef SC_AUTOFMT_HXX
#define SC_AUTOFMT_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef SV_MOREBTN_HXX
#include <vcl/morebtn.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif


//------------------------------------------------------------------------

class ScAutoFormat;
class ScAutoFormatData;
class SvxBoxItem;
class SvxBorderLine;
class AutoFmtPreview; // s.u.
class SvNumberFormatter;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

class ScAutoFormatDlg : public ModalDialog
{
public:
            ScAutoFormatDlg( Window*                    pParent,
                             ScAutoFormat*              pAutoFormat,
                             const ScAutoFormatData*    pSelFormatData );
            ~ScAutoFormatDlg();

    USHORT GetIndex() const { return nIndex; }
    String GetCurrFormatName();

private:
    FixedText       aFtFormat;
    ListBox         aLbFormat;
    AutoFmtPreview* pWndPreview;
    GroupBox        aGbPreview;
    PushButton      aBtnAdd;
    PushButton      aBtnRemove;
    PushButton      aBtnRename;
    CheckBox        aBtnNumFormat;
    CheckBox        aBtnBorder;
    CheckBox        aBtnFont;
    CheckBox        aBtnPattern;
    CheckBox        aBtnAlignment;
    CheckBox        aBtnAdjust;
    GroupBox        aGbFormat;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    MoreButton      aBtnMore;
    String          aStrTitle;
    String          aStrLabel;
    String          aStrClose;
    String          aStrDelTitle;
    String          aStrDelMsg;
    String          aStrRename;

    //------------------------
    ScAutoFormat*           pFormat;
    const ScAutoFormatData* pSelFmtData;
    USHORT                  nIndex;
    BOOL                    bCoreDataChanged;
    BOOL                    bFmtInserted;

    void Init           ();
    void UpdateChecks   ();
    //------------------------
    DECL_LINK( CheckHdl, Button * );
    DECL_LINK( AddHdl, void * );
    DECL_LINK( RemoveHdl, void * );
    DECL_LINK( SelFmtHdl, void * );
    DECL_LINK( CloseHdl, PushButton * );
    DECL_LINK( DblClkHdl, void * );
    DECL_LINK( RenameHdl, void *);

};

//========================================================================

class AutoFmtPreview : public Window
{
public:
            AutoFmtPreview( Window* pParent, const ResId& rRes );
            ~AutoFmtPreview();

    void NotifyChange( ScAutoFormatData* pNewData );

protected:
    virtual void Paint( const Rectangle& rRect );

private:
    ScAutoFormatData*   pCurData;
    VirtualDevice       aVD;
    BOOL                bFitWidth;
    static USHORT       aFmtMap[25];        // Zuordnung: Zelle->Format
    Rectangle           aCellArray[25];     // Position und Groesse der Zellen
    SvxBoxItem*         aLinePtrArray[49];  // LinienAttribute
    Size                aPrvSize;
    const USHORT        nLabelColWidth;
    const USHORT        nDataColWidth1;
    const USHORT        nDataColWidth2;
    const USHORT        nRowHeight;
    const String        aStrJan;
    const String        aStrFeb;
    const String        aStrMar;
    const String        aStrNorth;
    const String        aStrMid;
    const String        aStrSouth;
    const String        aStrSum;
    SvNumberFormatter*  pNumFmt;
    //-------------------------------------------
    void    Init            ();
    void    DoPaint         ( const Rectangle& rRect );
    void    CalcCellArray   ( BOOL bFitWidth );
    void    CalcLineMap     ();
    void    PaintCells      ();
    void    DrawBackground  ( USHORT nIndex );
    void    DrawFrame       ( USHORT nIndex );
    void    DrawString      ( USHORT nIndex );
    void    MakeFont        ( USHORT nIndex, Font& rFont );
    String  MakeNumberString( String cellString, BOOL bAddDec );
    void    DrawFrameLine   ( const SvxBorderLine&  rLineD,
                              Point                 from,
                              Point                 to,
                              BOOL                  bHorizontal,
                              const SvxBorderLine&  rLineLT,
                              const SvxBorderLine&  rLineL,
                              const SvxBorderLine&  rLineLB,
                              const SvxBorderLine&  rLineRT,
                              const SvxBorderLine&  rLineR,
                              const SvxBorderLine&  rLineRB );
    void    CheckPriority   ( USHORT            nCurLine,
                              AutoFmtLine       eLine,
                              SvxBorderLine&    rLine );
    void    GetLines        ( USHORT nIndex, AutoFmtLine eLine,
                              SvxBorderLine&    rLineD,
                              SvxBorderLine&    rLineLT,
                              SvxBorderLine&    rLineL,
                              SvxBorderLine&    rLineLB,
                              SvxBorderLine&    rLineRT,
                              SvxBorderLine&    rLineR,
                              SvxBorderLine&    rLineRB );
};

#endif // SC_AUTOFMT_HXX


