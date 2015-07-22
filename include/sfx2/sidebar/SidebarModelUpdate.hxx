/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_INCLUDE_SFX2_SIDEBAR_SIDEBARUPDATEMODEL_HXX
#define INCLUDED_INCLUDE_SFX2_SIDEBAR_SIDEBARUPDATEMODEL_HXX

#include <com/sun/star/frame/XModel.hpp>

#include <sfx2/dllapi.h>

namespace sfx2 { namespace sidebar {

class SFX2_DLLPUBLIC SidebarModelUpdate
{
public:
    virtual ~SidebarModelUpdate();
    virtual void updateModel(css::uno::Reference<css::frame::XModel> xModel) = 0;
};

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
