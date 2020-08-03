/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <drawinglayer/drawinglayerdllapi.h>
#include <rtl/ustring.hxx>

namespace com::sun::star::drawing { class XShape; }
namespace com::sun::star::drawing { class XShapes; }
namespace com::sun::star::uno { template <typename > class Reference; }

struct DRAWINGLAYER_DLLPUBLIC XShapeDumper
{
    XShapeDumper() = delete;
    static OUString dump(const css::uno::Reference<css::drawing::XShapes>& xPageShapes, bool bDumpInteropProperties=false);
    static OUString dump(const css::uno::Reference<css::drawing::XShape>& xPageShapes, bool bDumpInteropProperties=false);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
