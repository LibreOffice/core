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
        case 0:
        default:
            ;
    }

    return formula::FormulaGrammar::CONV_UNSPECIFIED;
}

}

ScCalcOptionsDialog::ScCalcOptionsDialog(vcl::Window* pParent, const ScCalcConfig& rConfig)
    : ModalDialog(pParent, "FormulaCalculationOptions",
        "modules/scalc/ui/formulacalculationoptions.ui")
    , maConfig(rConfig)
    , mbSelectedEmptyStringAsZero(rConfig.mbEmptyStringAsZero)
{
    get(mpTestButton, "test");
    get(mpOpenclInfoList, "opencl_list");
    get(mpBtnAutomaticSelectionTrue, "automatic_select_true");
    get(mpBtnAutomaticSelectionFalse, "automatic_select_false");
    get(mpFtFrequency, "frequency");
    get(mpFtComputeUnits, "compute_units");
    get(mpFtMemory, "memory");

    get(mpConversion,"comboConversion");
    mpConversion->SelectEntryPos(rConfig.meStringConversion, true);
    mpConversion->SetSelectHdl(LINK(this, ScCalcOptionsDialog, ConversionModifiedHdl));

    get(mpEmptyAsZero,"checkEmptyAsZero");
    mpEmptyAsZero->Check(rConfig.mbEmptyStringAsZero);
    mpEmptyAsZero->SetClickHdl(LINK(this, ScCalcOptionsDialog, AsZeroModifiedHdl));

    get(mpSyntax,"comboSyntaxRef");
    mpSyntax->SelectEntryPos(rConfig.meStringRefAddressSyntax);
    mpSyntax->SetSelectHdl(LINK(this, ScCalcOptionsDialog, SyntaxModifiedHdl));

    get(mpUseOpenCL,"CBUseOpenCL");
    mpUseOpenCL->Check(rConfig.mbOpenCLSubsetOnly);
    mpUseOpenCL->SetClickHdl(LINK(this, ScCalcOptionsDialog, CBUseOpenCLHdl));

    get(mpSpinButton,"spinOpenCLSize");
    mpSpinButton->SetValue(rConfig.mnOpenCLMinimumFormulaGroupSize);
    mpSpinButton->SetModifyHdl(LINK(this, ScCalcOptionsDialog, SpinOpenCLMinSizeHdl));

    get(mpEditField, "entry");
    mpEditField->SetText(ScOpCodeSetToSymbolicString(maConfig.maOpenCLSubsetOpCodes));
    mpEditField->set_height_request(4 * mpEditField->GetTextHeight());

    mpEditField->SetModifyHdl(LINK(this, ScCalcOptionsDialog, EditModifiedHdl));

    mpOpenclInfoList->set_height_request(4* mpOpenclInfoList->GetTextHeight());
    mpOpenclInfoList->SetStyle(mpOpenclInfoList->GetStyle() | WB_CLIPCHILDREN | WB_FORCE_MAKEVISIBLE);
    mpOpenclInfoList->SetHighlightRange();
    mpOpenclInfoList->GetParent()->Hide();
    mpOpenclInfoList->SetSelectHdl(LINK(this, ScCalcOptionsDialog, DeviceSelHdl));

    mpBtnAutomaticSelectionTrue->SetToggleHdl(LINK(this, ScCalcOptionsDialog, BtnAutomaticSelectHdl));

    maSoftware = get<vcl::Window>("software")->GetText();

    mpTestButton->SetClickHdl(LINK(this, ScCalcOptionsDialog, TestClickHdl));
}

ScCalcOptionsDialog::~ScCalcOptionsDialog() {}

#if HAVE_FEATURE_OPENCL

