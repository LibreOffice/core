/*************************************************************************
 *
 *  $RCSfile: autofmt.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 17:02:22 $
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
#ifndef SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef SV_MOREBTN_HXX
#include <vcl/morebtn.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _SVTOOLS_SCRIPTEDTEXT_HXX
#include <svtools/scriptedtext.hxx>
#endif
#ifndef SVX_FRAMELINKARRAY_HXX
#include <svx/framelinkarray.hxx>
#endif


//------------------------------------------------------------------------

class ScAutoFormat;
class ScAutoFormatData;
class SvxBoxItem;
class SvxLineItem;
class AutoFmtPreview; // s.u.
class SvNumberFormatter;
class ScDocument;

//------------------------------------------------------------------------

enum AutoFmtLine { TOP_LINE, BOTTOM_LINE, LEFT_LINE, RIGHT_LINE };

//========================================================================

//CHINA001 class ScAutoFormatDlg : public ModalDialog
//CHINA001 {
//CHINA001 public:
//CHINA001 ScAutoFormatDlg( Window*                 pParent,
//CHINA001 ScAutoFormat*                pAutoFormat,
//CHINA001 const ScAutoFormatData*    pSelFormatData,
//CHINA001 ScDocument*                pDoc );
//CHINA001 ~ScAutoFormatDlg();
//CHINA001
//CHINA001 USHORT GetIndex() const { return nIndex; }
//CHINA001 String GetCurrFormatName();
//CHINA001
//CHINA001 private:
//CHINA001 FixedLine       aFlFormat;
//CHINA001 ListBox          aLbFormat;
//CHINA001 AutoFmtPreview*  pWndPreview;
//CHINA001 OKButton     aBtnOk;
//CHINA001 CancelButton aBtnCancel;
//CHINA001 HelpButton       aBtnHelp;
//CHINA001 PushButton       aBtnAdd;
//CHINA001 PushButton       aBtnRemove;
//CHINA001 MoreButton       aBtnMore;
//CHINA001 FixedLine       aFlFormatting;
//CHINA001 CheckBox     aBtnNumFormat;
//CHINA001 CheckBox     aBtnBorder;
//CHINA001 CheckBox     aBtnFont;
//CHINA001 CheckBox     aBtnPattern;
//CHINA001 CheckBox     aBtnAlignment;
//CHINA001 CheckBox     aBtnAdjust;
//CHINA001 PushButton       aBtnRename;
//CHINA001 String           aStrTitle;
//CHINA001 String           aStrLabel;
//CHINA001 String           aStrClose;
//CHINA001 String           aStrDelTitle;
//CHINA001 String           aStrDelMsg;
//CHINA001 String           aStrRename;
//CHINA001
//CHINA001 //------------------------
//CHINA001 ScAutoFormat*            pFormat;
//CHINA001 const ScAutoFormatData*  pSelFmtData;
//CHINA001 USHORT                   nIndex;
//CHINA001 BOOL                 bCoreDataChanged;
//CHINA001 BOOL                 bFmtInserted;
//CHINA001
//CHINA001 void Init            ();
//CHINA001 void UpdateChecks    ();
//CHINA001 //------------------------
//CHINA001 DECL_LINK( CheckHdl, Button * );
//CHINA001 DECL_LINK( AddHdl, void * );
//CHINA001 DECL_LINK( RemoveHdl, void * );
//CHINA001 DECL_LINK( SelFmtHdl, void * );
//CHINA001 DECL_LINK( CloseHdl, PushButton * );
//CHINA001 DECL_LINK( DblClkHdl, void * );
//CHINA001 DECL_LINK( RenameHdl, void *);
//CHINA001
//CHINA001 };
//CHINA001
//========================================================================

class AutoFmtPreview : public Window
{
public:
            AutoFmtPreview( Window* pParent, const ResId& rRes, ScDocument* pDoc );
            ~AutoFmtPreview();

    void NotifyChange( ScAutoFormatData* pNewData );

protected:
    virtual void Paint( const Rectangle& rRect );

private:
    ScAutoFormatData*       pCurData;
    VirtualDevice           aVD;
    SvtScriptedTextHelper   aScriptedText;
    ::com::sun::star::uno::Reference< ::com::sun::star::i18n::XBreakIterator > xBreakIter;
    BOOL                    bFitWidth;
    svx::frame::Array       maArray;            /// Implementation to draw the frame borders.
    Size                    aPrvSize;
    long                    mnLabelColWidth;
    long                    mnDataColWidth1;
    long                    mnDataColWidth2;
    long                    mnRowHeight;
    const String            aStrJan;
    const String            aStrFeb;
    const String            aStrMar;
    const String            aStrNorth;
    const String            aStrMid;
    const String            aStrSouth;
    const String            aStrSum;
    SvNumberFormatter*      pNumFmt;
    //-------------------------------------------
    void    Init            ();
    void    DoPaint         ( const Rectangle& rRect );
    void    CalcCellArray   ( BOOL bFitWidth );
    void    CalcLineMap     ();
    void    PaintCells      ();

/*  Usage of type size_t instead of SCCOL/SCROW is correct here - used in
    conjunction with class svx::frame::Array (svx/framelinkarray.hxx), which
    expects size_t coordinates. */

    USHORT              GetFormatIndex( size_t nCol, size_t nRow ) const;
    const SvxBoxItem&   GetBoxItem( size_t nCol, size_t nRow ) const;
    const SvxLineItem&  GetDiagItem( size_t nCol, size_t nRow, bool bTLBR ) const;

    void                DrawString( size_t nCol, size_t nRow );
    void                DrawStrings();
    void                DrawBackground();

    void    MakeFonts       ( USHORT nIndex,
                              Font& rFont,
                              Font& rCJKFont,
                              Font& rCTLFont );
    String  MakeNumberString( String cellString, BOOL bAddDec );
};

#endif // SC_AUTOFMT_HXX


