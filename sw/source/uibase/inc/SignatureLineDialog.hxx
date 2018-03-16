/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_INC_SIGNATURELINEDIALOG_HXX
#define INCLUDED_SW_INC_SIGNATURELINEDIALOG_HXX

#include <svx/stddlg.hxx>
#include <swdllapi.h>
#include <vcl/weld.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/Reference.hxx>

class SwView;

class SW_DLLPUBLIC SignatureLineDialog : public weld::GenericDialogController
{
public:
    SignatureLineDialog(weld::Window* pParent, SwView& rView);

    short execute();

private:
    std::unique_ptr<weld::Entry> m_xEditName;
    std::unique_ptr<weld::Entry> m_xEditTitle;
    std::unique_ptr<weld::Entry> m_xEditEmail;
    std::unique_ptr<weld::TextView> m_xEditInstructions;
    std::unique_ptr<weld::CheckButton> m_xCheckboxCanAddComments;
    std::unique_ptr<weld::CheckButton> m_xCheckboxShowSignDate;
    SwView& mrView;
    css::uno::Reference<css::beans::XPropertySet> m_xExistingShapeProperties;
    OUString m_aSignatureLineId;

    OUString getSignatureImage();
    void Apply();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
