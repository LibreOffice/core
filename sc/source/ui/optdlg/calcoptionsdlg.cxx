/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <algorithm>
#include <cmath>

#include "calcconfig.hxx"
#include "calcoptionsdlg.hxx"
#include "docfunc.hxx"
#include "docsh.hxx"
#include "interpre.hxx"
#include "sc.hrc"
#include "scresid.hxx"
#include "scopetools.hxx"
#include "viewdata.hxx"

#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>

#include <comphelper/random.hxx>
#include <svtools/svlbitm.hxx>
#include <svtools/treelistentry.hxx>

#if HAVE_FEATURE_OPENCL
#include "formulagroup.hxx"
#include "globalnames.hxx"
#endif

namespace {

formula::FormulaGrammar::AddressConvention toAddressConvention(sal_Int32 nPos)
{
    switch (nPos)
    {
        case 1:
            return formula::FormulaGrammar::CONV_OOO;
        case 2:
            return formula::FormulaGrammar::CONV_XL_A1;
        case 3:
            return formula::FormulaGrammar::CONV_XL_R1C1;
        case 4:
            return formula::FormulaGrammar::CONV_A1_XL_A1;
        case 0:
        default:
            ;
    }

    return formula::FormulaGrammar::CONV_UNSPECIFIED;
}

sal_Int32 toSelectedItem( formula::FormulaGrammar::AddressConvention eConv )
{
    switch (eConv)
    {
        case formula::FormulaGrammar::CONV_OOO:
            return 1;
        case formula::FormulaGrammar::CONV_XL_A1:
            return 2;
        case formula::FormulaGrammar::CONV_XL_R1C1:
            return 3;
        case formula::FormulaGrammar::CONV_A1_XL_A1:
            return 4;
        default:
            ;
    }
    return 0;
}

}

ScCalcOptionsDialog::ScCalcOptionsDialog(vcl::Window* pParent, const ScCalcConfig& rConfig, bool bWriteConfig)
    : ModalDialog(pParent, "FormulaCalculationOptions",
        "modules/scalc/ui/formulacalculationoptions.ui")
    , maConfig(rConfig)
    , mbSelectedEmptyStringAsZero(rConfig.mbEmptyStringAsZero)
    , mbWriteConfig(bWriteConfig)
{
    get(mpTestButton, "test");

    get(mpConversion,"comboConversion");
    mpConversion->SelectEntryPos(static_cast<sal_Int32>(rConfig.meStringConversion));
    mpConversion->SetSelectHdl(LINK(this, ScCalcOptionsDialog, ConversionModifiedHdl));

    get(mpEmptyAsZero,"checkEmptyAsZero");
    mpEmptyAsZero->Check(rConfig.mbEmptyStringAsZero);
    mpEmptyAsZero->SetClickHdl(LINK(this, ScCalcOptionsDialog, AsZeroModifiedHdl));
    CoupleEmptyAsZeroToStringConversion();

    get(mpSyntax,"comboSyntaxRef");
    mpSyntax->SelectEntryPos( toSelectedItem(rConfig.meStringRefAddressSyntax) );
    mpSyntax->SetSelectHdl(LINK(this, ScCalcOptionsDialog, SyntaxModifiedHdl));

    get(mpCurrentDocOnly,"current_doc");
    mpCurrentDocOnly->Check(!mbWriteConfig);
    mpCurrentDocOnly->SetClickHdl(LINK(this, ScCalcOptionsDialog, CurrentDocOnlyHdl));

    get(mpUseOpenCL,"CBUseOpenCL");
    mpUseOpenCL->Check(rConfig.mbOpenCLSubsetOnly);
    mpUseOpenCL->SetClickHdl(LINK(this, ScCalcOptionsDialog, CBUseOpenCLHdl));

    get(mpSpinButton,"spinOpenCLSize");
    mpSpinButton->SetValue(rConfig.mnOpenCLMinimumFormulaGroupSize);
    mpSpinButton->SetModifyHdl(LINK(this, ScCalcOptionsDialog, SpinOpenCLMinSizeHdl));

    get(mpEditField, "entry");
    mpEditField->SetText(ScOpCodeSetToSymbolicString(maConfig.mpOpenCLSubsetOpCodes));
    mpEditField->set_height_request(4 * mpEditField->GetTextHeight());

    mpEditField->SetModifyHdl(LINK(this, ScCalcOptionsDialog, EditModifiedHdl));

    mpTestButton->SetClickHdl(LINK(this, ScCalcOptionsDialog, TestClickHdl));
}

