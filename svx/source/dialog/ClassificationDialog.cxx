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
#include <svx/ClassificationCommon.hxx>
#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>

#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/section.hxx>
#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/itemset.hxx>
#include <vcl/msgbox.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <rtl/uri.hxx>
#include <config_folders.h>
#include <tools/XmlWriter.hxx>
#include <tools/XmlWalker.hxx>
#include <vcl/builderfactory.hxx>

#include <officecfg/Office/Common.hxx>

namespace svx {


IntellectualPropertyPartEdit::IntellectualPropertyPartEdit(vcl::Window* pParent)
    : Edit(pParent, WB_LEFT|WB_VCENTER|WB_BORDER|WB_3DLOOK|WB_TABSTOP)
{
}

VCL_BUILDER_FACTORY(IntellectualPropertyPartEdit)

void IntellectualPropertyPartEdit::KeyInput(const KeyEvent& rKeyEvent)
{
    bool bTextIsFreeForm = officecfg::Office::Common::Classification::IntellectualPropertyTextInputIsFreeForm::get();

    if (bTextIsFreeForm)
    {
        Edit::KeyInput(rKeyEvent);
    }
    else
    {
        // Ignore key combination with modifier keys
        if (rKeyEvent.GetKeyCode().IsMod3()
         || rKeyEvent.GetKeyCode().IsMod2()
         || rKeyEvent.GetKeyCode().IsMod1())
        {
            return;
        }

        switch (rKeyEvent.GetKeyCode().GetCode())
        {
            // Allowed characters
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case KEY_DIVIDE:
            case KEY_SEMICOLON:
            case KEY_SPACE:
                Edit::KeyInput(rKeyEvent);
                return;
            // Anything else is ignored
            default:
                break;
        }
    }
}

namespace {

constexpr size_t RECENTLY_USED_LIMIT = 5;

const OUString constRecentlyUsedFileName("recentlyUsed.xml");

OUString lcl_getClassificationUserPath()
{
    OUString sPath("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/user/classification/");
    rtl::Bootstrap::expandMacros(sPath);
    return sPath;
}

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

bool stringToClassificationType(OString const & rsType, svx::ClassificationType & reType)
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

OUString classificationTypeToString(svx::ClassificationType const & reType)
{
    switch(reType)
    {
        case svx::ClassificationType::CATEGORY:
            return OUString("CATEGORY"); break;
        case svx::ClassificationType::MARKING:
            return OUString("MARKING"); break;
        case svx::ClassificationType::TEXT:
            return OUString("TEXT"); break;
        case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
            return OUString("INTELLECTUAL_PROPERTY_PART"); break;
        case svx::ClassificationType::PARAGRAPH:
            return OUString("PARAGRAPH"); break;
    }
    return OUString();
}

void writeResultToXml(tools::XmlWriter & rXmlWriter,
                      std::vector<ClassificationResult> const & rResultCollection)
{
    for (ClassificationResult const & rResult : rResultCollection)
    {
        rXmlWriter.startElement("element");
        OUString sType = classificationTypeToString(rResult.meType);
        rXmlWriter.attribute("type", sType);
        rXmlWriter.startElement("string");
        rXmlWriter.content(rResult.msName);
        rXmlWriter.endElement();
        rXmlWriter.startElement("abbreviatedString");
        rXmlWriter.content(rResult.msAbbreviatedName);
        rXmlWriter.endElement();
        rXmlWriter.startElement("identifier");
        rXmlWriter.content(rResult.msIdentifier);
        rXmlWriter.endElement();
        rXmlWriter.endElement();
    }
}

} // end anonymous namespace

ClassificationDialog::ClassificationDialog(vcl::Window* pParent, const bool bPerParagraph, const std::function<void()>& rParagraphSignHandler)
    : ModalDialog(pParent, "AdvancedDocumentClassificationDialog", "svx/ui/classificationdialog.ui")
    , maHelper(SfxObjectShell::Current()->getDocProperties())
    , maInternationalHelper(SfxObjectShell::Current()->getDocProperties(), /*bUseLocalizedPolicy*/ false)
    , m_bPerParagraph(bPerParagraph)
    , m_aParagraphSignHandler(rParagraphSignHandler)
    , m_nCurrentSelectedCategory(-1)
{
    get(m_pOkButton, "ok");
    get(m_pEditWindow, "classificationEditWindow");
    get(m_pSignButton, "signButton");
    get(m_pToolBox, "toolbox");
    get(m_pRecentlyUsedListBox, "recentlyUsedCB");
    get(m_pClassificationListBox, "classificationCB");
    get(m_pInternationalClassificationListBox, "internationalClassificationCB");
    get(m_pMarkingLabel, "markingLabel");
    get(m_pMarkingListBox, "markingCB");
    get(m_pIntellectualPropertyPartNumberListBox, "intellectualPropertyPartNumberCB");
    get(m_pIntellectualPropertyPartListBox, "intellectualPropertyPartLB");
    get(m_pIntellectualPropertyPartAddButton, "intellectualPropertyPartAddButton");
    get(m_pIntellectualPropertyPartEdit, "intellectualPropertyPartEntry");

    get(m_pIntellectualPropertyExpander, "intellectualPropertyExpander");

    m_pSignButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));
    m_pSignButton->Show(m_bPerParagraph);

    m_pToolBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectToolboxHdl));

    m_pIntellectualPropertyPartAddButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));

    m_pClassificationListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maHelper.GetBACNames())
        m_pClassificationListBox->InsertEntry(rName);
    m_pClassificationListBox->EnableAutoSize(true);

    m_pInternationalClassificationListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maInternationalHelper.GetBACNames())
        m_pInternationalClassificationListBox->InsertEntry(rName);
    m_pInternationalClassificationListBox->EnableAutoSize(true);

    if (!maHelper.GetMarkings().empty())
    {
        m_pMarkingListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectMarkingHdl));
        for (const OUString& rName : maHelper.GetMarkings())
            m_pMarkingListBox->InsertEntry(rName);
        m_pMarkingListBox->EnableAutoSize(true);
    }
    else
    {
        m_pMarkingListBox->Show(false);
        m_pMarkingLabel->Show(false);
    }

    m_pIntellectualPropertyPartNumberListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectIPPartNumbersHdl));
    for (const OUString& rName : maHelper.GetIntellectualPropertyPartNumbers())
        m_pIntellectualPropertyPartNumberListBox->InsertEntry(rName);
    m_pIntellectualPropertyPartNumberListBox->EnableAutoSize(true);

    m_pIntellectualPropertyPartListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectIPPartHdl));
    for (const OUString& rName : maHelper.GetIntellectualPropertyParts())
        m_pIntellectualPropertyPartListBox->InsertEntry(rName);
    m_pIntellectualPropertyPartListBox->EnableAutoSize(true);

    m_pRecentlyUsedListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectRecentlyUsedHdl));

    bool bExpand = officecfg::Office::Common::Classification::IntellectualPropertySectionExpanded::get();
    m_pIntellectualPropertyExpander->set_expanded(bExpand);
    m_pIntellectualPropertyExpander->SetExpandedHdl(LINK(this, ClassificationDialog, ExpandedHdl));

    m_pEditWindow->SetModifyHdl(LINK(this, ClassificationDialog, EditWindowModifiedHdl));
}

