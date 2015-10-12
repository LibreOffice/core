/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_INC_FORMULAOPT_HXX
#define INCLUDED_SC_INC_FORMULAOPT_HXX

#include <map>
#include <svl/poolitem.hxx>
#include <unotools/configitem.hxx>
#include <unotools/localedatawrapper.hxx>
#include <formula/grammar.hxx>
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

// item for the dialog / options page

class SC_DLLPUBLIC ScTpFormulaItem : public SfxPoolItem
{
public:
    TYPEINFO_OVERRIDE();
    ScTpFormulaItem( sal_uInt16 nWhich,
                   const ScFormulaOptions& rOpt );
    ScTpFormulaItem( const ScTpFormulaItem& rItem );
    virtual ~ScTpFormulaItem();

    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const override;

    const ScFormulaOptions& GetFormulaOptions() const { return theOptions; }

private:
    ScFormulaOptions theOptions;
};

// config item

class ScFormulaCfg : public ScFormulaOptions, public utl::ConfigItem
{
    typedef std::map<OUString,sal_uInt16> PropsToIds;
    static com::sun::star::uno::Sequence<OUString> GetPropertyNames();
    static ScFormulaCfg::PropsToIds GetPropNamesToId();
    void UpdateFromProperties( const com::sun::star::uno::Sequence<OUString>& rNames );

    virtual void ImplCommit() override;

public:
    ScFormulaCfg();

    void SetOptions( const ScFormulaOptions& rNew );

    virtual void Notify( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
