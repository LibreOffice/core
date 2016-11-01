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
#ifndef INCLUDED_SFX2_SOURCE_NOTEBOOKBAR_BIGTOOLBOX_HXX
#define INCLUDED_SFX2_SOURCE_NOTEBOOKBAR_BIGTOOLBOX_HXX

#include <sfx2/sidebar/SidebarToolBox.hxx>
#include <sfx2/dllapi.h>
#include <vcl/toolbox.hxx>
#include <com/sun/star/frame/XDispatch.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XToolbarController.hpp>
#include <com/sun/star/util/URL.hpp>
#include <map>


namespace sfx2 { namespace notebookbar {

/** BigToolBox
*/

class SFX2_DLLPUBLIC BigToolBox : public sfx2::sidebar::SidebarToolBox
{
public:
    explicit BigToolBox(vcl::Window* pParent);
};


} } // end of namespace sfx2::notebookbar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
