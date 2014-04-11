/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHART2_GL3DHELPER_HXX
#define CHART2_GL3DHELPER_HXX

#include <charttoolsdllapi.hxx>

#include <com/sun/star/chart2/XDiagram.hpp>

namespace chart {

class OOO_DLLPUBLIC_CHARTTOOLS GL3DHelper
{
public:
    static bool isGL3DDiagram( const css::uno::Reference<css::chart2::XDiagram>& xDiagram );
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
