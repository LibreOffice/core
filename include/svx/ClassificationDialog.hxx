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
#include <vcl/edit.hxx>
#include <svx/svxdllapi.h>
#include <svx/ClassificationEditView.hxx>
#include <sfx2/classificationhelper.hxx>
#include <svx/ClassificationField.hxx>

namespace svx {

class SVX_DLLPUBLIC ClassificationDialog : public ModalDialog
{
private:
    VclPtr<ClassificationEditView> m_pEditWindow;
    VclPtr<PushButton> m_pSignButton;
    VclPtr<PushButton> m_pBoldButton;
    VclPtr<ListBox> m_pClassificationListBox;
    VclPtr<ListBox> m_pInternationalClassificationListBox;
    VclPtr<ListBox> m_pMarkingListBox;
    VclPtr<ListBox> m_pIntellectualPropertyPartListBox;
    VclPtr<ListBox> m_pIntellectualPropertyPartNumberListBox;
    VclPtr<PushButton> m_pIntellectualPropertyPartAddButton;
    VclPtr<Edit> m_pIntellectualPropertyPartEdit;

    SfxClassificationHelper maHelper;
    SfxClassificationHelper maInternationalHelper;

    DECL_LINK(ButtonClicked, Button*, void);
    DECL_LINK(SelectClassificationHdl, ListBox&, void);
    DECL_LINK(SelectMarkingHdl, ListBox&, void);
    DECL_LINK(SelectIPPartNumbersHdl, ListBox&, void);
    DECL_LINK(DoubleClickIPPartHdl, ListBox&, void);

    void insertField(ClassificationType eType, OUString const & rString);

public:
    ClassificationDialog(vcl::Window* pParent);
    virtual ~ClassificationDialog() override;
    virtual void dispose() override;

    std::vector<ClassificationResult> getResult();
    void setupValues(std::vector<ClassificationResult> const & rInput);
};

} // end svx namespace

#endif // INCLUDED_SVX_CLASSIFICATIONDIALOG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
