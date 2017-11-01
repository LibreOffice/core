/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <svx/ClassificationDialog.hxx>
#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/section.hxx>
#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/itemset.hxx>

#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/uri.hxx>
#include <config_folders.h>
#include <tools/XmlWriter.hxx>
#include <tools/XmlWalker.hxx>

namespace svx {

namespace {

const SvxFieldItem* findField(editeng::Section const & rSection)
{
    for (SfxPoolItem const * pPool : rSection.maAttributes)
    {
        if (pPool->Which() == EE_FEATURE_FIELD)
            return static_cast<const SvxFieldItem*>(pPool);
    }
    return nullptr;
}

bool fileExists(OUString const & sFilename)
{
    osl::File aFile(sFilename);
    osl::FileBase::RC eRC = aFile.open(osl_File_OpenFlag_Read);
    return osl::FileBase::E_None == eRC;
}

bool stringToclassificationType(OString const & rsType, svx::ClassificationType & reType)
{
    if (rsType == "CATEGORY")
        reType = svx::ClassificationType::CATEGORY;
    else if (rsType == "INTELLECTUAL_PROPERTY_PART")
        reType = svx::ClassificationType::INTELLECTUAL_PROPERTY_PART;
    else if (rsType == "MARKING")
        reType = svx::ClassificationType::MARKING;
    else if (rsType == "PARAGRAPH")
        reType = svx::ClassificationType::PARAGRAPH;
    else if (rsType == "TEXT")
        reType = svx::ClassificationType::TEXT;
    else
        return false;
    return true;
}

} // end anonymous namespace

ClassificationDialog::ClassificationDialog(vcl::Window* pParent, const bool bPerParagraph, const std::function<void()>& rParagraphSignHandler)
    : ModalDialog(pParent, "AdvancedDocumentClassificationDialog", "svx/ui/classificationdialog.ui")
    , maHelper(SfxObjectShell::Current()->getDocProperties())
    , maInternationalHelper(SfxObjectShell::Current()->getDocProperties(), /*bUseLocalizedPolicy*/ false)
    , m_bPerParagraph(bPerParagraph)
    , m_aParagraphSignHandler(rParagraphSignHandler)
{
    get(m_pEditWindow, "classificationEditWindow");
    get(m_pSignButton, "signButton");
    get(m_pBoldButton, "boldButton");
    get(m_pClassificationListBox, "classificationCB");
    get(m_pInternationalClassificationListBox, "internationalClassificationCB");
    get(m_pMarkingListBox, "markingCB");
    get(m_pIntellectualPropertyPartNumberListBox, "intellectualPropertyPartNumberCB");
    get(m_pIntellectualPropertyPartListBox, "intellectualPropertyPartLB");
    get(m_pIntellectualPropertyPartAddButton, "intellectualPropertyPartAddButton");
    get(m_pIntellectualPropertyPartEdit, "intellectualPropertyPartEntry");

    m_pSignButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));
    m_pSignButton->Show(m_bPerParagraph);

    m_pBoldButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));
    m_pIntellectualPropertyPartAddButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));

    m_pClassificationListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maHelper.GetBACNames())
        m_pClassificationListBox->InsertEntry(rName);
    m_pClassificationListBox->EnableAutoSize(true);

    m_pInternationalClassificationListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maInternationalHelper.GetBACNames())
        m_pInternationalClassificationListBox->InsertEntry(rName);
    m_pInternationalClassificationListBox->EnableAutoSize(true);

    m_pMarkingListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectMarkingHdl));
    for (const OUString& rName : maHelper.GetMarkings())
        m_pMarkingListBox->InsertEntry(rName);
    m_pMarkingListBox->EnableAutoSize(true);

    m_pIntellectualPropertyPartNumberListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectIPPartNumbersHdl));
    for (const OUString& rName : maHelper.GetIntellectualPropertyPartNumbers())
        m_pIntellectualPropertyPartNumberListBox->InsertEntry(rName);
    m_pIntellectualPropertyPartNumberListBox->EnableAutoSize(true);

    m_pIntellectualPropertyPartListBox->SetDoubleClickHdl(LINK(this, ClassificationDialog, DoubleClickIPPartHdl));
    for (const OUString& rName : maHelper.GetIntellectualPropertyParts())
        m_pIntellectualPropertyPartListBox->InsertEntry(rName);
    m_pIntellectualPropertyPartListBox->EnableAutoSize(true);
}

ClassificationDialog::~ClassificationDialog()
{
    disposeOnce();
}

