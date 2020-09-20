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

#include <editeng/flditem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/section.hxx>
#include <editeng/editobj.hxx>
#include <editeng/wghtitem.hxx>
#include <svl/itemset.hxx>
#include <osl/file.hxx>
#include <rtl/bootstrap.hxx>
#include <config_folders.h>
#include <tools/stream.hxx>
#include <tools/XmlWriter.hxx>
#include <tools/XmlWalker.hxx>
#include <vcl/customweld.hxx>
#include <vcl/event.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/objsh.hxx>

#include <officecfg/Office/Common.hxx>

#include "ClassificationEditView.hxx"

namespace svx {

IMPL_STATIC_LINK(ClassificationDialog, KeyInput, const KeyEvent&, rKeyEvent, bool)
{
    bool bTextIsFreeForm = officecfg::Office::Common::Classification::IntellectualPropertyTextInputIsFreeForm::get();

    if (!bTextIsFreeForm)
    {
        // Ignore key combination with modifier keys
        if (rKeyEvent.GetKeyCode().IsMod3()
         || rKeyEvent.GetKeyCode().IsMod2()
         || rKeyEvent.GetKeyCode().IsMod1())
        {
            return true;
        }

        switch (rKeyEvent.GetKeyCode().GetCode())
        {
            // Allowed characters
            case KEY_BACKSPACE:
            case KEY_DELETE:
            case KEY_DIVIDE:
            case KEY_SEMICOLON:
            case KEY_SPACE:
                return false;
            // Anything else is ignored
            default:
                return true;
                break;
        }
    }

    return false;
}

namespace {

constexpr size_t RECENTLY_USED_LIMIT = 5;

constexpr OUStringLiteral constRecentlyUsedFileName(u"recentlyUsed.xml");

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
            return "CATEGORY"; break;
        case svx::ClassificationType::MARKING:
            return "MARKING"; break;
        case svx::ClassificationType::TEXT:
            return "TEXT"; break;
        case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
            return "INTELLECTUAL_PROPERTY_PART"; break;
        case svx::ClassificationType::PARAGRAPH:
            return "PARAGRAPH"; break;
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

ClassificationDialog::ClassificationDialog(weld::Window* pParent, const bool bPerParagraph, const std::function<void()>& rParagraphSignHandler)
    : GenericDialogController(pParent, "svx/ui/classificationdialog.ui", "AdvancedDocumentClassificationDialog")
    , maHelper(SfxObjectShell::Current()->getDocProperties())
    , maInternationalHelper(SfxObjectShell::Current()->getDocProperties(), /*bUseLocalizedPolicy*/ false)
    , m_bPerParagraph(bPerParagraph)
    , m_aParagraphSignHandler(rParagraphSignHandler)
    , m_nCurrentSelectedCategory(-1)
    , m_xOkButton(m_xBuilder->weld_button("ok"))
    , m_xSignButton(m_xBuilder->weld_button("signButton"))
    , m_xToolBox(m_xBuilder->weld_toggle_button("toolbox"))
    , m_xRecentlyUsedListBox(m_xBuilder->weld_combo_box("recentlyUsedCB"))
    , m_xClassificationListBox(m_xBuilder->weld_combo_box("classificationCB"))
    , m_xInternationalClassificationListBox(m_xBuilder->weld_combo_box("internationalClassificationCB"))
    , m_xMarkingLabel(m_xBuilder->weld_label("markingLabel"))
    , m_xMarkingListBox(m_xBuilder->weld_tree_view("markingLB"))
    , m_xIntellectualPropertyPartListBox(m_xBuilder->weld_tree_view("intellectualPropertyPartLB"))
    , m_xIntellectualPropertyPartNumberListBox(m_xBuilder->weld_tree_view("intellectualPropertyPartNumberLB"))
    , m_xIntellectualPropertyPartAddButton(m_xBuilder->weld_button("intellectualPropertyPartAddButton"))
    , m_xIntellectualPropertyPartEdit(m_xBuilder->weld_entry("intellectualPropertyPartEntry"))
    , m_xIntellectualPropertyExpander(m_xBuilder->weld_expander("intellectualPropertyExpander"))
    , m_xEditWindow(new ClassificationEditView)
    , m_xEditWindowWeld(new weld::CustomWeld(*m_xBuilder, "classificationEditWindow", *m_xEditWindow))
{
    m_xOkButton->connect_clicked(LINK(this, ClassificationDialog, OkHdl));
    m_xSignButton->connect_clicked(LINK(this, ClassificationDialog, ButtonClicked));
    m_xSignButton->set_visible(m_bPerParagraph);

    m_xIntellectualPropertyPartEdit->connect_key_press(LINK(this, ClassificationDialog, KeyInput));

    // no need for BOLD if we do paragraph classification
    if (m_bPerParagraph)
    {
        m_xToolBox->hide();
    }
    else
    {
        m_xToolBox->connect_toggled(LINK(this, ClassificationDialog, SelectToolboxHdl));
    }

    m_xIntellectualPropertyPartAddButton->connect_clicked(LINK(this, ClassificationDialog, ButtonClicked));

    m_xClassificationListBox->set_size_request(m_xClassificationListBox->get_approximate_digit_width() * 20, -1);
    m_xClassificationListBox->connect_changed(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maHelper.GetBACNames())
        m_xClassificationListBox->append_text(rName);

    m_xInternationalClassificationListBox->set_size_request(m_xInternationalClassificationListBox->get_approximate_digit_width() * 20, -1);
    m_xInternationalClassificationListBox->connect_changed(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maInternationalHelper.GetBACNames())
        m_xInternationalClassificationListBox->append_text(rName);

    if (!maHelper.GetMarkings().empty())
    {
        m_xMarkingListBox->set_size_request(m_xMarkingListBox->get_approximate_digit_width() * 10,
                                            m_xMarkingListBox->get_height_rows(4));
        m_xMarkingListBox->connect_row_activated(LINK(this, ClassificationDialog, SelectMarkingHdl));

        for (const OUString& rName : maHelper.GetMarkings())
            m_xMarkingListBox->append_text(rName);
    }
    else
    {
        m_xMarkingListBox->hide();
        m_xMarkingLabel->hide();
    }

    m_xIntellectualPropertyPartNumberListBox->set_size_request(m_xIntellectualPropertyPartNumberListBox->get_approximate_digit_width() * 10,
                                                               m_xIntellectualPropertyPartNumberListBox->get_height_rows(5));
    m_xIntellectualPropertyPartNumberListBox->connect_row_activated(LINK(this, ClassificationDialog, SelectIPPartNumbersHdl));
    for (const OUString& rName : maHelper.GetIntellectualPropertyPartNumbers())
        m_xIntellectualPropertyPartNumberListBox->append_text(rName);

    m_xIntellectualPropertyPartNumberListBox->set_size_request(m_xIntellectualPropertyPartNumberListBox->get_approximate_digit_width() * 20,
                                                               m_xIntellectualPropertyPartListBox->get_height_rows(5));
    m_xIntellectualPropertyPartListBox->connect_row_activated(LINK(this, ClassificationDialog, SelectIPPartHdl));
    for (const OUString& rName : maHelper.GetIntellectualPropertyParts())
        m_xIntellectualPropertyPartListBox->append_text(rName);

    m_xRecentlyUsedListBox->set_size_request(m_xRecentlyUsedListBox->get_approximate_digit_width() * 5, -1);
    m_xRecentlyUsedListBox->connect_changed(LINK(this, ClassificationDialog, SelectRecentlyUsedHdl));

    m_xIntellectualPropertyExpander->connect_expanded(LINK(this, ClassificationDialog, ExpandedHdl));
    if (officecfg::Office::Common::Classification::IntellectualPropertySectionExpanded::get())
        m_nAsyncExpandEvent = Application::PostUserEvent(LINK(this, ClassificationDialog, OnAsyncExpandHdl));
    else
        m_nAsyncExpandEvent = nullptr;

    m_xEditWindow->SetModifyHdl(LINK(this, ClassificationDialog, EditWindowModifiedHdl));

    readRecentlyUsed();
    toggleWidgetsDependingOnCategory();

    int nNumber = 1;
    if (m_aRecentlyUsedValuesCollection.empty())
    {
        m_xRecentlyUsedListBox->set_sensitive(false);
    }
    else
    {
        for (std::vector<ClassificationResult> const & rResults : m_aRecentlyUsedValuesCollection)
        {
            OUString rContentRepresentation = svx::classification::convertClassificationResultToString(rResults);
            OUString rDescription = OUString::number(nNumber) + ": " + rContentRepresentation;
            nNumber++;

            m_xRecentlyUsedListBox->append_text(rDescription);
        }
    }
}

//do it async so gtk has a chance to shrink it to best size, otherwise its larger than min
IMPL_LINK_NOARG(ClassificationDialog, OnAsyncExpandHdl, void*, void)
{
    m_nAsyncExpandEvent = nullptr;
    m_xIntellectualPropertyExpander->set_expanded(true);
}

ClassificationDialog::~ClassificationDialog()
{
    if (m_nAsyncExpandEvent)
        Application::RemoveUserEvent(m_nAsyncExpandEvent);
}

void ClassificationDialog::insertCategoryField(sal_Int32 nID)
{
    const OUString aFullString = maHelper.GetBACNames()[nID];
    const OUString aAbbreviatedString = maHelper.GetAbbreviatedBACNames()[nID];
    const OUString aIdentifierString = maHelper.GetBACIdentifiers()[nID];
    insertField(ClassificationType::CATEGORY, aAbbreviatedString, aFullString, aIdentifierString);
}

void ClassificationDialog::insertField(ClassificationType eType, OUString const & rString, OUString const & rFullString, OUString const & rIdentifier)
{
    ClassificationField aField(eType, rString, rFullString, rIdentifier);
    m_xEditWindow->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
}

void ClassificationDialog::setupValues(std::vector<ClassificationResult> const & rInput)
{
    m_aInitialValues = rInput;
    readIn(m_aInitialValues);
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

    if (aWalker.name() != "recentlyUsedClassifications")
        return;

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
                m_xEditWindow->getEditView().InsertText(rClassificationResult.msName);
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

                m_xClassificationListBox->set_active_text(sName);
                m_nCurrentSelectedCategory = m_xClassificationListBox->get_active();
                m_xInternationalClassificationListBox->set_active(m_xClassificationListBox->get_active());

                insertField(rClassificationResult.meType, sAbbreviatedName, sName, rClassificationResult.msIdentifier);
            }
            break;

