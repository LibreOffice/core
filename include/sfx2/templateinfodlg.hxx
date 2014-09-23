/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef INCLUDED_SFX2_TEMPLATEINFODLG_HXX
#define INCLUDED_SFX2_TEMPLATEINFODLG_HXX

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/layout.hxx>

namespace svtools {
    class ODocumentInfoPreview;
}

namespace com{ namespace sun { namespace star { namespace awt   { class XWindow; } } } }
namespace com{ namespace sun { namespace star { namespace frame { class XFrame2; } } } }

class SfxTemplateInfoDlg : public ModalDialog
{
public:

    SfxTemplateInfoDlg (vcl::Window *pParent = NULL);

    virtual ~SfxTemplateInfoDlg ();

    void loadDocument (const OUString &rURL);

protected:

    DECL_LINK (CloseHdl, void*);

private:

    PushButton*   mpBtnClose;
    VclBox*      mpBox;

    vcl::Window *mpPreviewView;  // gets released when xWindows get destroyed (dont delete in constructor)
    svtools::ODocumentInfoPreview *mpInfoView;

    ::com::sun::star::uno::Reference < ::com::sun::star::frame::XFrame2 > m_xFrame;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > xWindow;
};

#endif // INCLUDED_SFX2_TEMPLATEINFODLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