ClassificationDialog::~ClassificationDialog()
{
    disposeOnce();
}

void ClassificationDialog::dispose()
{
    m_pOkButton.clear();
    m_pEditWindow.clear();
    m_pSignButton.clear();
    m_pToolBox.clear();
    m_pRecentlyUsedListBox.clear();
    m_pClassificationListBox.clear();
    m_pInternationalClassificationListBox.clear();
    m_pMarkingLabel.clear();
    m_pMarkingListBox.clear();
    m_pIntellectualPropertyPartListBox.clear();
    m_pIntellectualPropertyPartNumberListBox.clear();
    m_pIntellectualPropertyPartAddButton.clear();
    m_pIntellectualPropertyPartEdit.clear();
    m_pIntellectualPropertyExpander.clear();

    ModalDialog::dispose();
}

short ClassificationDialog::Execute()
{
    readRecentlyUsed();
    readIn(m_aInitialValues);

    int nNumber = 1;
    if (m_aRecentlyUsedValuesCollection.empty())
    {
        m_pRecentlyUsedListBox->Disable();
    }
    else
    {
        for (std::vector<ClassificationResult> const & rResults : m_aRecentlyUsedValuesCollection)
        {
            OUString rContentRepresentation = svx::classification::convertClassificationResultToString(rResults);
            OUString rDescription = OUString::number(nNumber) + ": " + rContentRepresentation;
            nNumber++;

            m_pRecentlyUsedListBox->InsertEntry(rDescription);
        }
    }

    short nResult = ModalDialog::Execute();
    if (nResult == RET_OK)
    {
        writeRecentlyUsed();
    }
    return nResult;
}