            case svx::ClassificationType::MARKING:
            {
                m_xMarkingListBox->select_text(rClassificationResult.msName);
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
                    m_xEditWindow->getEditView().InsertParaBreak();

                // Set paragraph font weight
                FontWeight eWeight = (rClassificationResult.msName == "BOLD") ? WEIGHT_BOLD : WEIGHT_NORMAL;

                ClassificationEditEngine& rEdEngine = m_xEditWindow->getEditEngine();
                std::unique_ptr<SfxItemSet> pSet(new SfxItemSet(rEdEngine.GetParaAttribs(nParagraph)));
                pSet->Put(SvxWeightItem(eWeight, EE_CHAR_WEIGHT));
                rEdEngine.SetParaAttribs(nParagraph, *pSet);
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
    const EditEngine& rEditEngine = m_xEditWindow->getEditEngine();

    for (sal_Int32 nParagraph = 0; nParagraph < rEditEngine.GetParagraphCount(); ++nParagraph)
    {
        sal_uInt16 nFieldCount = rEditEngine.GetFieldCount(nParagraph);
        for (sal_uInt16 nField = 0; nField < nFieldCount; ++nField)
        {
            EFieldInfo aFieldInfo = rEditEngine.GetFieldInfo(nParagraph, nField);
            if (aFieldInfo.pFieldItem)
            {
                const ClassificationField* pClassificationField = dynamic_cast<const ClassificationField*>(aFieldInfo.pFieldItem->GetField());
                if (pClassificationField && pClassificationField->meType == ClassificationType::CATEGORY)
                {
                    m_xOkButton->set_sensitive(true);
                    return;
                }
            }
        }
    }

    // Category field in the text edit has been deleted, so reset the list boxes
    m_xOkButton->set_sensitive(false);
    m_xClassificationListBox->set_active(-1);
    m_xInternationalClassificationListBox->set_active(-1);
}

std::vector<ClassificationResult> ClassificationDialog::getResult()
{
    std::vector<ClassificationResult> aClassificationResults;

    ClassificationEditEngine& rEdEngine = m_xEditWindow->getEditEngine();
    std::unique_ptr<EditTextObject> pEditText(rEdEngine.CreateTextObject());

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
            SfxItemSet aItemSet(rEdEngine.GetParaAttribs(nCurrentParagraph));
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
        const OUString sDisplayString = rEdEngine.GetText(aSelection);
        if (!sDisplayString.isEmpty())
        {
            const ClassificationField* pClassificationField = pFieldItem ? dynamic_cast<const ClassificationField*>(pFieldItem->GetField()) : nullptr;

            if (pClassificationField)
            {
                aClassificationResults.push_back({ pClassificationField->meType, pClassificationField->msFullClassName,
                                                   pClassificationField->msDescription, pClassificationField->msIdentifier });
            }
            else
            {
                aClassificationResults.push_back({ ClassificationType::TEXT, sDisplayString, sDisplayString, OUString() });
            }
        }
    }

