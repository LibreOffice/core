/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <formulalogger.hxx>
#include <formulacell.hxx>
#include <tokenarray.hxx>
#include <document.hxx>
#include <docsh.hxx>
#include <tokenstringcontext.hxx>
#include <address.hxx>
#include <interpre.hxx>

#include <osl/file.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>
#include <formula/vectortoken.hxx>
#include <rtl/ustrbuf.hxx>

#include <cstdlib>
#include <utility>

namespace sc {

namespace {

std::unique_ptr<osl::File> initFile()
{
    const char* pPath = std::getenv("LIBO_FORMULA_LOG_FILE");
    if (!pPath)
        return nullptr;

    // Support both file:///... and system file path notations.
    OUString aPath = OUString::createFromAscii(pPath);
    INetURLObject aURL;
    aURL.SetSmartURL(aPath);
    aPath = aURL.GetMainURL(INetURLObject::DecodeMechanism::NONE);

    return std::make_unique<osl::File>(aPath);
}

ScRefFlags getRefFlags( const ScAddress& rCellPos, const ScAddress& rRefPos )
{
    ScRefFlags eFlags = ScRefFlags::VALID;
    if (rCellPos.Tab() != rRefPos.Tab())
        eFlags |= ScRefFlags::TAB_3D;
    return eFlags;
}

}

FormulaLogger& FormulaLogger::get()
{
    static FormulaLogger aLogger;
    return aLogger;
}

struct FormulaLogger::GroupScope::Impl
{
    FormulaLogger& mrLogger;
    const ScDocument& mrDoc;

    OUString maPrefix;
    std::vector<OUString> maMessages;

    bool mbCalcComplete;
    bool mbOutputEnabled;

    Impl( FormulaLogger& rLogger, OUString aPrefix, const ScDocument& rDoc,
        const ScFormulaCell& rCell, bool bOutputEnabled ) :
        mrLogger(rLogger), mrDoc(rDoc), maPrefix(std::move(aPrefix)),
        mbCalcComplete(false), mbOutputEnabled(bOutputEnabled)
    {
        ++mrLogger.mnNestLevel;

        if (!mbOutputEnabled)
            return;

        sc::TokenStringContext aCxt(rDoc, rDoc.GetGrammar());
        OUString aFormula = rCell.GetCode()->CreateString(aCxt, rCell.aPos);

        mrLogger.write(maPrefix);
        mrLogger.writeNestLevel();

        mrLogger.writeAscii("-- enter (formula='");
        mrLogger.write(aFormula);
        mrLogger.writeAscii("', size=");
        mrLogger.write(rCell.GetSharedLength());
        mrLogger.writeAscii(")\n");
    }

    ~Impl()
    {
        if (mbOutputEnabled)
        {
            for (const OUString& rMsg : maMessages)
            {
                mrLogger.write(maPrefix);
                mrLogger.writeNestLevel();
                mrLogger.writeAscii("   * ");
                mrLogger.write(rMsg);
                mrLogger.writeAscii("\n");
            }

            mrLogger.write(maPrefix);
            mrLogger.writeNestLevel();
            mrLogger.writeAscii("-- exit (");
            if (mbCalcComplete)
                mrLogger.writeAscii("calculation complete");
            else
                mrLogger.writeAscii("without calculation");

            mrLogger.writeAscii(")\n");

            mrLogger.sync();
        }

        --mrLogger.mnNestLevel;
    }
};

FormulaLogger::GroupScope::GroupScope(
    FormulaLogger& rLogger, const OUString& rPrefix, const ScDocument& rDoc,
    const ScFormulaCell& rCell, bool bOutputEnabled ) :
    mpImpl(std::make_unique<Impl>(rLogger, rPrefix, rDoc, rCell, bOutputEnabled)) {}

FormulaLogger::GroupScope::GroupScope(GroupScope&& r) noexcept : mpImpl(std::move(r.mpImpl)) {}

FormulaLogger::GroupScope::~GroupScope() {}

void FormulaLogger::GroupScope::addMessage( const OUString& rMsg )
{
    mpImpl->maMessages.push_back(rMsg);
}

void FormulaLogger::GroupScope::addRefMessage(
    const ScAddress& rCellPos, const ScAddress& rRefPos, size_t nLen,
    const formula::VectorRefArray& rArray )
{
    ScRange aRefRange(rRefPos);
    aRefRange.aEnd.IncRow(nLen-1);
    OUString aRangeStr = aRefRange.Format(mpImpl->mrDoc, getRefFlags(rCellPos, rRefPos));

    std::u16string_view aMsg;
    if (rArray.mpNumericArray)
    {
        if (rArray.mpStringArray)
        {
            // mixture of numeric and string cells.
            aMsg = u"numeric and string";
        }
        else
        {
            // numeric cells only.
            aMsg = u"numeric only";
        }
    }
    else
    {
        if (rArray.mpStringArray)
        {
            // string cells only.
            aMsg = u"string only";
        }
        else
        {
            // empty cells.
            aMsg = u"empty";
        }
    }

    mpImpl->maMessages.push_back(aRangeStr + ": " + aMsg);
}

void FormulaLogger::GroupScope::addRefMessage(
    const ScAddress& rCellPos, const ScAddress& rRefPos, size_t nLen,
    const std::vector<formula::VectorRefArray>& rArrays )
{
    ScAddress aPos(rRefPos); // copy
    for (const formula::VectorRefArray& rArray : rArrays)
    {
        addRefMessage(rCellPos, aPos, nLen, rArray);
        aPos.IncCol();
    }
}

void FormulaLogger::GroupScope::addRefMessage(
    const ScAddress& rCellPos, const ScAddress& rRefPos,
    const formula::FormulaToken& rToken )
{
    OUString aPosStr = rRefPos.Format(getRefFlags(rCellPos, rRefPos), &mpImpl->mrDoc);
    std::u16string_view aMsg;
    switch (rToken.GetType())
    {
        case formula::svDouble:
            aMsg = u"numeric value";
            break;
        case formula::svString:
            aMsg = u"string value";
            break;
        default:
            aMsg = u"unknown value";
    }

    mpImpl->maMessages.push_back(aPosStr + ": " + aMsg);
}

void FormulaLogger::GroupScope::addGroupSizeThresholdMessage( const ScFormulaCell& rCell )
{
    OUString aBuf = "group length below minimum threshold ("
        + OUString::number(rCell.GetWeight())
        + " < "
        + OUString::number(ScInterpreter::GetGlobalConfig().mnOpenCLMinimumFormulaGroupSize)
        + ")";
    mpImpl->maMessages.push_back(aBuf);
}

void FormulaLogger::GroupScope::setCalcComplete()
{
    mpImpl->mbCalcComplete = true;
    addMessage(u"calculation performed"_ustr);
}

FormulaLogger::FormulaLogger()
{
    mpLogFile = initFile();

    if (!mpLogFile)
        return;

    osl::FileBase::RC eRC = mpLogFile->open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);