void ClassificationDialog::insertField(ClassificationType eType, OUString const & rString, OUString const & rFullString, OUString const & rIdentifier)
{
    ClassificationField aField(eType, rString, rFullString, rIdentifier);
    m_pEditWindow->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
}

void ClassificationDialog::setupValues(std::vector<ClassificationResult> const & rInput)
{
    m_aInitialValues = rInput;
}

void ClassificationDialog::readRecentlyUsed()
{
    OUString sPath = lcl_getClassificationUserPath();
    OUString sFilePath(sPath + constRecentlyUsedFileName);

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
                        OUString sIdentifier;

                        // Convert string to classification type, but continue only if
                        // conversion was successful.
                        if (stringToClassificationType(aWalker.attribute("type"), eType))
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
                                else if (aWalker.name() == "identifier")
                                {
                                    sIdentifier = OStringToOUString(aWalker.content(), RTL_TEXTENCODING_UTF8);
                                }
                                aWalker.next();
                            }
                            aWalker.parent();

                            aResults.push_back({ eType, sString, sAbbreviatedString, sIdentifier });
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
    OUString sPath = lcl_getClassificationUserPath();
    osl::Directory::createPath(sPath);
    OUString sFilePath(sPath + constRecentlyUsedFileName);

    std::unique_ptr<SvStream> pStream;
    pStream.reset(new SvFileStream(sFilePath, StreamMode::STD_READWRITE | StreamMode::TRUNC));

    tools::XmlWriter aXmlWriter(pStream.get());

    if (!aXmlWriter.startDocument())
        return;

    aXmlWriter.startElement("recentlyUsedClassifications");

    aXmlWriter.startElement("elementGroup");

    writeResultToXml(aXmlWriter, getResult());

    aXmlWriter.endElement();

    if (m_aRecentlyUsedValuesCollection.size() >= RECENTLY_USED_LIMIT)
        m_aRecentlyUsedValuesCollection.pop_back();

    for (std::vector<ClassificationResult> const & rResultCollection : m_aRecentlyUsedValuesCollection)
    {
        aXmlWriter.startElement("elementGroup");

        writeResultToXml(aXmlWriter, rResultCollection);

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

        switch (rClassificationResult.meType)
        {
            case svx::ClassificationType::TEXT:
            {
                m_pEditWindow->pEdView->InsertText(rClassificationResult.msName);
            }
            break;

            case svx::ClassificationType::CATEGORY:
            {
                OUString sName;
                if (rClassificationResult.msName.isEmpty())
                    sName = maHelper.GetBACNameForIdentifier(rClassificationResult.msIdentifier);
                else
                    sName = rClassificationResult.msName;

                OUString sAbbreviatedName = rClassificationResult.msAbbreviatedName;
                if (sAbbreviatedName.isEmpty())
                    sAbbreviatedName = maHelper.GetAbbreviatedBACName(sName);

                m_pClassificationListBox->SelectEntry(sName);
                m_nCurrentSelectedCategory = m_pClassificationListBox->GetSelectedEntryPos();
                m_pInternationalClassificationListBox->SelectEntryPos(m_pClassificationListBox->GetSelectedEntryPos());

                insertField(rClassificationResult.meType, sAbbreviatedName, sName, rClassificationResult.msIdentifier);
            }
            break;

            case svx::ClassificationType::MARKING:
            {
                m_pMarkingListBox->SelectEntry(rClassificationResult.msName);
                insertField(rClassificationResult.meType, rClassificationResult.msName, rClassificationResult.msName, rClassificationResult.msIdentifier);
            }
            break;

            case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
            {
                insertField(rClassificationResult.meType, rClassificationResult.msName, rClassificationResult.msName, rClassificationResult.msIdentifier);
            }
            break;

            case svx::ClassificationType::PARAGRAPH:
            {
                nParagraph++;

                if (nParagraph != 0)
                    m_pEditWindow->pEdView->InsertParaBreak();

                // Set paragraph font weight
                FontWeight eWeight = (rClassificationResult.msName == "BOLD") ? WEIGHT_BOLD : WEIGHT_NORMAL;
                std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(m_pEditWindow->pEdEngine->GetParaAttribs(nParagraph)));
                pSet->Put(SvxWeightItem(eWeight, EE_CHAR_WEIGHT));
                m_pEditWindow->pEdEngine->SetParaAttribs(nParagraph, *pSet);
            }
            break;

            default:
            break;
        }
    }
    toggleWidgetsDependingOnCategory();
}