void ClassificationDialog::dispose()
{
    m_pEditWindow.clear();
    m_pSignButton.clear();
    m_pBoldButton.clear();
    m_pClassificationListBox.clear();
    m_pInternationalClassificationListBox.clear();
    m_pMarkingListBox.clear();
    m_pIntellectualPropertyPartListBox.clear();
    m_pIntellectualPropertyPartNumberListBox.clear();
    m_pIntellectualPropertyPartAddButton.clear();
    m_pIntellectualPropertyPartEdit.clear();

    ModalDialog::dispose();
}

short ClassificationDialog::Execute()
{
    readRecentlyUsed();
    readIn(m_aInitialValues);

    short nResult = ModalDialog::Execute();
    if (nResult == RET_OK)
    {
        writeRecentlyUsed();
    }
    return nResult;
}

void ClassificationDialog::insertField(ClassificationType eType, OUString const & rString, OUString const & rFullString)
{
    ClassificationField aField(eType, rString, rFullString);
    m_pEditWindow->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
}

void ClassificationDialog::setupValues(std::vector<ClassificationResult> const & rInput)
{
    m_aInitialValues = rInput;
}

void ClassificationDialog::readRecentlyUsed()
{
    OUString sPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/user/classification/");
    rtl::Bootstrap::expandMacros(sPath);

    OUString sFilePath(sPath + "recentlyUsed.xml");

    if (!fileExists(sFilePath))
        return;

    SvFileStream aFileStream(sFilePath, StreamMode::READ);
    tools::XmlWalker aWalker;
    if (!aWalker.open(&aFileStream))
        return;

    if (aWalker.name() == "recentlyUsedClassifications")
    {
        aWalker.children();
        while (aWalker.isValid())
        {
            if (aWalker.name() == "elementGroup")
            {
                std::vector<ClassificationResult> aResults;

                aWalker.children();

                while (aWalker.isValid())
                {
                    if (aWalker.name() == "element")
                    {
                        svx::ClassificationType eType = svx::ClassificationType::TEXT;
                        OUString sString;
                        OUString sAbbreviatedString;

                        // Convert string to classification type, but continue only if
                        // conversion was successful.
                        if (stringToclassificationType(aWalker.attribute("type"), eType))
                        {
                            aWalker.children();

                            while (aWalker.isValid())
                            {
                                if (aWalker.name() == "string")
                                {
                                    sString = OStringToOUString(aWalker.content(), RTL_TEXTENCODING_UTF8);
                                }
                                else if (aWalker.name() == "abbreviatedString")
                                {
                                    sAbbreviatedString = OStringToOUString(aWalker.content(), RTL_TEXTENCODING_UTF8);
                                }
                                aWalker.next();
                            }
                            aWalker.parent();

                            aResults.push_back({ eType, sString, sAbbreviatedString });
                        }
                    }
                    aWalker.next();
                }
                aWalker.parent();
                m_aRecentlyUsedValuesCollection.push_back(aResults);
            }
            aWalker.next();
        }
        aWalker.parent();
    }
}

void ClassificationDialog::writeRecentlyUsed()
{
    OUString sPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/user/classification/");
    rtl::Bootstrap::expandMacros(sPath);
    osl::Directory::createPath(sPath);

    OUString sFilePath(sPath + "recentlyUsed.xml");

    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(sFilePath, StreamMode::STD_READWRITE | StreamMode::TRUNC));

    tools::XmlWriter aXmlWriter(pStream.get());

    if (!aXmlWriter.startDocument())
        return;

    aXmlWriter.startElement("recentlyUsedClassifications");

    aXmlWriter.startElement("elementGroup");

    for (ClassificationResult const & rResult : getResult())
    {
        aXmlWriter.startElement("element");
        OUString sType;
        switch(rResult.meType)
        {
            case svx::ClassificationType::CATEGORY:
                sType = "CATEGORY"; break;
            case svx::ClassificationType::MARKING:
                sType = "MARKING"; break;
            case svx::ClassificationType::TEXT:
                sType = "TEXT"; break;
            case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
                sType = "INTELLECTUAL_PROPERTY_PART"; break;
            case svx::ClassificationType::PARAGRAPH:
                sType = "PARAGRAPH"; break;
        }
        aXmlWriter.attribute("type", sType);
        aXmlWriter.startElement("string");
        aXmlWriter.content(rResult.msString);
        aXmlWriter.endElement();
        aXmlWriter.startElement("abbreviatedString");
        aXmlWriter.content(rResult.msAbbreviatedString);
        aXmlWriter.endElement();
        aXmlWriter.endElement();
    }
    aXmlWriter.endElement();

    if (m_aRecentlyUsedValuesCollection.size() >= 5)
        m_aRecentlyUsedValuesCollection.pop_back();

    for (std::vector<ClassificationResult> const & rResultCollection : m_aRecentlyUsedValuesCollection)
    {
        aXmlWriter.startElement("elementGroup");

        for (ClassificationResult const & rResult : rResultCollection)
        {
            aXmlWriter.startElement("element");
            OUString sType;
            switch(rResult.meType)
            {
                case svx::ClassificationType::CATEGORY:
                    sType = "CATEGORY"; break;
                case svx::ClassificationType::MARKING:
                    sType = "MARKING"; break;
                case svx::ClassificationType::TEXT:
                    sType = "TEXT"; break;
                case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
                    sType = "INTELLECTUAL_PROPERTY_PART"; break;
                case svx::ClassificationType::PARAGRAPH:
                    sType = "PARAGRAPH"; break;
            }
            aXmlWriter.attribute("type", sType);
            aXmlWriter.startElement("string");
            aXmlWriter.content(rResult.msString);
            aXmlWriter.endElement();
            aXmlWriter.startElement("abbreviatedString");
            aXmlWriter.content(rResult.msAbbreviatedString);
            aXmlWriter.endElement();
            aXmlWriter.endElement();
        }
        aXmlWriter.endElement();
    }

    aXmlWriter.endElement();

    aXmlWriter.endDocument();
}

