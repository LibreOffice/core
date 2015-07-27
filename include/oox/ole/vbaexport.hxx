/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __INCLUDED_INCLUDE_OOX_OLE_VBAEXPORT_HXX__
#define __INCLUDED_INCLUDE_OOX_OLE_VBAEXPORT_HXX__

#include <com/sun/star/uno/XInterface.hpp>
#include <oox/dllapi.h>

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace frame { class XModel; }
} } }

class OOX_DLLPUBLIC VbaExport
{
public:
    VbaExport(css::uno::Reference<css::frame::XModel> xModel);

    void exportVBA();

private:

    css::uno::Reference<css::container::XNameContainer>
        getBasicLibrary();

    css::uno::Reference<css::frame::XModel> mxModel;

    OUString maProjectName;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
