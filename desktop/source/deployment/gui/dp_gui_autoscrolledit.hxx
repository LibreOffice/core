/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_AUTOSCROLLEDIT_HXX
#define INCLUDED_DESKTOP_SOURCE_DEPLOYMENT_GUI_DP_GUI_AUTOSCROLLEDIT_HXX

#include "svtools/svmedit2.hxx"
#include "svl/lstner.hxx"

namespace dp_gui {

/** This control shows automatically the vertical scroll bar if text is inserted,
    that does not fit into the text area. In the resource one uses MultiLineEdit
    and needs to set VScroll = TRUE
*/
class AutoScrollEdit : public ExtMultiLineEdit, public SfxListener
{
public:
    AutoScrollEdit( Window* pParent, const ResId& rResId );
    ~AutoScrollEdit();

    using ExtMultiLineEdit::Notify;
    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );
};

} // namespace dp_gui

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
