/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SVX_CLASSIFICATIONDIALOG_HXX
#define INCLUDED_SVX_CLASSIFICATIONDIALOG_HXX

#include <sal/config.h>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/edit.hxx>
#include <vcl/layout.hxx>
#include <svx/svxdllapi.h>
#include <svx/ClassificationEditView.hxx>
#include <sfx2/classificationhelper.hxx>
#include <svx/ClassificationField.hxx>

namespace svx {

class IntellectualPropertyPartEdit : public Edit
{
public:
    IntellectualPropertyPartEdit(vcl::Window* pParent);

protected:
    virtual void KeyInput(const KeyEvent &rKEvt) override;
};

class SVX_DLLPUBLIC ClassificationDialog : public ModalDialog
{
private:
    VclPtr<PushButton> m_pOkButton;
    VclPtr<ClassificationEditView> m_pEditWindow;
    VclPtr<PushButton> m_pSignButton;
    VclPtr<ToolBox> m_pToolBox;
    VclPtr<ListBox> m_pClassificationListBox;
    VclPtr<ListBox> m_pRecentlyUsedListBox;
    VclPtr<ListBox> m_pInternationalClassificationListBox;
    VclPtr<FixedText> m_pMarkingLabel;
    VclPtr<ListBox> m_pMarkingListBox;
    VclPtr<ListBox> m_pIntellectualPropertyPartListBox;
    VclPtr<ListBox> m_pIntellectualPropertyPartNumberListBox;
    VclPtr<PushButton> m_pIntellectualPropertyPartAddButton;
    VclPtr<IntellectualPropertyPartEdit> m_pIntellectualPropertyPartEdit;
    VclPtr<VclExpander> m_pIntellectualPropertyExpander;

    SfxClassificationHelper maHelper;
    SfxClassificationHelper maInternationalHelper;

    const bool m_bPerParagraph;
    const std::function<void()> m_aParagraphSignHandler;

    sal_Int32 m_nCurrentSelectedCategory;
    sal_Int16 m_nInsertMarkings;

    DECL_LINK(ButtonClicked, Button*, void);
    DECL_LINK(SelectToolboxHdl, ToolBox*, void);
    DECL_LINK(SelectClassificationHdl, ListBox&, void);
    DECL_LINK(SelectMarkingHdl, ListBox&, void);
    DECL_LINK(SelectIPPartNumbersHdl, ListBox&, void);
    DECL_LINK(SelectRecentlyUsedHdl, ListBox&, void);
    DECL_LINK(SelectIPPartHdl, ListBox&, void);
    DECL_LINK(EditWindowModifiedHdl, LinkParamNone*, void);
    DECL_LINK(ExpandedHdl, VclExpander&, void);

    void insertField(ClassificationType eType, OUString const & rString, OUString const & rFullString, OUString const & rIdentifier = OUString());
    void insertCategoryField(sal_Int32 nID);

    std::vector<std::vector<ClassificationResult>> m_aRecentlyUsedValuesCollection;
    std::vector<ClassificationResult> m_aInitialValues;

    void readIn(std::vector<ClassificationResult> const & rInput);
    void readRecentlyUsed();
    void writeRecentlyUsed();
    void toggleWidgetsDependingOnCategory();

public:
    ClassificationDialog(vcl::Window* pParent, bool bPerParagraph, const std::function<void()>& rParagraphSignHandler = [](){});
    ~ClassificationDialog() override;

    void dispose() override;
    short Execute() override;

    std::vector<ClassificationResult> getResult();
    void setupValues(std::vector<ClassificationResult> const & rInput);
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