void ScCalcOptionsDialog::fillOpenCLList()
{
    mpOpenclInfoList->SetUpdateMode(false);
    mpOpenclInfoList->Clear();
    SvTreeListEntry* pSoftwareEntry = mpOpenclInfoList->InsertEntry(maSoftware);

    OUString aStoredDevice = maConfig.maOpenCLDevice;

    SvTreeListEntry* pSelectedEntry = NULL;

    sc::FormulaGroupInterpreter::fillOpenCLInfo(maPlatformInfo);
    for(std::vector<OpenCLPlatformInfo>::iterator it = maPlatformInfo.begin(),
            itEnd = maPlatformInfo.end(); it != itEnd; ++it)
    {
        for(std::vector<OpenCLDeviceInfo>::iterator
                itr = it->maDevices.begin(), itrEnd = it->maDevices.end(); itr != itrEnd; ++itr)
        {
            OUString aDeviceId = it->maVendor + " " + itr->maName + " " + itr->maDriver;
            SvTreeListEntry* pEntry = mpOpenclInfoList->InsertEntry(aDeviceId);
            if(aDeviceId == aStoredDevice)
            {
                pSelectedEntry = pEntry;
            }
            pEntry->SetUserData(&(*itr));
        }
    }

    mpOpenclInfoList->SetUpdateMode(true);
    mpOpenclInfoList->GetModel()->GetView(0)->SelectAll(false, false);

    if (pSelectedEntry)
        mpOpenclInfoList->Select(pSelectedEntry);
    else if (aStoredDevice == OPENCL_SOFTWARE_DEVICE_CONFIG_NAME)
        mpOpenclInfoList->Select(pSoftwareEntry);

    SelectedDeviceChanged();
}

#endif

void ScCalcOptionsDialog::OpenCLAutomaticSelectionChanged()
{
    bool bValue = mpBtnAutomaticSelectionTrue->IsChecked();
    if(bValue)
        mpOpenclInfoList->Disable();
    else
        mpOpenclInfoList->Enable();

    maConfig.mbOpenCLAutoSelect = bValue;
}

void ScCalcOptionsDialog::SelectedDeviceChanged()
{
#if HAVE_FEATURE_OPENCL
    SvTreeListEntry* pEntry = mpOpenclInfoList->GetModel()->GetView(0)->FirstSelected();
    if(!pEntry)
        return;

    OpenCLDeviceInfo* pInfo = reinterpret_cast<OpenCLDeviceInfo*>(pEntry->GetUserData());
    if(pInfo)
    {
        mpFtFrequency->SetText(OUString::number(pInfo->mnFrequency));
        mpFtComputeUnits->SetText(OUString::number(pInfo->mnComputeUnits));
        mpFtMemory->SetText(OUString::number(pInfo->mnMemory/1024/1024));
    }
    else
    {
        mpFtFrequency->SetText(OUString());
        mpFtComputeUnits->SetText(OUString());
        mpFtMemory->SetText(OUString());
    }

    SvLBoxString* pBoxEntry = dynamic_cast<SvLBoxString*>(pEntry->GetItem(1));
    if (!pBoxEntry)
        return;

    OUString aDevice = pBoxEntry->GetText();
    // use english string for configuration
    if(aDevice == maSoftware)
        aDevice = OPENCL_SOFTWARE_DEVICE_CONFIG_NAME;

    maConfig.maOpenCLDevice = aDevice;
#endif
}

IMPL_LINK(ScCalcOptionsDialog, AsZeroModifiedHdl, CheckBox*, pCheckBox )
{
    maConfig.mbEmptyStringAsZero = pCheckBox->IsChecked();
    return 0;
}

IMPL_LINK(ScCalcOptionsDialog, ConversionModifiedHdl, ListBox*, pConv )
{

  maConfig.meStringConversion = (ScCalcConfig::StringConversion)pConv->GetSelectEntryPos();
    switch (maConfig.meStringConversion)
    {
         case ScCalcConfig::STRING_CONVERSION_AS_ERROR:
                    maConfig.mbEmptyStringAsZero = false;
                    mpEmptyAsZero->Check(false);
                    mpEmptyAsZero->Enable(false);
         break;
         case ScCalcConfig::STRING_CONVERSION_AS_ZERO:
                    maConfig.mbEmptyStringAsZero = true;
                    mpEmptyAsZero->Check(true);
                    mpEmptyAsZero->Enable(false);
         break;
         case ScCalcConfig::STRING_CONVERSION_UNAMBIGUOUS:
         case ScCalcConfig::STRING_CONVERSION_LOCALE_DEPENDENT:
                    // Reset to the value the user selected before.
                    maConfig.mbEmptyStringAsZero = mbSelectedEmptyStringAsZero;
                    mpEmptyAsZero->Enable(true);
         break;
     }
    return 0;
}

IMPL_LINK(ScCalcOptionsDialog, SyntaxModifiedHdl, ListBox*, pSyntax)
{
    maConfig.meStringRefAddressSyntax = toAddressConvention(pSyntax->GetSelectEntryPos());
    return 0;
}

