/*************************************************************************
 *
 *  $RCSfile: validate.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:45:02 $
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

#ifndef SC_VALIDATE_HXX
#define SC_VALIDATE_HXX


#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _SV_EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#ifndef _SV_GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif

#ifndef _SV_LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _SVEDIT_HXX //autogen
#include <svtools/svmedit.hxx>
#endif


//==================================================================

class ScValidationDlg : public SfxTabDialog
{
public:
                ScValidationDlg( Window* pParent, const SfxItemSet* pArgSet );
                ~ScValidationDlg();

};

//==================================================================

class ScTPValidationValue : public SfxTabPage
{
private:
    FixedText   aFtAllow;
    ListBox     aLbAllow;
    TriStateBox aTsbAllow;
    FixedText   aFtValue;
    ListBox     aLbValue;
    FixedText   aFtMin;
    Edit        aEdtMin;
    FixedText   aFtMax;
    Edit        aEdtMax;
    GroupBox    aGrpValues;

    String      aStrMin;
    String      aStrMax;
    String      aStrValue;

    const SfxItemSet& rSet;

#ifdef _VALIDATE_CXX
    void    Init();

    // Handler ------------------------
    DECL_LINK( SelectAllowHdl, ListBox * );
    DECL_LINK( SelectValueHdl, ListBox * );
#endif

public:
            ScTPValidationValue( Window* pParent, const SfxItemSet& rArgSet );
            ~ScTPValidationValue();

    static  SfxTabPage* Create      ( Window* pParent, const SfxItemSet& rArgSet );
    static  USHORT*     GetRanges   ();
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};

//==================================================================

class ScTPValidationHelp : public SfxTabPage
{
private:
    TriStateBox     aTsbHelp;
    FixedText       aFtTitle;
    Edit            aEdtTitle;
    FixedText       aFtInputHelp;
    MultiLineEdit   aEdInputHelp;
    GroupBox        aGrpContent;

    const SfxItemSet& rSet;

#ifdef _VALIDATE_CXX
    void    Init();

    // Handler ------------------------
    // DECL_LINK( SelectHdl, ListBox * );
#endif

public:
            ScTPValidationHelp( Window* pParent, const SfxItemSet& rArgSet );
            ~ScTPValidationHelp();

    static  SfxTabPage* Create      ( Window* pParent, const SfxItemSet& rArgSet );
    static  USHORT*     GetRanges   ();
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};

//==================================================================

class ScTPValidationError : public SfxTabPage
{
private:
    TriStateBox     aTsbShow;
    FixedText       aFtAction;
    ListBox         aLbAction;
    PushButton      aBtnSearch;
    FixedText       aFtTitle;
    Edit            aEdtTitle;
    FixedText       aFtError;
    MultiLineEdit   aEdError;
    GroupBox        aGrpContent;

    const SfxItemSet& rSet;

#ifdef _VALIDATE_CXX
    void    Init();

    // Handler ------------------------
    DECL_LINK( SelectActionHdl, ListBox * );
    DECL_LINK( ClickSearchHdl, PushButton * );
#endif

public:
            ScTPValidationError( Window* pParent, const SfxItemSet& rArgSet );
            ~ScTPValidationError();

    static  SfxTabPage* Create      ( Window* pParent, const SfxItemSet& rArgSet );
    static  USHORT*     GetRanges   ();
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};


#endif // SC_VALIDATE_HXX


