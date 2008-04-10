/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: validate.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SC_VALIDATE_HXX
#define SC_VALIDATE_HXX

#include <sfx2/tabdlg.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/svmedit.hxx>


// ============================================================================

/** The "Validity" tab dialog. */
class ScValidationDlg : public SfxTabDialog
{
public:
    explicit                    ScValidationDlg( Window* pParent, const SfxItemSet* pArgSet );
    virtual                     ~ScValidationDlg();
};


// ============================================================================

/** The tab page "Criteria" from the Validation dialog. */
class ScTPValidationValue : public SfxTabPage
{
public:
    explicit                    ScTPValidationValue( Window* pParent, const SfxItemSet& rArgSet );
    virtual                     ~ScTPValidationValue();

    static SfxTabPage*          Create( Window* pParent, const SfxItemSet& rArgSet );
    static USHORT*              GetRanges();

    virtual BOOL                FillItemSet( SfxItemSet& rArgSet );
    virtual void                Reset( const SfxItemSet& rArgSet );

private:
    void                        Init();

    String                      GetFirstFormula() const;
    String                      GetSecondFormula() const;

    void                        SetFirstFormula( const String& rFmlaStr );
    void                        SetSecondFormula( const String& rFmlaStr );

                                DECL_LINK( SelectHdl, ListBox* );
                                DECL_LINK( CheckHdl, CheckBox* );

    FixedText                   maFtAllow;
    ListBox                     maLbAllow;
    CheckBox                    maCbAllow;      /// Allow blank cells.
    CheckBox                    maCbShow;       /// Show selection list in cell.
    CheckBox                    maCbSort;       /// Sort selection list in cell.
    FixedText                   maFtValue;
    ListBox                     maLbValue;
    FixedText                   maFtMin;
    Edit                        maEdMin;
    MultiLineEdit               maEdList;       /// Entries for explicit list
    FixedText                   maFtMax;
    Edit                        maEdMax;
    FixedText                   maFtHint;       /// Hint text for cell range validity.

    String                      maStrMin;
    String                      maStrMax;
    String                      maStrValue;
    String                      maStrRange;
    String                      maStrList;
    sal_Unicode                 mcFmlaSep;      /// List separator in formulas.
};


//==================================================================

class ScTPValidationHelp : public SfxTabPage
{
private:
    TriStateBox     aTsbHelp;
    FixedLine       aFlContent;
    FixedText       aFtTitle;
    Edit            aEdtTitle;
    FixedText       aFtInputHelp;
    MultiLineEdit   aEdInputHelp;

    const SfxItemSet& mrArgSet;

    void    Init();

    // Handler ------------------------
    // DECL_LINK( SelectHdl, ListBox * );

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
    FixedLine       aFlContent;
    FixedText       aFtAction;
    ListBox         aLbAction;
    PushButton      aBtnSearch;
    FixedText       aFtTitle;
    Edit            aEdtTitle;
    FixedText       aFtError;
    MultiLineEdit   aEdError;

    const SfxItemSet& mrArgSet;

    void    Init();

    // Handler ------------------------
    DECL_LINK( SelectActionHdl, ListBox * );
    DECL_LINK( ClickSearchHdl, PushButton * );

public:
            ScTPValidationError( Window* pParent, const SfxItemSet& rArgSet );
            ~ScTPValidationError();

    static  SfxTabPage* Create      ( Window* pParent, const SfxItemSet& rArgSet );
    static  USHORT*     GetRanges   ();
    virtual BOOL        FillItemSet ( SfxItemSet& rArgSet );
    virtual void        Reset       ( const SfxItemSet& rArgSet );
};


#endif // SC_VALIDATE_HXX

