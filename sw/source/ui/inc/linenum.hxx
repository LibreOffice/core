/*************************************************************************
 *
 *  $RCSfile: linenum.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:40 $
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

#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
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
    ListBox         aFormatLB;
    FixedText       aPosFT;
    ListBox         aPosLB;
    FixedText       aOffsetFT;
    MetricField     aOffsetMF;
    FixedText       aNumIntervalFT;
    NumericField    aNumIntervalNF;
    FixedText       aNumRowsFT;
    GroupBox        aDisplayGB;
    FixedText       aDivisorFT;
    Edit            aDivisorED;
    FixedText       aDivIntervalFT;
    NumericField    aDivIntervalNF;
    FixedText       aDivRowsFT;
    GroupBox        aDivisorGB;
    CheckBox        aCountEmptyLinesCB;
    CheckBox        aCountFrameLinesCB;
    CheckBox        aRestartEachPageCB;
    GroupBox        aCountGB;

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