void ClassificationDialog::readIn(std::vector<ClassificationResult> const & rInput)
{
    sal_Int32 nParagraph = -1;
    for (ClassificationResult const & rClassificationResult : rInput)
    {
        OUString msAbbreviatedString = rClassificationResult.msAbbreviatedString;
        if (msAbbreviatedString.isEmpty())
            msAbbreviatedString = maHelper.GetAbbreviatedBACName(rClassificationResult.msString);

        switch (rClassificationResult.meType)
        {
            case svx::ClassificationType::TEXT:
            {
                m_pEditWindow->pEdView->InsertText(rClassificationResult.msString);
            }
            break;

            case svx::ClassificationType::CATEGORY:
            {
                m_pClassificationListBox->SelectEntry(rClassificationResult.msString);
                m_pInternationalClassificationListBox->SelectEntryPos(m_pClassificationListBox->GetSelectedEntryPos());
                insertField(rClassificationResult.meType, msAbbreviatedString, rClassificationResult.msString);
            }
            break;

            case svx::ClassificationType::MARKING:
            {
                m_pMarkingListBox->SelectEntry(rClassificationResult.msString);
                insertField(rClassificationResult.meType, msAbbreviatedString, rClassificationResult.msString);
            }
            break;

            case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
            {
                insertField(rClassificationResult.meType, msAbbreviatedString, rClassificationResult.msString);
            }
            break;

            case svx::ClassificationType::PARAGRAPH:
            {
                nParagraph++;

                if (nParagraph != 0)
                    m_pEditWindow->pEdView->InsertParaBreak();

                // Set paragraph font weight
                FontWeight eWeight = (rClassificationResult.msString == "BOLD") ? WEIGHT_BOLD : WEIGHT_NORMAL;
                std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(m_pEditWindow->pEdEngine->GetParaAttribs(nParagraph)));
                pSet->Put(SvxWeightItem(eWeight, EE_CHAR_WEIGHT));
                m_pEditWindow->pEdEngine->SetParaAttribs(nParagraph, *pSet);
            }
            break;

            default:
            break;
        }
    }
}

