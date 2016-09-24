/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_SIDEBAR_TOOLS_HXX
#define INCLUDED_SFX2_SIDEBAR_TOOLS_HXX

#include <vcl/image.hxx>
#include <vcl/gradient.hxx>
#include <tools/svborder.hxx>

#include <sfx2/dllapi.h>

#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/util/URL.hpp>


namespace sfx2 { namespace sidebar {

class SFX2_DLLPUBLIC Tools
{
public:
    static Image GetImage (
        const ::rtl::OUString& rsImageURL,
        const ::rtl::OUString& rsHighContrastImageURL,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static Image GetImage (
        const ::rtl::OUString& rsURL,
        const css::uno::Reference<css::frame::XFrame>& rxFrame);

    static css::awt::Gradient VclToAwtGradient (const Gradient& rGradient);
    static Gradient AwtToVclGradient (const css::awt::Gradient& rGradient);

    static css::util::URL GetURL (const ::rtl::OUString& rsCommand);
    static css::uno::Reference<css::frame::XDispatch> GetDispatch (
        const css::uno::Reference<css::frame::XFrame>& rxFrame,
        const css::util::URL& rURL);

    static ::rtl::OUString GetModuleName (
        const css::uno::Reference<css::frame::XController>& rxFrame);
};


} } // end of namespace sfx2::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
