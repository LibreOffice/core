/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: numfmt.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:19:31 $
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
#ifndef _SVX_NUMFMT_HXX
#define _SVX_NUMFMT_HXX

//------------------------------------------------------------------------

#ifndef _WINDOW_HXX //autogen
#include <vcl/window.hxx>
#endif

#ifndef _COLOR_HXX //autogen
#include <tools/color.hxx>
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SVX_LANGBOX_HXX //autogen
#include "langbox.hxx"
#endif

#ifndef SVX_FONTLB_HXX //autogen
#include "fontlb.hxx"
#endif

//CHINA001 #define SVX_NUMVAL_STANDARD      -1234.12345678901234
//CHINA001 #define SVX_NUMVAL_CURRENCY      -1234
//CHINA001 #define SVX_NUMVAL_PERCENT       -0.1295
//CHINA001 #define SVX_NUMVAL_TIME      36525.5678935185
//CHINA001 #define SVX_NUMVAL_DATE      36525.5678935185
//CHINA001 #define SVX_NUMVAL_BOOLEAN       1

//------------------------------------------------------------------------

class SvxNumberFormatShell;
class SvxNumberInfoItem;

//------------------------------------------------------------------------

class SvxNumberPreviewImpl : public Window
{
private:
    String          aPrevStr;
    Color           aPrevCol;

    void            InitSettings( BOOL bForeground, BOOL bBackground );

protected:
    virtual void    Paint( const Rectangle& rRect );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

public:
    SvxNumberPreviewImpl( Window* pParent, const ResId& rResId );
    ~SvxNumberPreviewImpl();

    void            NotifyChange( const String& rPrevStr, const Color* pColor = NULL );
};

// -----------------------------------------------------------------------

class SvxNumberFormatTabPage : public SfxTabPage

/*  {k:\svx\prototyp\dialog\numfmt.bmp}

    [Beschreibung]
    Mit dieser TabPage koennen Zahlenformate eingestellt und benutzerdefinierte
    Formate verwaltet werden.

    [Items]
    <SfxUInt32Item>:        <SID_ATTR_NUMBERFORMAT_VALUE>
    <SvxNumberInfoItem>:    <SID_ATTR_NUMBERFORMAT_INFO>
*/

{
    using TabPage::DeactivatePage;
public:
    ~SvxNumberFormatTabPage();

    static SfxTabPage*      Create( Window* pParent,
                                    const SfxItemSet& rAttrSet );
    static USHORT*          GetRanges();

    virtual BOOL            FillItemSet( SfxItemSet& rSet );
    virtual void            Reset( const SfxItemSet& rSet );
    virtual int             DeactivatePage  ( SfxItemSet* pSet = NULL );

    void                    SetInfoItem( const SvxNumberInfoItem& rItem );
    void                    SetNumberFormatList( const SvxNumberInfoItem& rItem )
                                { SetInfoItem( rItem ); }

    void                    SetOkHdl( const Link& rOkHandler );
    void                    HideLanguage(BOOL nFlag=TRUE);
    virtual long            PreNotify( NotifyEvent& rNEvt );
    virtual void            PageCreated (SfxAllItemSet aSet); //add CHINA001
private:
    SvxNumberFormatTabPage( Window* pParent,
                            const SfxItemSet& rCoreAttrs );
    FixedText               aFtCategory;
    ListBox                 aLbCategory;
    FixedText               aFtFormat;
    ListBox                 aLbCurrency;
    SvxFontListBox          aLbFormat;
    FixedText               aFtLanguage;
    SvxLanguageBox          aLbLanguage;
    CheckBox                aCbSourceFormat;

    FixedText               aFtDecimals;
    NumericField            aEdDecimals;
    FixedText               aFtLeadZeroes;
    NumericField            aEdLeadZeroes;
    CheckBox                aBtnNegRed;
    CheckBox                aBtnThousand;
    FixedLine               aFlOptions;

    FixedText               aFtEdFormat;
    Edit                    aEdFormat;
    ImageButton             aIbAdd;
    ImageButton             aIbInfo;
    ImageButton             aIbRemove;
    ImageList               aIconListDepricated;

    FixedText               aFtComment;
    Edit                    aEdComment;
    Timer                   aResetWinTimer;

    SvxNumberPreviewImpl    aWndPreview;

    SvxNumberInfoItem*      pNumItem;
    SvxNumberFormatShell*   pNumFmtShell;
    ULONG                   nInitFormat;
    Link                    fnOkHdl;

    BOOL                    bNumItemFlag; //Fuer Handling mit DocShell
    BOOL                    bOneAreaFlag;
    short                   nFixedCategory;

    long                    nCatHeight;

    long                    nCurFormatY;
    long                    nCurFormatHeight;
    long                    nStdFormatY;
    long                    nStdFormatHeight;
    String                  aStrEurope;
    String                  sAutomaticEntry;

    Window*                 pLastActivWindow;

#ifdef _SVX_NUMFMT_CXX
    void    Init_Impl();
    void    FillCurrencyBox();
    void    FillFormatListBox_Impl( SvxDelStrgs& rEntries );
    void    UpdateOptions_Impl( BOOL bCheckCatChange );
    void    UpdateFormatListBox_Impl( USHORT bCat, BOOL bUpdateEdit );
    void    DeleteEntryList_Impl( SvxDelStrgs& rEntries );
    void    Obstructing();
    void    EnableBySourceFormat_Impl();
    void    SetCategory( USHORT nPos );
    String  GetExpColorString( Color*& rpPreviewColor, const String& aFormatStr, short nTmpCatPos );
    void    MakePreviewText( const String& rFormat );
    void    ChangePreviewText( USHORT nPos );
    void    AddAutomaticLanguage_Impl(LanguageType eAutoLang, BOOL bSelect);
    // Handler
    DECL_LINK( LostFocusHdl_Impl, Edit* pEd );
    DECL_LINK( DoubleClickHdl_Impl, SvxFontListBox* pLb );
    DECL_LINK( SelFormatHdl_Impl, void * );
    DECL_LINK( ClickHdl_Impl, ImageButton* pIB );
    DECL_LINK( EditHdl_Impl, Edit* pEdFormat );
    DECL_LINK( OptHdl_Impl, void * );
    DECL_LINK( TimeHdl_Impl, Timer * );

#endif
};


#endif

