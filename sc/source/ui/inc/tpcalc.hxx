/*************************************************************************
 *
 *  $RCSfile: tpcalc.hxx,v $
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
    virtual int         DeactivatePage  ( SfxItemSet* pSet = NULL );

private:
                ScTpCalcOptions( Window*            pParent,
                                 const SfxItemSet&  rCoreSet );
                ~ScTpCalcOptions();

private:
    CheckBox        aBtnIterate;
    FixedText       aFtSteps;
    NumericField    aEdSteps;
    FixedText       aFtEps;
    Edit            aEdEps;
    GroupBox        aGbZRefs;

    RadioButton     aBtnDateStd;
    RadioButton     aBtnDateSc10;
    RadioButton     aBtnDate1904;

    GroupBox        aGbDate;

    CheckBox        aBtnCase;
    CheckBox        aBtnCalc;
    CheckBox        aBtnMatch;
    CheckBox        aBtnLookUp;

    FixedText       aFtPrec;
    NumericField    aEdPrec;

    const char      cDecSep;

    ScDocOptions*   pOldOptions;
    ScDocOptions*   pLocalOptions;
    USHORT          nWhichCalc;

#ifdef _TPCALC_CXX
private:
    void            Init();
    BOOL            CheckEps();
    BOOL            GetEps( double& rEps );

    //------------------------------------
    // Handler:
    DECL_LINK( RadioClickHdl, RadioButton* );
    DECL_LINK( CheckClickHdl, CheckBox* );

#endif
};


#endif