IMPL_LINK(ScCalcOptionsDialog, CBUseOpenCLHdl, CheckBox*, pCheckBox)
{
    maConfig.mbOpenCLSubsetOnly = pCheckBox->IsChecked();
    return 0;
}

IMPL_LINK(ScCalcOptionsDialog, SpinOpenCLMinSizeHdl, NumericField*, pSpin)
{
    maConfig.mnOpenCLMinimumFormulaGroupSize = pSpin->GetValue();
    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, BtnAutomaticSelectHdl)
{
    OpenCLAutomaticSelectionChanged();
    return 0;
}

IMPL_LINK_NOARG(ScCalcOptionsDialog, DeviceSelHdl)
{
    SelectedDeviceChanged();
    return 0;
}

IMPL_LINK(ScCalcOptionsDialog, EditModifiedHdl, Edit*, pCtrl)
{
    maConfig.maOpenCLSubsetOpCodes = ScStringToOpCodeSet(pCtrl->GetText());
    return 0;
}

namespace {

struct Area
{
    OUString msTitle;
    int mnRows;

    Area(const OUString& rTitle, int nRows) :
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

        mpDoc->SetString(ScAddress(0,0,0), "Result:");
    }

    void addTest(const Area &rArea)
    {
        sc::AutoCalcSwitch aACSwitch(*mpDoc, true);

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
                                ScAddress(1,1+mnTestAreas-1,0)).Format(SCA_VALID|SCA_VALID_COL|SCA_VALID_ROW) +
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
        Area(rTitle, 200),
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