ScCalcOptionsDialog::~ScCalcOptionsDialog()
{
    disposeOnce();
}

void ScCalcOptionsDialog::dispose()
{
    mpEmptyAsZero.clear();
    mpConversion.clear();
    mpSyntax.clear();
    mpCurrentDocOnly.clear();
    mpUseOpenCL.clear();
    mpSpinButton.clear();
    mpEditField.clear();
    mpTestButton.clear();
    ModalDialog::dispose();
}

void ScCalcOptionsDialog::CoupleEmptyAsZeroToStringConversion()
{
    switch (maConfig.meStringConversion)
    {
        case ScCalcConfig::StringConversion::ILLEGAL:
            maConfig.mbEmptyStringAsZero = false;
            mpEmptyAsZero->Check(false);
            mpEmptyAsZero->Enable(false);
            break;
        case ScCalcConfig::StringConversion::ZERO:
            maConfig.mbEmptyStringAsZero = true;
            mpEmptyAsZero->Check();
            mpEmptyAsZero->Enable(false);
            break;
        case ScCalcConfig::StringConversion::UNAMBIGUOUS:
        case ScCalcConfig::StringConversion::LOCALE:
            // Reset to the value the user selected before.
            maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero;
            mpEmptyAsZero->Enable();
            mpEmptyAsZero->Check( mbSelectedEmptyStringAsZero);
            break;
    }
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, AsZeroModifiedHdl, Button*, pCheckBox, void )
{
    maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero = static_cast<CheckBox*>(pCheckBox)->IsChecked();
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, ConversionModifiedHdl, ListBox&, rConv, void )
{
    maConfig.meStringConversion = (ScCalcConfig::StringConversion)rConv.GetSelectEntryPos();
    CoupleEmptyAsZeroToStringConversion();
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, SyntaxModifiedHdl, ListBox&, rSyntax, void)
{
    maConfig.SetStringRefSyntax(toAddressConvention(rSyntax.GetSelectEntryPos()));
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, CurrentDocOnlyHdl, Button*, pCheckBox, void)
{
    mbWriteConfig = !(static_cast<CheckBox*>(pCheckBox)->IsChecked());
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, CBUseOpenCLHdl, Button*, pCheckBox, void)
{
    maConfig.mbOpenCLSubsetOnly = static_cast<CheckBox*>(pCheckBox)->IsChecked();
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, SpinOpenCLMinSizeHdl, Edit&, rEdit, void)
{
    maConfig.mnOpenCLMinimumFormulaGroupSize = static_cast<NumericField&>(rEdit).GetValue();
}

IMPL_LINK_TYPED(ScCalcOptionsDialog, EditModifiedHdl, Edit&, rCtrl, void)
{
    maConfig.mpOpenCLSubsetOpCodes = ScStringToOpCodeSet(rCtrl.GetText());
}

namespace {

struct Area
{
    OUString msTitle;
    int mnRows;

    Area(const OUString& rTitle, int nRows = ScInterpreter::GetGlobalConfig().mnOpenCLMinimumFormulaGroupSize + 2) :
        msTitle(rTitle),
        mnRows(nRows)
    {
    }

    virtual ~Area()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const = 0;

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const = 0;

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const = 0;
};

struct OpenCLTester
{
    int mnTestAreas;
    ScDocShell* mpDocShell;
    ScDocument *mpDoc;
    bool mbOldAutoCalc;
    ScCalcConfig mpOldCalcConfig;

