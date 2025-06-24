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

#pragma once

#include <framework/AbstractResource.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/rendering/XCanvas.hpp>
#include <comphelper/compbase.hxx>
#include <sddllapi.h>

namespace sd::framework
{
/** A view in the drawing framework is any object that paints into a pane.
    <p>Typical examples are the Impress views that show a graphical
    representation of a document.  But the task pane, which is primarily a
    container of dialogs, is a view as well.</p>
    <p>Luckily the drawing framework does not need to know much about what a
    view is.  It just needs to identify view objects and a typesafe way to
    reference them.</p>
    <p>The URL prefix of views is <code>private:resource/view</code></p>
*/
class SD_DLLPUBLIC AbstractView : public AbstractResource
{
public:
    virtual ~AbstractView() override;
};

} // end of namespace sd::framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
