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
#include <tokenstringcontext.hxx>

#include <o3tl/make_unique.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <tools/urlobj.hxx>

namespace sc {

FormulaLogger& FormulaLogger::get()
{
    static FormulaLogger aLogger;
    return aLogger;
}

struct FormulaLogger::GroupScope::Impl
{
    FormulaLogger& mrLogger;

    OUString maPrefix;
    std::vector<OUString> maMessages;

    bool mbCalcComplete = false;

    Impl( FormulaLogger& rLogger, const OUString& rPrefix, const ScDocument& rDoc, const ScFormulaCell& rCell ) :
        mrLogger(rLogger), maPrefix(rPrefix)
    {
        ++mrLogger.mnNestLevel;

        sc::TokenStringContext aCxt(&rDoc, rDoc.GetGrammar());
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

        mrLogger.mpLogFile->sync();

        --mrLogger.mnNestLevel;
    }
};

FormulaLogger::GroupScope::GroupScope(
    FormulaLogger& rLogger, const OUString& rPrefix, const ScDocument& rDoc, const ScFormulaCell& rCell ) :
    mpImpl(o3tl::make_unique<Impl>(rLogger, rPrefix, rDoc, rCell)) {}

FormulaLogger::GroupScope::GroupScope( GroupScope&& r ) : mpImpl(std::move(r.mpImpl)) {}

FormulaLogger::GroupScope::~GroupScope() {}

void FormulaLogger::GroupScope::addMessage( const OUString& rMsg )
{
    mpImpl->maMessages.push_back(rMsg);
}

void FormulaLogger::GroupScope::setCalcComplete()
{
    mpImpl->mbCalcComplete = true;
    addMessage("calculation performed");
}

FormulaLogger::FormulaLogger() : mpLogFile(o3tl::make_unique<osl::File>("file:///home/kohei/tmp/formula.log"))
{
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

    sal_uInt64 nBytes;
    mpLogFile->write("---\n", 4, nBytes);
    mpLogFile->sync();
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

void FormulaLogger::write( const OUString& ou )
{
    OString s = rtl::OUStringToOString(ou, RTL_TEXTENCODING_UTF8).getStr();
    writeAscii(s.getStr(), s.getLength());
}

void FormulaLogger::write( sal_Int32 n )
{
    OString s = OString::number(n);
    writeAscii(s.getStr(), s.getLength());
}

void FormulaLogger::writeNestLevel()
{
    // Write the nest level, but keep it only 1-character length to avoid
    // messing up the spacing.
    if (mnNestLevel < 10)
        write(mnNestLevel);
    else
        writeAscii("!");

    writeAscii(":");
    for (sal_Int32 i = 0; i < mnNestLevel; ++i)
        writeAscii("   ");
}

FormulaLogger::GroupScope FormulaLogger::enterGroup(
    const ScDocument& rDoc, const ScFormulaCell& rCell )
{
    // Get the file name if available.
    const SfxObjectShell* pShell = rDoc.GetDocumentShell();
    const SfxMedium* pMedium = pShell->GetMedium();
    OUString aName = pMedium->GetURLObject().GetLastName();
    if (aName.isEmpty())
        aName = "-"; // unsaved document.

    OUString aGroupPrefix = aName;

    aGroupPrefix += ": formula-group: ";
    aGroupPrefix += rCell.aPos.Format(ScRefFlags::VALID | ScRefFlags::TAB_3D, &rDoc, rDoc.GetAddressConvention());
    aGroupPrefix += ": ";

    return GroupScope(*this, aGroupPrefix, rDoc, rCell);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
