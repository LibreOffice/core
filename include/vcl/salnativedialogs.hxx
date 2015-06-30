/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_SALNATIVEDIALOGS_HXX
#define INCLUDED_VCL_SALNATIVEDIALOGS_HXX

#include <vcl/dllapi.h>

class VCL_DLLPUBLIC MessageDialogImpl : public virtual VclPtrRefCountBase
{
public:
    virtual short Execute() = 0;
    virtual void response(short nResponseId) = 0;
    virtual OUString get_primary_text() const = 0;
    virtual OUString get_secondary_text() const = 0;
    virtual void set_title(const OUString &rTitle) = 0;
    virtual void set_primary_text(const OUString &rPrimaryString) = 0;
    virtual void set_secondary_text(const OUString &rSecondaryString) = 0;
    virtual void dispose() = 0;
    virtual ~MessageDialogImpl() = 0;
};

enum VclButtonsType
{
    VCL_BUTTONS_NONE,
    VCL_BUTTONS_OK,
    VCL_BUTTONS_CLOSE,
    VCL_BUTTONS_CANCEL,
    VCL_BUTTONS_YES_NO,
    VCL_BUTTONS_OK_CANCEL
};

enum VclMessageType
{
    VCL_MESSAGE_INFO,
    VCL_MESSAGE_WARNING,
    VCL_MESSAGE_QUESTION,
    VCL_MESSAGE_ERROR
};

class VCL_DLLPUBLIC MessageDialog : public VclPtrRefCountBase
{
private:
    VclPtr<MessageDialogImpl> m_xImpl;
public:
    MessageDialog(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription);
    MessageDialog(vcl::Window* pParent,
        const OUString &rMessage,
        VclMessageType eMessageType = VCL_MESSAGE_ERROR,
        VclButtonsType eButtonsType = VCL_BUTTONS_OK);
    short Execute() { return m_xImpl->Execute(); }
    ///Emitted when an action widget is clicked
    void response(short nResponseId) { m_xImpl->response(nResponseId); }
    OUString get_primary_text() const { return m_xImpl->get_primary_text(); }
    OUString get_secondary_text() const { return m_xImpl->get_secondary_text(); }
    void set_title(const OUString &rTitle) { return m_xImpl->set_title(rTitle); }
    void set_primary_text(const OUString &rPrimaryString) { m_xImpl->set_primary_text(rPrimaryString); }
    void set_secondary_text(const OUString &rSecondaryString) { m_xImpl->set_secondary_text(rSecondaryString); }
    virtual void dispose() SAL_OVERRIDE { m_xImpl->dispose(); }
    ~MessageDialog();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
