/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <docsh.hxx>
#include <osl/file.hxx>
#include <vcl/scheduler.hxx>
#include <unotxdoc.hxx>
#include <swdtflvr.hxx>

static std::vector<OUString> aIgnoreFiles{
    "ofz18563.docx",
    "threadedException.fodt",
    "Encrypted_LO_Standard_abc.docx",
    "Encrypted_MSO2007_abc.docx",
    "Encrypted_MSO2010_abc.docx",
    "Encrypted_MSO2013_abc.docx",
    "math-malformed_xml.docx",
    "groupshape-relsize.docx", // tdf#146848
    "line-rotation.docx", // tdf#146216
    "n779627.docx", // tdf#146216
    "n820504.docx", // tdf#146216
    "tdf114217.docx", // tdf#146216
    "fdo69893.odt", // tdf#146216
    "tdf104425.odt", // tdf#146216
    "tdf112741.fodt", // tdf#146216
    "tdf115065.odt", // tdf#146216
    "tdf123102.odt", // tdf#146216
    "tdf126504.odt", // tdf#146216
    "tdf126784_distributeSelectedColumns.odt", // tdf#146216
    "tdf134227.docx", // tdf#146216
    "tdf134931.odt", // tdf#146216
    "tdf138482.docx", // tdf#146216
    "tdf143244.odt", // tdf#146216
    "try2.fodt", // tdf#146216
    "tdf100691.fodt", // tdf#146859
    "tdf124904.fodt", // tdf#146859
    "tdf61154.fodt", // tdf#146859
    "tdf77014.odt", //tdf#146848
    "tdf118693.docx", // For some reason it makes CppunitTest to crash
};

static std::vector<OUString> aDirNames{
    "/sw/qa/extras/uiwriter/data/",
    "/sw/qa/extras/ooxmlimport/data/",
};

static std::vector<OUString> aExtensions{
    ".odt", ".fodt", ".odm", ".doc", ".docx", ".dotx", ".dotx",
};

class basicActions : public SwModelTestBase
{
public:
    void recursiveScan(const OUString& rURL, std::set<OUString>& rFiles);
    void getFiles(std::set<OUString>& rFiles);
};

void basicActions::recursiveScan(const OUString& rURL, std::set<OUString>& rFiles)
{
    osl::Directory aDir(rURL);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(OUString("Failed to open directory " + rURL).toUtf8().getStr(),
                                 osl::FileBase::E_None, aDir.open());
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL | osl_FileStatus_Mask_FileName
                                | osl_FileStatus_Mask_Type);
    std::set<OUString> aDirs;
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        OUString sURL = aFileStatus.getFileURL();
        if (aFileStatus.getFileType() == osl::FileStatus::Directory)
        {
            aDirs.insert(sURL);
        }
        else
        {
            OUString sName = aFileStatus.getFileName();
            if (std::find(aIgnoreFiles.begin(), aIgnoreFiles.end(), sName) != aIgnoreFiles.end())
                continue;

            for (auto const& sExtension : aExtensions)
            {
                if (sName.endsWith(sExtension))
                {
                    rFiles.insert(sURL);
                    break;
                }
            }
        }
    }

    for (auto const& sURL : aDirs)
    {
        recursiveScan(sURL, rFiles);
    }
}

void basicActions::getFiles(std::set<OUString>& rFiles)
{
    for (auto const& sURL : aDirNames)
    {
        recursiveScan(m_directories.getURLFromSrc(sURL), rFiles);
    }
}

CPPUNIT_TEST_FIXTURE(basicActions, selectAllCopyPasteUndo)
{
    std::set<OUString> aFiles;
    getFiles(aFiles);
    sal_uInt32 nCount = 0;

    for (auto const& sFile : aFiles)
    {
        nCount += 1;
        fprintf(stderr, "%i/%li. Running %s\n", nCount, aFiles.size(),
                OUStringToOString(sFile, RTL_TEXTENCODING_UTF8).getStr());

        loadURL(sFile, nullptr);

        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);

        SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetDoc()->GetDocShell()->GetWrtShell();

        dispatchCommand(mxComponent, ".uno:SelectAll", {});
        dispatchCommand(mxComponent, ".uno:SelectAll", {});
        dispatchCommand(mxComponent, ".uno:SelectAll", {});
        Scheduler::ProcessEventsToIdle();

        rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
        xTransfer->Copy();

        TransferableDataHelper aHelper(xTransfer);
        SwTransferable::Paste(*pWrtShell, aHelper);

        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();
    }
}

CPPUNIT_TEST_FIXTURE(basicActions, selectAllCutPasteUndo)
{
    std::set<OUString> aFiles;
    getFiles(aFiles);

    for (auto const& sFile : aFiles)
    {
        fprintf(stderr, "Running %s\n", OUStringToOString(sFile, RTL_TEXTENCODING_UTF8).getStr());

        loadURL(sFile, nullptr);

        SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
        CPPUNIT_ASSERT(pTextDoc);

        SwWrtShell* pWrtShell = pTextDoc->GetDocShell()->GetDoc()->GetDocShell()->GetWrtShell();

        dispatchCommand(mxComponent, ".uno:SelectAll", {});
        dispatchCommand(mxComponent, ".uno:SelectAll", {});
        dispatchCommand(mxComponent, ".uno:SelectAll", {});
        Scheduler::ProcessEventsToIdle();

        rtl::Reference<SwTransferable> xTransfer = new SwTransferable(*pWrtShell);
        xTransfer->Cut();

        TransferableDataHelper aHelper(xTransfer);
        SwTransferable::Paste(*pWrtShell, aHelper);

        dispatchCommand(mxComponent, ".uno:Undo", {});
        Scheduler::ProcessEventsToIdle();
    }
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
