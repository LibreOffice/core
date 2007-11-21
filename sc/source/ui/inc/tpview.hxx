/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpview.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-21 19:09:52 $
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

#ifndef SC_TPVIEW_HXX
#define SC_TPVIEW_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _CTRLBOX_HXX //autogen
#include <svtools/ctrlbox.hxx>
#endif

#ifndef _SVX_STRARRAY_HXX //autogen
#include <svx/strarray.hxx>
#endif

//========================================================================
// View-Optionen:

class ScViewOptions;

//========================================================================
// TabPage Inhalte

class ScTpContentOptions : public SfxTabPage
{
    FixedLine       aLinesGB;
    CheckBox        aGridCB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;
    CheckBox        aBreakCB;
    CheckBox        aGuideLineCB;
    CheckBox        aHandleCB;
    CheckBox        aBigHandleCB;

    FixedLine       aSeparator1FL;

    FixedLine       aDisplayGB;
    CheckBox        aFormulaCB;
    CheckBox        aNilCB;
    CheckBox        aAnnotCB;
    CheckBox        aValueCB;
    CheckBox        aAnchorCB;
    CheckBox        aClipMarkCB;
    CheckBox        aRangeFindCB;

    FixedLine       aObjectGB;
    FixedText       aObjGrfFT;
    ListBox         aObjGrfLB;
    FixedText       aDiagramFT;
    ListBox         aDiagramLB;
    FixedText       aDrawFT;
    ListBox         aDrawLB;

    FixedLine       aZoomGB;
    CheckBox        aSyncZoomCB;

    FixedLine       aSeparator2FL;

    FixedLine       aWindowGB;
    CheckBox        aRowColHeaderCB;
    CheckBox        aHScrollCB;
    CheckBox        aVScrollCB;
    CheckBox        aTblRegCB;
    CheckBox        aOutlineCB;

    ScViewOptions*  pLocalOptions;

    void    InitGridOpt();
    DECL_LINK( GridHdl, CheckBox* );
    DECL_LINK( SelLbObjHdl, ListBox* );
    DECL_LINK( CBHdl, CheckBox* );

            ScTpContentOptions( Window*         pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpContentOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );


};

//========================================================================
// TabPage Layout
class ScDocument;
class ScTpLayoutOptions : public SfxTabPage
{
    FixedLine       aUnitGB;
    FixedText       aUnitFT;
    ListBox         aUnitLB;
    FixedText       aTabFT;
    MetricField     aTabMF;

    FixedLine       aSeparatorFL;
    FixedLine       aLinkGB;
    FixedText       aLinkFT;
    RadioButton     aAlwaysRB;
    RadioButton     aRequestRB;
    RadioButton     aNeverRB;

    FixedLine       aOptionsGB;
    CheckBox        aAlignCB;
    ListBox         aAlignLB;
    CheckBox        aEditModeCB;
    CheckBox        aFormatCB;
    CheckBox        aExpRefCB;
    CheckBox        aMarkHdrCB;
    CheckBox        aTextFmtCB;
    CheckBox        aReplWarnCB;

    SvxStringArray  aUnitArr;

    DECL_LINK( CBHdl, CheckBox* );

    DECL_LINK(MetricHdl, ListBox*);
    DECL_LINK( AlignHdl, CheckBox* );

    ScDocument *pDoc;

    DECL_LINK(  UpdateHdl, CheckBox* );

            ScTpLayoutOptions( Window*          pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpLayoutOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::ActivatePage;
    using SfxTabPage::DeactivatePage;
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    void                SetDocument(ScDocument* pPtr){pDoc = pPtr;}

};
//========================================================================

#endif // SC_TPUSRLST_HXX

