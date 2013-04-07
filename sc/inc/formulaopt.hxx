/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Albert Thuswaldner <albert.thuswaldner@gmail.com>
 * Portions created by the Initial Developer are Copyright (C) 2012 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef SC_FORMULAOPT_HXX
#define SC_FORMULAOPT_HXX

#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include <unotools/localedatawrapper.hxx>
#include "formula/grammar.hxx"
#include "scdllapi.h"
#include "global.hxx"
#include "calcconfig.hxx"

class SC_DLLPUBLIC ScFormulaOptions
{
private:
    bool bUseEnglishFuncName;     // use English function name even if the locale is not English.
    formula::FormulaGrammar::Grammar eFormulaGrammar;  // formula grammar used to switch different formula syntax
    ScCalcConfig aCalcConfig;

    OUString aFormulaSepArg;
    OUString aFormulaSepArrayRow;
    OUString aFormulaSepArrayCol;

    ScRecalcOptions meOOXMLRecalc;
    ScRecalcOptions meODFRecalc;

public:
    ScFormulaOptions();
    ScFormulaOptions( const ScFormulaOptions& rCpy );
    ~ScFormulaOptions();

    void SetDefaults();

    void SetFormulaSyntax( ::formula::FormulaGrammar::Grammar eGram ) { eFormulaGrammar = eGram; }
    ::formula::FormulaGrammar::Grammar GetFormulaSyntax() const { return eFormulaGrammar; }

    ScCalcConfig& GetCalcConfig() { return aCalcConfig; }
    const ScCalcConfig& GetCalcConfig() const { return aCalcConfig; }
    void SetCalcConfig(const ScCalcConfig& rConfig) { aCalcConfig = rConfig; }

    void SetUseEnglishFuncName( bool bVal ) { bUseEnglishFuncName = bVal; }
    bool GetUseEnglishFuncName() const { return bUseEnglishFuncName; }

    void SetFormulaSepArg(const OUString& rSep) { aFormulaSepArg = rSep; }
    OUString GetFormulaSepArg() const { return aFormulaSepArg; }

    void SetFormulaSepArrayRow(const OUString& rSep) { aFormulaSepArrayRow = rSep; }
    OUString GetFormulaSepArrayRow() const { return aFormulaSepArrayRow; }

    void SetFormulaSepArrayCol(const OUString& rSep) { aFormulaSepArrayCol = rSep; }
    OUString GetFormulaSepArrayCol() const { return aFormulaSepArrayCol; }

    void SetOOXMLRecalcOptions( ScRecalcOptions eOpt ) { meOOXMLRecalc = eOpt; }
    ScRecalcOptions GetOOXMLRecalcOptions() const { return meOOXMLRecalc; }

    void SetODFRecalcOptions( ScRecalcOptions eOpt ) { meODFRecalc = eOpt; }
    ScRecalcOptions GetODFRecalcOptions() const { return meODFRecalc; }

    void ResetFormulaSeparators();

    static void GetDefaultFormulaSeparators(OUString& rSepArg, OUString& rSepArrayCol, OUString& rSepArrayRow);

    static const LocaleDataWrapper& GetLocaleDataWrapper();

    ScFormulaOptions&  operator=  ( const ScFormulaOptions& rCpy );
    bool               operator== ( const ScFormulaOptions& rOpt ) const;
    bool               operator!= ( const ScFormulaOptions& rOpt ) const;
};

//==================================================================
// item for the dialog / options page
//==================================================================

class SC_DLLPUBLIC ScTpFormulaItem : public SfxPoolItem
{
public:
    TYPEINFO();
    ScTpFormulaItem( sal_uInt16 nWhich,
                   const ScFormulaOptions& rOpt );
    ScTpFormulaItem( const ScTpFormulaItem& rItem );
    ~ScTpFormulaItem();

    virtual String          GetValueText() const;
    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

    const ScFormulaOptions& GetFormulaOptions() const { return theOptions; }

private:
    ScFormulaOptions theOptions;
};

//==================================================================
// config item
//==================================================================

class ScFormulaCfg : public ScFormulaOptions, public utl::ConfigItem
{
    com::sun::star::uno::Sequence<OUString> GetPropertyNames();
public:
    ScFormulaCfg();

    void SetOptions( const ScFormulaOptions& rNew );

    virtual void Commit();
    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
