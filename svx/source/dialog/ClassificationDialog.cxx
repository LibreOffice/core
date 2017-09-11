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
#include <svl/itemset.hxx>

namespace svx {

namespace {

const SvxFieldItem* findField(editeng::Section const & rSection)
{
    for (SfxPoolItem const * pPool: rSection.maAttributes)
    {
        if (pPool->Which() == EE_FEATURE_FIELD)
            return static_cast<const SvxFieldItem*>(pPool);
    }
    return nullptr;
}

} // end anonymous namespace

ClassificationDialog::ClassificationDialog(vcl::Window* pParent)
    : ModalDialog(pParent, "AdvancedDocumentClassificationDialog", "svx/ui/classificationdialog.ui")
    , maHelper(SfxObjectShell::Current()->getDocProperties())
{
    get(m_pEditWindow, "classificationEditWindow");
    get(m_pBoldButton, "boldButton");
    get(m_pClassificationListBox, "classificationCB");
    get(m_pInternationalClassificationListBox, "internationalClassificationCB");
    get(m_pMarkingListBox, "markingCB");
    get(m_pIntellectualPropertyPartNumberListBox, "intellectualPropertyPartNumberCB");
    get(m_pIntellectualPropertyPartListBox, "intellectualPropertyPartLB");
    get(m_pIntellectualPropertyPartAddButton, "intellectualPropertyPartAddButton");

    m_pBoldButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));
    m_pIntellectualPropertyPartAddButton->SetClickHdl(LINK(this, ClassificationDialog, ButtonClicked));

    m_pClassificationListBox->SetSelectHdl(LINK(this, ClassificationDialog, SelectClassificationHdl));
    for (const OUString& rName : maHelper.GetBACNames())
        m_pClassificationListBox->InsertEntry(rName);
    m_pClassificationListBox->EnableAutoSize(true);

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
    ModalDialog::dispose();
}

short ClassificationDialog::Execute()
{
    return ModalDialog::Execute();
}

std::vector<ClassificationResult> ClassificationDialog::getResult()
{
    std::vector<ClassificationResult> aClassificationResults;

    std::unique_ptr<EditTextObject> pEditText(m_pEditWindow->pEdEngine->CreateTextObject());

    std::vector<editeng::Section> aSections;
    pEditText->GetAllSections(aSections);

    for (editeng::Section const & rSection : aSections)
    {
        const SvxFieldItem* rField = findField(rSection);
        if (rField)
        {
        }
        else
        {
            ESelection aSelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd);
            OUString sString = m_pEditWindow->pEdEngine->GetText(aSelection);
            aClassificationResults.push_back({ ClassificationType::TEXT, sString, rSection.mnParagraph });
        }
    }
    return aClassificationResults;
}

IMPL_LINK(ClassificationDialog, SelectClassificationHdl, ListBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.GetSelectEntryPos();
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
                if (pClassificationField && pClassificationField->meType == ClassificationType::CLASSIFICATION)
                {
                    m_pEditWindow->pEdView->SetSelection(ESelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd));
                }
            }
        }

        OUString aString = maHelper.GetBACNames()[nSelected];
        ClassificationField aField(ClassificationType::CLASSIFICATION, aString);
        m_pEditWindow->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
    }
}

IMPL_LINK(ClassificationDialog, SelectMarkingHdl, ListBox&, rBox, void)
{
    sal_Int32 nSelected = rBox.GetSelectEntryPos();
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
                if (pClassificationField && pClassificationField->meType == ClassificationType::MARKINGS)
                {
                    m_pEditWindow->pEdView->SetSelection(ESelection(rSection.mnParagraph, rSection.mnStart, rSection.mnParagraph, rSection.mnEnd));
                }
            }
        }

        OUString aString = maHelper.GetMarkings()[nSelected];
        ClassificationField aField(ClassificationType::MARKINGS, aString);
        m_pEditWindow->InsertField(SvxFieldItem(aField, EE_FEATURE_FIELD));
    }
}

IMPL_LINK(ClassificationDialog, SelectIPPartNumbersHdl, ListBox&, rBox, void)
{
    printf ("DoubleClickIPPartHdl\n");
}

IMPL_LINK(ClassificationDialog, DoubleClickIPPartHdl, ListBox&, rBox, void)
{
    printf ("DoubleClickIPPartHdl\n");
}

IMPL_LINK(ClassificationDialog, ButtonClicked, Button*, pButton, void)
{
    if (pButton == m_pBoldButton)
    {
        m_pEditWindow->InvertSelectionWeight();
    }
    else if (pButton == m_pIntellectualPropertyPartAddButton)
    {
    }
}

} // end sfx2

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
