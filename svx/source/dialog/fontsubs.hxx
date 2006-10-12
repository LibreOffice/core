/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fontsubs.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-12 12:12:28 $
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
#ifndef _SVX_FONT_SUBSTITUTION_HXX
#define _SVX_FONT_SUBSTITUTION_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SVX_SIMPTABL_HXX //autogen
#include "simptabl.hxx"
#endif
#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif
#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// class SvxFontSubstCheckListBox ------------------------------------------

class SvxFontSubstCheckListBox : public SvxSimpleTable
{
    friend class SvxFontSubstTabPage;
    using SvxSimpleTable::SetTabs;
    using SvTreeListBox::GetCheckButtonState;
    using SvTreeListBox::SetCheckButtonState;

    protected:
        virtual void    SetTabs();
        virtual void    KeyInput( const KeyEvent& rKEvt );

    public:
        SvxFontSubstCheckListBox(Window* pParent, const ResId& rResId ) :
            SvxSimpleTable( pParent, rResId ){}

        inline void     *GetUserData(ULONG nPos) { return GetEntry(nPos)->GetUserData(); }
        inline void     SetUserData(ULONG nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }

        BOOL            IsChecked(ULONG nPos, USHORT nCol = 0);
        BOOL            IsChecked(SvLBoxEntry* pEntry, USHORT nCol = 0);
        void            CheckEntryPos(ULONG nPos, USHORT nCol, BOOL bChecked);
        void            CheckEntry(SvLBoxEntry* pEntry, USHORT nCol, BOOL bChecked);
        SvButtonState   GetCheckButtonState( SvLBoxEntry*, USHORT nCol ) const;
        void            SetCheckButtonState( SvLBoxEntry*, USHORT nCol, SvButtonState );
};

// class SvxFontSubstTabPage ----------------------------------------------------
class SvtFontSubstConfig;
namespace svt {class SourceViewConfig;}
class SVX_DLLPUBLIC SvxFontSubstTabPage : public SfxTabPage
{
    CheckBox                    aUseTableCB;
    FixedText                   aFont1FT;
    FontNameBox                 aFont1CB;
    FixedText                   aFont2FT;
    FontNameBox                 aFont2CB;
    ToolBox                     aNewDelTBX;
    SvxFontSubstCheckListBox    aCheckLB;

    FixedLine                   aSourceViewFontsFL;
    FixedText                   aFontNameFT;
    ListBox                     aFontNameLB;
    CheckBox                    aNonPropFontsOnlyCB;
    FixedText                   aFontHeightFT;
    ListBox                     aFontHeightLB;

    ImageList                   aImageList;
    String                      sAutomatic;

    SvtFontSubstConfig*         pConfig;
    svt::SourceViewConfig*      pSourceViewConfig;

    String          sHeader1;
    String          sHeader2;
    String          sHeader3;
    String          sHeader4;

    Bitmap          aChkunBmp;
    Bitmap          aChkchBmp;
    Bitmap          aChkchhiBmp;
    Bitmap          aChkunhiBmp;
    Bitmap          aChktriBmp;
    Bitmap          aChktrihiBmp;
    Color           aTextColor;
    ByteString      sFontGroup;

    SvLBoxButtonData*   pCheckButtonData;

    DECL_LINK(SelectHdl, Window *pWin = 0);
    DECL_LINK(NonPropFontsHdl, CheckBox* pBox);

    SvLBoxEntry*    CreateEntry(String& rFont1, String& rFont2);
    void            CheckEnable();


    SvxFontSubstTabPage( Window* pParent, const SfxItemSet& rSet );
    ~SvxFontSubstTabPage();

public:
    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);
    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};


#endif // _SVX_FONT_SUBSTITUTION_HXX












