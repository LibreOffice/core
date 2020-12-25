/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/BinaryDataContainerTools.hxx>
#include <graphic/UnoBinaryDataContainer.hxx>

namespace vcl
{
BinaryDataContainer convertUnoBinaryDataContainer(
    const css::uno::Reference<css::util::XBinaryDataContainer>& rxBinaryDataContainer)
{
    BinaryDataContainer aBinaryDataContainer;
    UnoBinaryDataContainer* pUnoBinaryDataContainer
        = comphelper::getUnoTunnelImplementation<UnoBinaryDataContainer>(rxBinaryDataContainer);
    if (pUnoBinaryDataContainer)
        aBinaryDataContainer = pUnoBinaryDataContainer->getBinaryDataContainer();
    return aBinaryDataContainer;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
