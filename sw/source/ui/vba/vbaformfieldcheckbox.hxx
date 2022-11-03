/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_UI_VBA_VBAFORMFIELDCHECKBOX_HXX
#define INCLUDED_SW_SOURCE_UI_VBA_VBAFORMFIELDCHECKBOX_HXX

#include <ooo/vba/word/XCheckBox.hpp>

#include <vbahelper/vbahelperinterface.hxx>

#include <IDocumentMarkAccess.hxx>

typedef InheritedHelperInterfaceWeakImpl<ooo::vba::word::XCheckBox> SwVbaFormFieldCheckBox_BASE;

class SwVbaFormFieldCheckBox : public SwVbaFormFieldCheckBox_BASE
{
private:
    sw::mark::ICheckboxFieldmark* m_pCheckBox;

public:
    /// @throws css::uno::RuntimeException
    SwVbaFormFieldCheckBox(const css::uno::Reference<ooo::vba::XHelperInterface>& rParent,
                           const css::uno::Reference<css::uno::XComponentContext>& rContext,
                           sw::mark::IFieldmark& rFormField);
    ~SwVbaFormFieldCheckBox() override;

    // XCheckBox
    virtual OUString SAL_CALL getDefaultPropertyName() override;

    // Default member: True if the specified form field object is a valid check box form field
    virtual sal_Bool SAL_CALL getValid() override;

    virtual sal_Bool SAL_CALL getAutoSize() override;
    virtual void SAL_CALL setAutoSize(sal_Bool bSet) override;
    // Returns the default check box value
    virtual sal_Bool SAL_CALL getDefault() override;
    virtual void SAL_CALL setDefault(sal_Bool bSet) override;
    // Returns the size of a check box, in points
    virtual sal_Int32 SAL_CALL getSize() override;
    virtual void SAL_CALL setSize(sal_Int32 nSet) override;

    virtual sal_Bool SAL_CALL getValue() override;
    virtual void SAL_CALL setValue(sal_Bool bSet) override;

    // XHelperInterface
    OUString getServiceImplName() override;
    css::uno::Sequence<OUString> getServiceNames() override;
};
#endif // INCLUDED_SW_SOURCE_UI_VBA_VBAFORMFIELDCHECKBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