    return aClassificationResults;
}

IMPL_LINK(ClassificationDialog, SelectClassificationHdl, weld::ComboBox&, rBox, void)
{
    const sal_Int32 nSelected = rBox.get_active();
    if (nSelected < 0 || m_nCurrentSelectedCategory == nSelected)
        return;

    std::unique_ptr<EditTextObject> pEditText(m_xEditWindow->getEditEngine().CreateTextObject());
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
        m_xEditWindow->getEditView().SetSelection(aExistingFieldSelection);

    insertCategoryField(nSelected);

    // Change category to the new selection
    m_xInternationalClassificationListBox->set_active(nSelected);
    m_xClassificationListBox->set_active(nSelected);
    m_nCurrentSelectedCategory = nSelected;
}

IMPL_LINK(ClassificationDialog, SelectMarkingHdl, weld::TreeView&, rBox, bool)
{
    sal_Int32 nSelected = rBox.get_selected_index();
    if (nSelected >= 0)
    {
        const OUString aString = maHelper.GetMarkings()[nSelected];
        insertField(ClassificationType::MARKING, aString, aString);
    }
    return true;
}

IMPL_LINK(ClassificationDialog, SelectIPPartNumbersHdl, weld::TreeView&, rBox, bool)
{
    sal_Int32 nSelected = rBox.get_selected_index();
    if (nSelected >= 0)
    {
        OUString sString = maHelper.GetIntellectualPropertyPartNumbers()[nSelected];
        m_xIntellectualPropertyPartEdit->replace_selection(sString);
        m_xIntellectualPropertyPartEdit->grab_focus();
    }
    return true;
}

IMPL_LINK(ClassificationDialog, SelectRecentlyUsedHdl, weld::ComboBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.get_active();
    if (nSelected >= 0)
    {
        m_xEditWindow->getEditEngine().Clear();
        readIn(m_aRecentlyUsedValuesCollection[nSelected]);
    }
}

IMPL_LINK(ClassificationDialog, SelectIPPartHdl, weld::TreeView&, rBox, bool)
{
    const sal_Int32 nSelected = rBox.get_selected_index();
    if (nSelected >= 0)
    {
        const OUString sString = maHelper.GetIntellectualPropertyParts()[nSelected];
        m_xIntellectualPropertyPartEdit->replace_selection(sString);
        m_xIntellectualPropertyPartEdit->grab_focus();
    }
    return true;
}

IMPL_LINK(ClassificationDialog, ButtonClicked, weld::Button&, rButton, void)
{
    if (&rButton == m_xSignButton.get())
    {
        m_aParagraphSignHandler();
    }
    else if (&rButton == m_xIntellectualPropertyPartAddButton.get())
    {
        const OUString sString = m_xIntellectualPropertyPartEdit->get_text();
        insertField(ClassificationType::INTELLECTUAL_PROPERTY_PART, sString, sString);
    }
}

IMPL_LINK_NOARG(ClassificationDialog, OkHdl, weld::Button&, void)
{
    writeRecentlyUsed();
    m_xDialog->response(RET_OK);
}

IMPL_LINK_NOARG(ClassificationDialog, SelectToolboxHdl, weld::ToggleButton&, void)
{
    m_xEditWindow->InvertSelectionWeight();
}

IMPL_LINK_NOARG(ClassificationDialog, EditWindowModifiedHdl, LinkParamNone*, void)
{
    toggleWidgetsDependingOnCategory();
}

IMPL_STATIC_LINK(ClassificationDialog, ExpandedHdl, weld::Expander&, rExpander, void)
{
    std::shared_ptr<comphelper::ConfigurationChanges> aConfigurationChanges(comphelper::ConfigurationChanges::create());
    officecfg::Office::Common::Classification::IntellectualPropertySectionExpanded::set(rExpander.get_expanded(), aConfigurationChanges);
    aConfigurationChanges->commit();
}

} // end svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