    OpenCLTester() :
        mnTestAreas(0)
    {
        css::uno::Reference< css::uno::XComponentContext > xContext( comphelper::getProcessComponentContext() );
        css::uno::Reference< css::frame::XDesktop2 > xComponentLoader = css::frame::Desktop::create(xContext);
        css::uno::Reference< css::lang::XComponent >
            xComponent( xComponentLoader->loadComponentFromURL( "private:factory/scalc",
                                                                "_blank", 0,
                                                                css::uno::Sequence < css::beans::PropertyValue >() ) );
        mpDocShell = dynamic_cast<ScDocShell*>(SfxObjectShell::GetShellFromComponent(xComponent));

        assert(mpDocShell);

        mpDoc = &mpDocShell->GetDocument();

        mbOldAutoCalc = mpDoc->GetAutoCalc();
        mpDoc->SetAutoCalc(false);
        mpOldCalcConfig = ScInterpreter::GetGlobalConfig();
        ScCalcConfig aConfig(mpOldCalcConfig);
        aConfig.mnOpenCLMinimumFormulaGroupSize = 20;
        ScInterpreter::SetGlobalConfig(aConfig);

        mpDoc->SetString(ScAddress(0,0,0), "Result:");
    }

    void addTest(const Area &rArea)
    {
        mnTestAreas++;
        (void) mpDocShell->GetDocFunc().InsertTable(mnTestAreas, rArea.msTitle, false, true);

        rArea.addHeader(mpDoc, mnTestAreas);

        for (int i = 0; i < rArea.mnRows; ++i)
            rArea.addRow(mpDoc, i, mnTestAreas);

        mpDoc->SetString(ScAddress(0,1+mnTestAreas-1,0), rArea.msTitle + ":");
        mpDoc->SetString(ScAddress(1,1+mnTestAreas-1,0), rArea.getSummaryFormula(mpDoc, mnTestAreas));

        mpDoc->SetString(ScAddress(1,0,0),
                        "=IF(SUM(" +
                        ScRange(ScAddress(1,1,0),
                                ScAddress(1,1+mnTestAreas-1,0)).Format(ScRefFlags::VALID|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        ")=0,\"PASS\",\"FAIL\")");
    }
};

struct Op : Area
{
    OUString msOp;
    double mnRangeLo;
    double mnRangeHi;
    double mnEpsilon;

    Op(const OUString& rTitle,
       const OUString& rOp,
       double nRangeLo, double nRangeHi,
       double nEpsilon) :
        Area(rTitle),
        msOp(rOp),
        mnRangeLo(nRangeLo),
        mnRangeHi(nRangeHi),
        mnEpsilon(nEpsilon)
    {
    }

    virtual ~Op()
    {
    }
};

struct UnOp : Op
{
    double (*mpFun)(double nArg);
    bool (*mpFilterOut)(double nArg);