std::vector<ClassificationResult> ClassificationDialog::getResult()
{
    std::vector<ClassificationResult> aClassificationResults;

    std::unique_ptr<EditTextObject> pEditText(m_pEditWindow->pEdEngine->CreateTextObject());

    sal_Int32 nCurrentParagraph = -1;

    std::vector<editeng::Section> aSections;
    pEditText->GetAllSections(aSections);
    for (editeng::Section const & rSection : aSections)
    {
        while (nCurrentParagraph < rSection.mnParagraph)
        {
            nCurrentParagraph++;

            // Get Weight of current paragraph
            FontWeight eFontWeight = WEIGHT_NORMAL;
            SfxItemSet aItemSet(m_pEditWindow->pEdEngine->GetParaAttribs(nCurrentParagraph));
            if (const SfxPoolItem* pItem = aItemSet.GetItem(EE_CHAR_WEIGHT, false))
            {
                const SvxWeightItem* pWeightItem = dynamic_cast<const SvxWeightItem*>(pItem);
                if (pWeightItem && pWeightItem->GetWeight() == WEIGHT_BOLD)
                    eFontWeight = WEIGHT_BOLD;
            }
            // Font weight to string
            OUString sWeightProperty = "NORMAL";
            if (eFontWeight == WEIGHT_BOLD)
                sWeightProperty = "BOLD";
            // Insert into collection
            OUString sBlank;
            aClassificationResults.push_back({ ClassificationType::PARAGRAPH, sWeightProperty, sBlank });
        }

        const SvxFieldItem* pFieldItem = findField(rSection);

        ESelection aSelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd);
        const OUString sDisplayString = m_pEditWindow->pEdEngine->GetText(aSelection);
        if (!sDisplayString.isEmpty())
        {
            const ClassificationField* pClassificationField = pFieldItem ? dynamic_cast<const ClassificationField*>(pFieldItem->GetField()) : nullptr;

            if (pClassificationField)
            {
                aClassificationResults.push_back({ pClassificationField->meType, pClassificationField->msFullClassName, sDisplayString });
            }
            else
            {
                aClassificationResults.push_back({ ClassificationType::TEXT, sDisplayString, sDisplayString });
            }
        }
    }

    return aClassificationResults;
}

IMPL_LINK(ClassificationDialog, SelectClassificationHdl, ListBox&, rBox, void)
{
    const sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
        std::unique_ptr<EditTextObject> pEditText(m_pEditWindow->pEdEngine->CreateTextObject());
        std::vector<editeng::Section> aSections;
        pEditText->GetAllSections(aSections);

        for (editeng::Section const & rSection : aSections)
        {
            const SvxFieldItem* pFieldItem = findField(rSection);
            if (pFieldItem)
            {
                const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(pFieldItem->GetField());
                if (pClassificationField && pClassificationField->meType == ClassificationType::CATEGORY)
                {
                    m_pEditWindow->pEdView->SetSelection(ESelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd));
                }
            }
        }

        const OUString aFullString = maHelper.GetBACNames()[nSelected];
        const OUString aAbbreviatedString = maHelper.GetAbbreviatedBACNames()[nSelected];
        insertField(ClassificationType::CATEGORY, aAbbreviatedString, aFullString);

        m_pInternationalClassificationListBox->SelectEntryPos(nSelected);
        m_pClassificationListBox->SelectEntryPos(nSelected);
    }
}

IMPL_LINK(ClassificationDialog, SelectMarkingHdl, ListBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
        std::unique_ptr<EditTextObject> pEditText(m_pEditWindow->pEdEngine->CreateTextObject());
        std::vector<editeng::Section> aSections;
        pEditText->GetAllSections(aSections);

        for (editeng::Section const & rSection : aSections)
        {
            const SvxFieldItem* pFieldItem = findField(rSection);
            if (pFieldItem)
            {
                const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(pFieldItem->GetField());
                if (pClassificationField && pClassificationField->meType == ClassificationType::MARKING)
                {
                    m_pEditWindow->pEdView->SetSelection(ESelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd));
                }
            }
        }

        const OUString aString = maHelper.GetMarkings()[nSelected];
        insertField(ClassificationType::MARKING, aString, aString);
    }
}

IMPL_LINK(ClassificationDialog, SelectIPPartNumbersHdl, ListBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
        OUString sString = maHelper.GetIntellectualPropertyPartNumbers()[nSelected];
        m_pIntellectualPropertyPartEdit->SetText(m_pIntellectualPropertyPartEdit->GetText() + sString);
    }
}

IMPL_LINK(ClassificationDialog, DoubleClickIPPartHdl, ListBox&, rBox, void)
{
    const sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
        const OUString sString = maHelper.GetIntellectualPropertyParts()[nSelected];
        m_pIntellectualPropertyPartEdit->SetText(m_pIntellectualPropertyPartEdit->GetText() + sString);
    }
}

IMPL_LINK(ClassificationDialog, ButtonClicked, Button*, pButton, void)
{
    if (pButton == m_pBoldButton)
    {
        m_pEditWindow->InvertSelectionWeight();
    }
    else if (pButton == m_pSignButton)
    {
        m_aParagraphSignHandler();
    }
    else if (pButton == m_pIntellectualPropertyPartAddButton)
    {
        const OUString sString = m_pIntellectualPropertyPartEdit->GetText();
        insertField(ClassificationType::INTELLECTUAL_PROPERTY_PART, sString, sString);
    }
}

} // end sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
