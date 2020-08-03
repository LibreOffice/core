/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <sax/fshelper.hxx>

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <com/sun/star/uno/Reference.hxx>

#include "epptooxml.hxx"

namespace oox::core
{
void WriteAnimations(const ::sax_fastparser::FSHelperPtr& pFS,
                     const css::uno::Reference<css::drawing::XDrawPage>& rXDrawPage,
                     PowerPointExport& rExport);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
