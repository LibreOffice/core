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
#ifndef _SVX_SIDEBAR_COLOR_POPUP_HXX_
#define _SVX_SIDEBAR_COLOR_POPUP_HXX_

#include "svx/sidebar/Popup.hxx"

#include <tools/color.hxx>


namespace svx { namespace sidebar {

/** Popup control that displays all named colors in a matrix.
    The number of rows and columns of the matrix are computed from
    the number of named colors so that both have roughly the same
    value.

    The ColorPopup uses ColorControl as control for its content.
*/
class SVX_DLLPUBLIC ColorPopup
    : public Popup
{
public :
    ColorPopup (
        Window* pParent,
        const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator);
    virtual ~ColorPopup (void);

    void SetCurrentColor (
        const Color aCurrentColor,
        const bool bIsColorAvailable);
};

} } // end of namespace svx::sidebar

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