void ClassificationDialog::toggleWidgetsDependingOnCategory()
{
    const EditEngine& rEditEngine = m_pEditWindow->getEditEngine();

    for (sal_Int32 nParagraph = 0; nParagraph < rEditEngine.GetParagraphCount(); ++nParagraph)
    {
        sal_uInt16 nFieldCount = rEditEngine.GetFieldCount(nParagraph);
        for (sal_Int16 nField = 0; nField < nFieldCount; ++nField)
        {
            EFieldInfo aFieldInfo = rEditEngine.GetFieldInfo(nParagraph, nField);
            if (aFieldInfo.pFieldItem)
            {
                const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(aFieldInfo.pFieldItem->GetField());
                if (pClassificationField && pClassificationField->meType == ClassificationType::CATEGORY)
                {
                    m_pOkButton->Enable();
                    return;
                }
            }
        }
    }

    // Category field in the text edit has been deleted, so reset the list boxes
    m_pOkButton->Disable();
    m_pClassificationListBox->SetNoSelection();
    m_pInternationalClassificationListBox->SetNoSelection();
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
            aClassificationResults.push_back({ ClassificationType::PARAGRAPH, sWeightProperty, sBlank, sBlank });
        }

        const SvxFieldItem* pFieldItem = findField(rSection);

        ESelection aSelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd);
        const OUString sDisplayString = m_pEditWindow->pEdEngine->GetText(aSelection);
        if (!sDisplayString.isEmpty())
        {
            const ClassificationField* pClassificationField = pFieldItem ? dynamic_cast<const ClassificationField*>(pFieldItem->GetField()) : nullptr;

            if (pClassificationField)
            {
                aClassificationResults.push_back({ pClassificationField->meType, pClassificationField->msFullClassName,
                                                   sDisplayString, pClassificationField->msIdentifier });
            }
            else
            {
                aClassificationResults.push_back({ ClassificationType::TEXT, sDisplayString, sDisplayString, OUString() });
            }
        }
    }

    return aClassificationResults;
}

