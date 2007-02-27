/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tpcalc.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2007-02-27 13:26:43 $
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

#ifndef SC_TPCALC_HXX
#define SC_TPCALC_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _STDCTRL_HXX //autogen
#include <svtools/stdctrl.hxx>
#endif

#ifndef SC_EDITFIELD_HXX
#include "editfield.hxx"
#endif

//===================================================================

class ScDocOptions;

class ScTpCalcOptions : public SfxTabPage
{
public:
    static  SfxTabPage* Create          ( Window*               pParent,
                                          const SfxItemSet&     rCoreSet );
    static  USHORT*     GetRanges       ();
    virtual BOOL        FillItemSet     ( SfxItemSet& rCoreSet );
    virtual void        Reset           ( const SfxItemSet& rCoreSet );
    using SfxTabPage::DeactivatePage;
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
                ScTpCalcOptions( Window*            pParent,
                                 const SfxItemSet&  rCoreSet );
                ~ScTpCalcOptions();

private:
    FixedLine       aGbZRefs;
    CheckBox        aBtnIterate;
    FixedText       aFtSteps;
    NumericField    aEdSteps;
    FixedText       aFtEps;
    ScDoubleField   aEdEps;

    FixedLine       aSeparatorFL;
    FixedLine       aGbDate;
    RadioButton     aBtnDateStd;
    RadioButton     aBtnDateSc10;
    RadioButton     aBtnDate1904;

    FixedLine       aHSeparatorFL;
    CheckBox        aBtnCase;
    CheckBox        aBtnCalc;
    CheckBox        aBtnMatch;
    CheckBox        aBtnRegex;
    CheckBox        aBtnLookUp;

    FixedText       aFtPrec;
    NumericField    aEdPrec;

    ScDocOptions*   pOldOptions;
    ScDocOptions*   pLocalOptions;
    USHORT          nWhichCalc;

#ifdef _TPCALC_CXX
private:
    void            Init();

    //------------------------------------
    // Handler:
    DECL_LINK( RadioClickHdl, RadioButton* );
    DECL_LINK( CheckClickHdl, CheckBox* );

#endif
};


#endif

