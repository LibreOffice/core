/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linenum.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:24:44 $
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
#ifndef _SWLINENUM_HXX
#define _SWLINENUM_HXX

#ifndef _BASEDLGS_HXX //autogen
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _NUMBERINGTYPELISTBOX_HXX
#include <numberingtypelistbox.hxx>
#endif

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

class Window;
class SfxItemSet;
class SwView;
class SwWrtShell;

/*--------------------------------------------------------------------
   Beschreibung: SingleTabDialog
 --------------------------------------------------------------------*/

class SwLineNumberingDlg : public SfxSingleTabDialog
{
    SwWrtShell* pSh;

    DECL_LINK( OKHdl, Button * );

public:
    inline SwWrtShell* GetWrtShell() const { return pSh; }

     SwLineNumberingDlg(SwView *pVw);
    ~SwLineNumberingDlg();
};

/*--------------------------------------------------------------------
   Beschreibung: TabPage
 --------------------------------------------------------------------*/

class SwLineNumberingPage : public SfxTabPage
{
    CheckBox        aNumberingOnCB;
    FixedText       aCharStyleFT;
    ListBox         aCharStyleLB;
    FixedText       aFormatFT;
    SwNumberingTypeListBox   aFormatLB;
    FixedText       aPosFT;
    ListBox         aPosLB;
    FixedText       aOffsetFT;
    MetricField     aOffsetMF;
    FixedText       aNumIntervalFT;
    NumericField    aNumIntervalNF;
    FixedText       aNumRowsFT;
    FixedLine        aDisplayFL;
    FixedText       aDivisorFT;
    Edit            aDivisorED;
    FixedText       aDivIntervalFT;
    NumericField    aDivIntervalNF;
    FixedText       aDivRowsFT;
    FixedLine        aDivisorFL;
    CheckBox        aCountEmptyLinesCB;
    CheckBox        aCountFrameLinesCB;
    CheckBox        aRestartEachPageCB;
    FixedLine        aCountFL;

    SwWrtShell*     pSh;

    SwLineNumberingPage( Window* pParent, const SfxItemSet& rSet );
    ~SwLineNumberingPage();

    DECL_LINK( LineOnOffHdl, CheckBox *pCB = 0 );
    DECL_LINK( ModifyHdl, Edit *pED = 0 );

public:

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet );

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
};

#endif


