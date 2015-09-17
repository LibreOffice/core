/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#include <LibreOfficeKit/LibreOfficeKit.h>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <memory>
#include "../../source/inc/desktopdllapi.h"

using namespace css;
using namespace boost;

namespace desktop {
    struct DESKTOP_DLLPUBLIC LibLODocument_Impl : public _LibreOfficeKitDocument
    {
        uno::Reference<css::lang::XComponent> mxComponent;
        std::shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;

        explicit LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent);
        ~LibLODocument_Impl();
    };
}
