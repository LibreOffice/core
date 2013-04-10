/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "GroupSlidesDialog.hxx"

#include <com/sun/star/graphic/GraphicProvider.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/componentcontext.hxx>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>

#include <sfx2/filedlghelper.hxx>
#include <tools/urlobj.hxx>

#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <vcl/msgbox.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::presentation;

namespace sd
{

SdGroupSlidesDialog::SdGroupSlidesDialog(Window* pWindow, SdDrawDocument* pActDoc,
                                         const std::vector< SdPage * > &rPages )
  : ModalDialog(pWindow, "GroupSlidesDialog", "modules/simpress/ui/groupslides.ui"),
    pDoc(pActDoc),
    maPages( rPages )
{
    get(pCancelBtn, "cancel_btn");
    pCancelBtn->SetClickHdl(LINK(this, SdGroupSlidesDialog, CancelHdl));
}

SdGroupSlidesDialog::~SdGroupSlidesDialog()
{
}

IMPL_LINK_NOARG(SdGroupSlidesDialog, CancelHdl)
{
    EndDialog(0);
    return 0;
}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