IMPL_LINK(ClassificationDialog, SelectClassificationHdl, ListBox&, rBox, void)
{
    const sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected < 0 || m_nCurrentSelectedCategory == nSelected)
        return;

    std::unique_ptr<EditTextObject> pEditText(m_pEditWindow->pEdEngine->CreateTextObject());
    std::vector<editeng::Section> aSections;
    pEditText->GetAllSections(aSections);

    // if we are replacing an existing field
    bool bReplaceExisting = false;
    // selection of the existing field, which will be replaced
    ESelection aExistingFieldSelection;

    for (editeng::Section const & rSection : aSections)
    {
        const SvxFieldItem* pFieldItem = findField(rSection);
        if (pFieldItem)
        {
            const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(pFieldItem->GetField());
            if (pClassificationField && pClassificationField->meType == ClassificationType::CATEGORY)
            {
                aExistingFieldSelection = ESelection(rSection.mnParagraph, rSection.mnStart,
                                                     rSection.mnParagraph, rSection.mnEnd);
                bReplaceExisting = true;
            }
        }
    }

    if (bReplaceExisting)
    {
        ScopedVclPtrInstance<QueryBox> aQueryBox(this, MessBoxStyle::YesNo | MessBoxStyle::DefaultYes, SvxResId(RID_CLASSIFICATION_CHANGE_CATEGORY));
        if (aQueryBox->Execute() == RET_NO)
        {
            // Revert to previosuly selected
            m_pInternationalClassificationListBox->SelectEntryPos(m_nCurrentSelectedCategory);
            m_pClassificationListBox->SelectEntryPos(m_nCurrentSelectedCategory);
            return;
        }
        m_pEditWindow->pEdView->SetSelection(aExistingFieldSelection);
    }

    const OUString aFullString = maHelper.GetBACNames()[nSelected];
    const OUString aAbbreviatedString = maHelper.GetAbbreviatedBACNames()[nSelected];
    const OUString aIdentifierString = maHelper.GetBACIdentifiers()[nSelected];
    insertField(ClassificationType::CATEGORY, aAbbreviatedString, aFullString, aIdentifierString);

    // Change category to the new selection
    m_pInternationalClassificationListBox->SelectEntryPos(nSelected);
    m_pClassificationListBox->SelectEntryPos(nSelected);
    m_nCurrentSelectedCategory = nSelected;
}

IMPL_LINK(ClassificationDialog, SelectMarkingHdl, ListBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
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
        m_pIntellectualPropertyPartEdit->ReplaceSelected(sString);
        m_pIntellectualPropertyPartEdit->GrabFocus();
    }
}

IMPL_LINK(ClassificationDialog, SelectRecentlyUsedHdl, ListBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
        m_pEditWindow->pEdEngine->Clear();
        readIn(m_aRecentlyUsedValuesCollection[nSelected]);
    }
}

IMPL_LINK(ClassificationDialog, SelectIPPartHdl, ListBox&, rBox, void)
{
    const sal_Int32 nSelected = rBox.GetSelectedEntryPos();
    if (nSelected >= 0)
    {
        const OUString sString = maHelper.GetIntellectualPropertyParts()[nSelected];
        m_pIntellectualPropertyPartEdit->ReplaceSelected(sString);
        m_pIntellectualPropertyPartEdit->GrabFocus();
    }
}

IMPL_LINK(ClassificationDialog, ButtonClicked, Button*, pButton, void)
{
    if (pButton == m_pSignButton)
    {
        m_aParagraphSignHandler();
    }
    else if (pButton == m_pIntellectualPropertyPartAddButton)
    {
        const OUString sString = m_pIntellectualPropertyPartEdit->GetText();
        insertField(ClassificationType::INTELLECTUAL_PROPERTY_PART, sString, sString);
    }
}


IMPL_LINK_NOARG(ClassificationDialog, SelectToolboxHdl, ToolBox*, void)
{
    sal_uInt16 nId = m_pToolBox->GetCurItemId();
    const OUString sCommand = m_pToolBox->GetItemCommand(nId);
    if (sCommand == "bold")
    {
        m_pEditWindow->InvertSelectionWeight();
    }
}


IMPL_LINK_NOARG(ClassificationDialog, EditWindowModifiedHdl, LinkParamNone*, void)
{
    toggleWidgetsDependingOnCategory();
}

IMPL_LINK(ClassificationDialog, ExpandedHdl, VclExpander&, rExpander, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> aConfigurationChanges(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Classification::IntellectualPropertySectionExpanded::set(rExpander.get_expanded(), aConfigurationChanges);
    aConfigurationChanges->commit();
}

} // end svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
