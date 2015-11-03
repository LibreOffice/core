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
#include <boost/shared_ptr.hpp>
#include "../../source/inc/desktopdllapi.h"
#include <osl/thread.h>

using namespace css;
using namespace boost;

namespace desktop {
    struct DESKTOP_DLLPUBLIC LibLODocument_Impl : public _LibreOfficeKitDocument
    {
        uno::Reference<css::lang::XComponent> mxComponent;
        shared_ptr< LibreOfficeKitDocumentClass > m_pDocumentClass;
        LibreOfficeKitCallback mpCallback;
        void *mpCallbackData;

        explicit LibLODocument_Impl(const uno::Reference <css::lang::XComponent> &xComponent);
        ~LibLODocument_Impl();
    };

    struct DESKTOP_DLLPUBLIC LibLibreOffice_Impl : public _LibreOfficeKit
    {
        OUString maLastExceptionMsg;
        boost::shared_ptr< LibreOfficeKitClass > m_pOfficeClass;
        oslThread maThread;
        LibreOfficeKitCallback mpCallback;
        void *mpCallbackData;

        LibLibreOffice_Impl();
    };
}