    virtual void addHeader(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        pDoc->SetString(ScAddress(0,0,nTab), "arg");
        pDoc->SetString(ScAddress(1,0,nTab), msOp + "(arg)");
        pDoc->SetString(ScAddress(2,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const SAL_OVERRIDE
    {
        double nArg;

        do {
            nArg = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nArg));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nArg);

        pDoc->SetString(ScAddress(1,1+nRow,nTab),
                        "=" + msOp + "(" + ScAddress(0,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) + ")");

        pDoc->SetValue(ScAddress(2,1+nRow,nTab), mpFun(nArg));

        if (mnEpsilon < 0)
        {
            // relative epsilon
            pDoc->SetString(ScAddress(3,1+nRow,nTab),
                            "=IF(ABS((" + ScAddress(1,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            "-" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            ")/" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            ")<=" + OUString::number(-mnEpsilon) +
                            ",0,1)");
        }
        else
        {
            // absolute epsilon
            pDoc->SetString(ScAddress(3,1+nRow,nTab),
                            "=IF(ABS(" + ScAddress(1,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            "-" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                            ")<=" + OUString::number(mnEpsilon) +
                            ",0,1)");
        }
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        return "=SUM(" +
            ScRange(ScAddress(3,1,nTab),
                    ScAddress(3,1+mnRows-1,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB, pDoc) +
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

    virtual void addHeader(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        pDoc->SetString(ScAddress(0,0,nTab), "lhs");
        pDoc->SetString(ScAddress(1,0,nTab), "rhs");
        pDoc->SetString(ScAddress(2,0,nTab), "lhs" + msOp + "rhs");
        pDoc->SetString(ScAddress(3,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const SAL_OVERRIDE
    {
        double nLhs, nRhs;

        do {
            nLhs = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
            nRhs = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nLhs, nRhs));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nLhs);
        pDoc->SetValue(ScAddress(1,1+nRow,nTab), nRhs);

        pDoc->SetString(ScAddress(2,1+nRow,nTab),
                        "=" + ScAddress(0,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                        msOp + ScAddress(1,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW));

        pDoc->SetValue(ScAddress(3,1+nRow,nTab), mpFun(nLhs, nRhs));

        pDoc->SetString(ScAddress(4,1+nRow,nTab),
                        "=IF(ABS(" + ScAddress(2,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                        "-" + ScAddress(3,1+nRow,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                        ")<=" + OUString::number(mnEpsilon) +
                        ",0,1)");
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        return "=SUM(" +
            ScRange(ScAddress(4,1,nTab),
                    ScAddress(4,1+mnRows-1,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB, pDoc) +
            ")";
    }
};

struct Reduction : Op
{
    int mnNum;
    double mnAccumInitial;
    double (*mpFun)(double nAccum, double nArg);
    bool (*mpFilterOut)(double nArg);

    Reduction(const OUString& rTitle,
              const OUString& rOp,
              int nNum,
              double nAccumInitial,
              double nRangeLo, double nRangeHi,
              double nEpsilon,
              double (*pFun)(double nAccum, double nArg),
              bool (*pFilterOut)(double nArg) = nullptr) :
        Op(rTitle, rOp, nRangeLo, nRangeHi, nEpsilon),
        mnNum(nNum),
        mnAccumInitial(nAccumInitial),
        mpFun(pFun),
        mpFilterOut(pFilterOut)
    {
    }

    virtual ~Reduction()
    {
    }

    virtual void addHeader(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        pDoc->SetString(ScAddress(0,0,nTab), "x");
        pDoc->SetString(ScAddress(1,0,nTab), msOp);
        pDoc->SetString(ScAddress(2,0,nTab), "expected");
    }

    virtual void addRow(ScDocument *pDoc, int nRow, int nTab) const SAL_OVERRIDE
    {
        double nArg;

        do {
            nArg = comphelper::rng::uniform_real_distribution(mnRangeLo, mnRangeHi);
        } while (mpFilterOut != nullptr && mpFilterOut(nArg));

        pDoc->SetValue(ScAddress(0,1+nRow,nTab), nArg);

        if (nRow >= mnNum-1)
        {
            pDoc->SetString(ScAddress(1,1+nRow-mnNum+1,nTab),
                            "=" + msOp + "(" +
                            ScRange(ScAddress(0,1+nRow-mnNum+1,nTab),
                                    ScAddress(0,1+nRow,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW) +
                            ")");

            double nAccum(mnAccumInitial);
            for (int i = 0; i < mnNum; i++)
                nAccum = mpFun(nAccum, pDoc->GetValue(ScAddress(0,1+nRow-mnNum+i+1,nTab)));

            pDoc->SetValue(ScAddress(2,1+nRow-mnNum+1,nTab), nAccum);

            if (mnEpsilon != 0)
                pDoc->SetString(ScAddress(3,1+nRow-mnNum+1,nTab),
                                "=IF(ABS(" + ScAddress(1,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                "-" + ScAddress(2,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                ")<=" + OUString::number(mnEpsilon) +
                                ",0,1)");
            else
                pDoc->SetString(ScAddress(3,1+nRow-mnNum+1,nTab),
                                "=IF(" + ScAddress(1,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                "=" + ScAddress(2,1+nRow-mnNum+1,nTab).Format(SCA_VALID_COL|SCA_VALID_ROW) +
                                ",0,1)");
        }
    }

    virtual OUString getSummaryFormula(ScDocument *pDoc, int nTab) const SAL_OVERRIDE
    {
        return "=SUM(" +
            ScRange(ScAddress(3,1+0,nTab),
                    ScAddress(3,1+mnRows-mnNum-1,nTab)).Format(SCA_VALID|SCA_TAB_3D|SCA_VALID_COL|SCA_VALID_ROW|SCA_VALID_TAB, pDoc) +
            ")";
    }
};

}

IMPL_LINK( ScCalcOptionsDialog, TestClickHdl, PushButton*, )
{
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

    xTestDocument->addTest(Reduction("Sum", "SUM", 100, 0, -1000, 1000, 3e-10,
                                     [] (double nAccum, double nArg)
                                     {
                                         return (nAccum + nArg);
                                     }));

    xTestDocument->addTest(Reduction("Average", "AVERAGE", 100, 0, -1000, 1000, 3e-10,
                                     [] (double nAccum, double nArg)
                                     {
                                         return (nAccum + nArg/100.);
                                     }));

    xTestDocument->addTest(Reduction("Product", "PRODUCT", 100, 1, 0.1, 2.5, 3e-10,
                                     [] (double nAccum, double nArg)
                                     {
                                         return (nAccum * nArg);
                                     }));

    xTestDocument->addTest(Reduction("Min", "MIN", 100, DBL_MAX, -1000, 1000, 0,
                                     [] (double nAccum, double nArg)
                                     {
                                         return std::min(nAccum, nArg);
                                     }));

    xTestDocument->addTest(Reduction("Max", "MAX", 100, -DBL_MAX, -1000, 1000, 0,
                                     [] (double nAccum, double nArg)
                                     {
                                         return std::max(nAccum, nArg);
                                     }));

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
