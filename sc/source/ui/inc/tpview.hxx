/*************************************************************************
 *
 *  $RCSfile: tpview.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:01 $
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
    GroupBox    aDisplayGB;
    CheckBox    aFormulaCB;
    CheckBox    aNilCB;
    CheckBox    aAnnotCB;
    CheckBox    aValueCB;
    CheckBox    aAnchorCB;
    CheckBox    aClipMarkCB;

    GroupBox    aObjectGB;
    FixedText   aObjGrfFT;
    ListBox     aObjGrfLB;
    FixedText   aDiagramFT;
    ListBox     aDiagramLB;
    FixedText   aDrawFT;
    ListBox     aDrawLB;

    ScViewOptions*  pLocalOptions;

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
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );


};

//========================================================================
// TabPage Layout

class ScTpLayoutOptions : public SfxTabPage
{
    GroupBox        aLinesGB;
    CheckBox        aGridCB;
    FixedText       aColorFT;
    ColorListBox    aColorLB;
    CheckBox        aBreakCB;
    CheckBox        aGuideLineCB;
    CheckBox        aHandleCB;
    CheckBox        aBigHandleCB;

    GroupBox        aWindowGB;
    CheckBox        aRowColHeaderCB;
    CheckBox        aHScrollCB;
    CheckBox        aVScrollCB;
    CheckBox        aTblRegCB;
    CheckBox        aOutlineCB;

    GroupBox        aUnitGB;
    ListBox         aUnitLB;

    GroupBox    aTabGB;
    MetricField aTabMF;

    SvxStringArray  aUnitArr;
    ScViewOptions*  pLocalOptions;

    void    InitGridOpt();
    DECL_LINK( GridHdl, CheckBox* );
    DECL_LINK( CBHdl, CheckBox* );
    DECL_LINK(MetricHdl, ListBox*);

            ScTpLayoutOptions( Window*          pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpLayoutOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );


};
//========================================================================
// TabPage Eingabe

class ScTpInputOptions : public SfxTabPage
{
    GroupBox        aOptionsGB;
    CheckBox        aAlignCB;
    ListBox         aAlignLB;
    CheckBox        aEditModeCB;
    CheckBox        aFormatCB;
    CheckBox        aRangeFindCB;
    CheckBox        aExpRefCB;
    CheckBox        aMarkHdrCB;

    DECL_LINK( AlignHdl, CheckBox* );

            ScTpInputOptions( Window*           pParent,
                             const SfxItemSet&  rArgSet );
            ~ScTpInputOptions();

public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );

};



#endif // SC_TPUSRLST_HXX