    if (eRC == osl::FileBase::E_EXIST)
    {
        eRC = mpLogFile->open(osl_File_OpenFlag_Write);

        if (eRC != osl::FileBase::E_None)
        {
            // Failed to open an existing log file.
            mpLogFile.reset();
            return;
        }

        if (mpLogFile->setPos(osl_Pos_End, 0) != osl::FileBase::E_None)
        {
            // Failed to set the position to the end of the file.
            mpLogFile.reset();
            return;
        }
    }
    else if (eRC != osl::FileBase::E_None)
    {
        // Failed to create a new file.
        mpLogFile.reset();
        return;
    }

    // Output the header information.
    writeAscii("---\n");
    writeAscii("OpenCL: ");
    writeAscii(ScCalcConfig::isOpenCLEnabled() ? "enabled\n" : "disabled\n");
    writeAscii("---\n");

    sync();
}

FormulaLogger::~FormulaLogger()
{
    if (mpLogFile)
        mpLogFile->close();
}

void FormulaLogger::writeAscii( const char* s )
{
    if (!mpLogFile)
        return;

    sal_uInt64 nBytes;
    mpLogFile->write(s, strlen(s), nBytes);
}

void FormulaLogger::writeAscii( const char* s, size_t n )
{
    if (!mpLogFile)
        return;

    sal_uInt64 nBytes;
    mpLogFile->write(s, n, nBytes);
}

void FormulaLogger::write( std::u16string_view ou )
{
    OString s = OUStringToOString(ou, RTL_TEXTENCODING_UTF8);
    writeAscii(s.getStr(), s.getLength());
}

void FormulaLogger::write( sal_Int32 n )
{
    OString s = OString::number(n);
    writeAscii(s.getStr(), s.getLength());
}

void FormulaLogger::sync()
{
    if (!mpLogFile)
        return;

    mpLogFile->sync();
}

void FormulaLogger::writeNestLevel()
{
    // Write the nest level, but keep it only 1-character length to avoid
    // messing up the spacing.
    if (mnNestLevel < 10)
        write(mnNestLevel);
    else
        writeAscii("!");

    writeAscii(": ");
    for (sal_Int32 i = 1; i < mnNestLevel; ++i)
        writeAscii("   ");
}

FormulaLogger::GroupScope FormulaLogger::enterGroup(
    const ScDocument& rDoc, const ScFormulaCell& rCell )
{
    // Get the file name if available.
    const ScDocShell* pShell = rDoc.GetDocumentShell();
    const SfxMedium* pMedium = pShell ? pShell->GetMedium() : nullptr;
    OUString aName;
    if (pMedium)
        aName = pMedium->GetURLObject().GetLastName();
    if (aName.isEmpty())
        aName = "-"; // unsaved document.

    OUString aGroupPrefix = aName + ": formula-group: " +
        rCell.aPos.Format(ScRefFlags::VALID | ScRefFlags::TAB_3D, &rDoc, rDoc.GetAddressConvention()) + ": ";

    bool bOutputEnabled = mpLastGroup != rCell.GetCellGroup().get();
    mpLastGroup = rCell.GetCellGroup().get();

    return GroupScope(*this, aGroupPrefix, rDoc, rCell, bOutputEnabled);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