    UnOp(const OUString& rTitle,
         const OUString& rOp,
         double nRangeLo, double nRangeHi,
         double nEpsilon,
         double (*pFun)(double nArg),
         bool (*pFilterOut)(double nArg) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~UnOp()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const override
    {
        pDoc->SetString(ScAddress(0,0,nTab), "arg");
        pDoc->SetString(ScAddress(1,0,nTab), msOp + "(arg)");
        pDoc->SetString(ScAddress(2,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const override
    {
        double nArg;

        do {
            nArg = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nArg));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nArg);

        pDoc->SetString(ScAddress(1,1+nRow,nTab),
                        "=" + msOp + "(" + ScAddress(0,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) + ")");

        pDoc->SetValue(ScAddress(2,1+nRow,nTab), mpFun(nArg));

        if (mnEpsilon < 0)
        {
            // relative epsilon
            pDoc->SetString(ScAddress(3,1+nRow,nTab),
                            "=IF(ABS((" + ScAddress(1,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                            "-" + ScAddress(2,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                            ")/" + ScAddress(2,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                            ")<=" + OUString::number(-mnEpsilon) +
                            ",0,1)");
        }
        else
        {
            // absolute epsilon
            pDoc->SetString(ScAddress(3,1+nRow,nTab),
                            "=IF(ABS(" + ScAddress(1,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                            "-" + ScAddress(2,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                            ")<=" + OUString::number(mnEpsilon) +
                            ",0,1)");
        }
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const override
    {
        return "=SUM(" +
            ScRange(ScAddress(3,1,nTab),
                    ScAddress(3,1+mnRows-1,nTab)).Format(ScRefFlags::VALID|ScRefFlags::TAB_3D|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID|ScRefFlags::TAB_VALID, pDoc) +
            ")";
    }
};

struct BinOp : Op
{
    double (*mpFun)(double nLhs, double nRhs);
    bool (*mpFilterOut)(double nLhs, double nRhs);

    BinOp(const OUString& rTitle,
          const OUString& rOp,
          double nRangeLo, double nRangeHi,
          double nEpsilon,
          double (*pFun)(double nLhs, double nRhs),
          bool (*pFilterOut)(double nLhs, double nRhs) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~BinOp()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const override
    {
        pDoc->SetString(ScAddress(0,0,nTab), "lhs");
        pDoc->SetString(ScAddress(1,0,nTab), "rhs");
        pDoc->SetString(ScAddress(2,0,nTab), "lhs" + msOp + "rhs");
        pDoc->SetString(ScAddress(3,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const override
    {
        double nLhs, nRhs;

        do {
            nLhs = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
            nRhs = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nLhs, nRhs));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nLhs);
        pDoc->SetValue(ScAddress(1,1+nRow,nTab), nRhs);

        pDoc->SetString(ScAddress(2,1+nRow,nTab),
                        "=" + ScAddress(0,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        msOp + ScAddress(1,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID));

        pDoc->SetValue(ScAddress(3,1+nRow,nTab), mpFun(nLhs, nRhs));

        pDoc->SetString(ScAddress(4,1+nRow,nTab),
                        "=IF(ABS(" + ScAddress(2,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "-" + ScAddress(3,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        ")<=" + OUString::number(mnEpsilon) +
                        ",0,1)");
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const override
    {
        return "=SUM(" +
            ScRange(ScAddress(4,1,nTab),
                    ScAddress(4,1+mnRows-1,nTab)).Format(ScRefFlags::VALID|ScRefFlags::TAB_3D|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID|ScRefFlags::TAB_VALID, pDoc) +
            ")";
    }
};

struct Round : Area
{
    Round() :
        Area("Round")
    {
    }

    virtual ~Round()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const override
    {
        pDoc->SetString(ScAddress(0,0,nTab), "x");
        pDoc->SetString(ScAddress(1,0,nTab), "n");
        pDoc->SetString(ScAddress(2,0,nTab), "ROUND(x,n)");
        pDoc->SetString(ScAddress(3,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const override
    {
        const double nX(comphelper::rng::uniform_real_distribution(0, 100));
        const int nN(comphelper::rng::uniform_int_distribution(1, 10));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nX);
        pDoc->SetValue(ScAddress(1,1+nRow,nTab), nN);

        pDoc->SetString(ScAddress(2,1+nRow,nTab),
                        "=ROUND(" + ScAddress(0,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "," + ScAddress(1,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        ")");

        pDoc->SetValue(ScAddress(3,1+nRow,nTab), ::rtl::math::round(nX, (short) nN));

        pDoc->SetString(ScAddress(4,1+nRow,nTab),
                        "=IF(ABS(" + ScAddress(2,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "-" + ScAddress(3,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        ")<=3e-10"
                        ",0,1)");
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const override
    {
        return "=SUM(" +
            ScRange(ScAddress(4,1,nTab),
                    ScAddress(4,1+mnRows-1,nTab)).Format(ScRefFlags::VALID|ScRefFlags::TAB_3D|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID|ScRefFlags::TAB_VALID, pDoc) +
            ")";
    }

};

struct Normdist : Area
{
    Normdist() :
        Area("Normdist")
    {
    }

    virtual ~Normdist()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const override
    {
        pDoc->SetString(ScAddress(0,0,nTab), "num");
        pDoc->SetString(ScAddress(1,0,nTab), "avg");
        pDoc->SetString(ScAddress(2,0,nTab), "stdev");
        pDoc->SetString(ScAddress(3,0,nTab), "type");
        pDoc->SetString(ScAddress(4,0,nTab), "NORMDIST(num,avg,stdev,type)");
        pDoc->SetString(ScAddress(5,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const override
    {
        const double nNum(comphelper::rng::uniform_real_distribution(0, 100));
        const double nAvg(comphelper::rng::uniform_real_distribution(0, 100));
        const double nStDev(comphelper::rng::uniform_real_distribution(1, 10));
        const int nType(comphelper::rng::uniform_int_distribution(0, 1));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nNum);
        pDoc->SetValue(ScAddress(1,1+nRow,nTab), nAvg);
        pDoc->SetValue(ScAddress(2,1+nRow,nTab), nStDev);
        pDoc->SetValue(ScAddress(3,1+nRow,nTab), nType);

        pDoc->SetString(ScAddress(4,1+nRow,nTab),
                        "=NORMDIST(" + ScAddress(0,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "," + ScAddress(1,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "," + ScAddress(2,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "," + ScAddress(3,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        ")");

        if (nType == 1)
            pDoc->SetValue(ScAddress(5,1+nRow,nTab), ScInterpreter::integralPhi((nNum-nAvg)/nStDev));
        else
            pDoc->SetValue(ScAddress(5,1+nRow,nTab), ScInterpreter::phi((nNum-nAvg)/nStDev)/nStDev);

        pDoc->SetString(ScAddress(6,1+nRow,nTab),
                        "=IF(ABS(" + ScAddress(4,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        "-" + ScAddress(5,1+nRow,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                        ")<=3e-10"
                        ",0,1)");
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const override
    {
        return "=SUM(" +
            ScRange(ScAddress(6,1,nTab),
                    ScAddress(6,1+mnRows-1,nTab)).Format(ScRefFlags::VALID|ScRefFlags::TAB_3D|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID|ScRefFlags::TAB_VALID, pDoc) +
            ")";
    }

};

struct Reduction : Op
{
    double mnAccumInitial;
    double (*mpFun)(double nAccum, double nArg, const Reduction& rReduction);
    bool (*mpFilterOut)(double nArg);

    Reduction(const OUString& rTitle,
              const OUString& rOp,
              double nAccumInitial,
              double nRangeLo, double nRangeHi,
              double nEpsilon,
              double (*pFun)(double nAccum, double nArg, const Reduction& rReduction),
              bool (*pFilterOut)(double nArg) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mnAccumInitial(nAccumInitial),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~Reduction()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const override
    {
        pDoc->SetString(ScAddress(0,0,nTab), "x");
        pDoc->SetString(ScAddress(1,0,nTab), msOp);
        pDoc->SetString(ScAddress(2,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const override
    {
        double nArg;

        do {
            nArg = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nArg));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nArg);

        if (nRow >= mnRows/2-1)
        {
            pDoc->SetString(ScAddress(1,1+nRow-mnRows/2+1,nTab),
                            "=" + msOp + "(" +
                            ScRange(ScAddress(0,1+nRow-mnRows/2+1,nTab),
                                    ScAddress(0,1+nRow,nTab)).Format(ScRefFlags::VALID|ScRefFlags::TAB_3D|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                            ")");

            double nAccum(mnAccumInitial);
            for (int i = 0; i < mnRows/2; i++)
                nAccum = mpFun(nAccum, pDoc->GetValue(ScAddress(0,1+nRow-mnRows/2+i+1,nTab)), *this);

            pDoc->SetValue(ScAddress(2,1+nRow-mnRows/2+1,nTab), nAccum);

            if (mnEpsilon != 0)
                pDoc->SetString(ScAddress(3,1+nRow-mnRows/2+1,nTab),
                                "=IF(ABS(" + ScAddress(1,1+nRow-mnRows/2+1,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                                "-" + ScAddress(2,1+nRow-mnRows/2+1,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                                ")<=" + OUString::number(mnEpsilon) +
                                ",0,1)");
            else
                pDoc->SetString(ScAddress(3,1+nRow-mnRows/2+1,nTab),
                                "=IF(" + ScAddress(1,1+nRow-mnRows/2+1,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                                "=" + ScAddress(2,1+nRow-mnRows/2+1,nTab).Format(ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID) +
                                ",0,1)");
        }
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const override
    {
        return "=SUM(" +
            ScRange(ScAddress(3,1+0,nTab),
                    ScAddress(3,1+mnRows-mnRows/2-1,nTab)).Format(ScRefFlags::VALID|ScRefFlags::TAB_3D|ScRefFlags::COL_VALID|ScRefFlags::ROW_VALID|ScRefFlags::TAB_VALID, pDoc) +
            ")";
    }
};

}

IMPL_STATIC_LINK_TYPED(ScCalcOptionsDialog, TestClickHdl, Button*, pButton, void)
{
    pButton->Disable();

    // Automatically test the current implementation of OpenCL. If it
    // seems good, whitelist it. If it seems bad, blacklist it.

    std::unique_ptr<OpenCLTester> xTestDocument(new OpenCLTester());

    xTestDocument->addTest(BinOp("Plus", "+", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs + nRhs;
                                 }));

    xTestDocument->addTest(BinOp("Minus", "-", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs - nRhs;
                                 }));

    xTestDocument->addTest(BinOp("Times", "*", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs * nRhs;
                                 }));

    xTestDocument->addTest(BinOp("Divided", "/", -1000, 1000, 3e-10,
                                 [] (double nLhs, double nRhs)
                                 {
                                     return nLhs / nRhs;
                                 },
                                 [] (double, double nRhs)
                                 {
                                     return (nRhs == 0);
                                 }));

    xTestDocument->addTest(UnOp("Sin", "SIN", -10, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return sin(nArg);
                                }));

    xTestDocument->addTest(UnOp("Cos", "COS", -10, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return cos(nArg);
                                }));

    xTestDocument->addTest(UnOp("Tan", "TAN", 0, 10, -3e-10,
                                [] (double nArg)
                                {
                                    return tan(nArg);
                                },
                                [] (double nArg)
                                {
                                    return (std::fmod(nArg, M_PI) == M_PI/2);
                                }));

    xTestDocument->addTest(UnOp("Atan", "ATAN", -10, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return atan(nArg);
                                }));

    xTestDocument->addTest(UnOp("Sqrt", "SQRT", 0, 1000, 3e-10,
                                [] (double nArg)
                                {
                                    return sqrt(nArg);
                                }));

    xTestDocument->addTest(UnOp("Exp", "EXP", 0, 10, 3e-10,
                                [] (double nArg)
                                {
                                    return exp(nArg);
                                }));

    xTestDocument->addTest(UnOp("Ln", "LN", 0, 1000, 3e-10,
                                [] (double nArg)
                                {
                                    return log(nArg);
                                },
                                [] (double nArg)
                                {
                                    return (nArg == 0);
                                }));

    xTestDocument->addTest(UnOp("NormSInv", "NORMSINV", 0, 1, 3e-10,
                                [] (double nArg)
                                {
                                    return ScInterpreter::gaussinv(nArg);
                                }));

    xTestDocument->addTest(Round());

    xTestDocument->addTest(Normdist());

    xTestDocument->addTest(Reduction("Sum", "SUM", 0, -1000, 1000, 3e-10,
                                     [] (double nAccum, double nArg, const Reduction&)
                                     {
                                         return (nAccum + nArg);
                                     }));

    xTestDocument->addTest(Reduction("Average", "AVERAGE", 0, -1000, 1000, 3e-10,
                                     [] (double nAccum, double nArg, const Reduction& rReduction)
                                     {
                                         return (nAccum + nArg/(rReduction.mnRows/2));
                                     }));

    xTestDocument->addTest(Reduction("Product", "PRODUCT", 1, 0.1, 2.5, 3e-10,
                                     [] (double nAccum, double nArg, const Reduction&)
                                     {
                                         return (nAccum * nArg);
                                     }));

    xTestDocument->addTest(Reduction("Min", "MIN", DBL_MAX, -1000, 1000, 0,
                                     [] (double nAccum, double nArg, const Reduction&)
                                     {
                                         return std::min(nAccum, nArg);
                                     }));

    xTestDocument->addTest(Reduction("Max", "MAX", -DBL_MAX, -1000, 1000, 0,
                                     [] (double nAccum, double nArg, const Reduction&)
                                     {
                                         return std::max(nAccum, nArg);
                                     }));

    xTestDocument->mpDoc->CalcAll();
    ScInterpreter::SetGlobalConfig(xTestDocument->mpOldCalcConfig);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
