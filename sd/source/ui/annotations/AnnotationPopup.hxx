/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <svx/annotation/IAnnotationPopup.hxx>
#include <vcl/weld.hxx>

namespace com::sun::star::office
{
class XAnnotation;
}
namespace sdr::annotation
{
class Annotation;
}

namespace sd
{
class View;
class AnnotationWindow;

class AnnotationPopup final : public sdr::annotation::IAnnotationPopup
{
public:
    AnnotationPopup(rtl::Reference<sdr::annotation::Annotation> const& xAnnotation);
    virtual ~AnnotationPopup() override;

    void openPopup() override;
    void closePopup() override;

private:
    DECL_LINK(PopupModeEndHdl, weld::Popover&, void);

    std::unique_ptr<AnnotationWindow> mpAnnotationWindow;
};

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
