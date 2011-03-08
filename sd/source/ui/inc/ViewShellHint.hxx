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

#ifndef SD_VIEW_SHELL_HINT_HXX
#define SD_VIEW_SHELL_HINT_HXX

#include <svl/hint.hxx>

namespace sd {

/** Local derivation of the SfxHint class that defines some hint ids that
    are used by the ViewShell class and its decendants.
*/
class ViewShellHint
    : public SfxHint
{
public:
    enum HintId {
        // Indicate that a page resize is about to begin.
        HINT_PAGE_RESIZE_START,
        // Indicate that a page resize has been completed.
        HINT_PAGE_RESIZE_END,
        // Indicate that an edit mode change is about to begin.
        HINT_CHANGE_EDIT_MODE_START,
        // Indicate that an edit mode change has been completed.
        HINT_CHANGE_EDIT_MODE_END,

        HINT_COMPLEX_MODEL_CHANGE_START,
        HINT_COMPLEX_MODEL_CHANGE_END
    };

    TYPEINFO();

    ViewShellHint (HintId nHintId);

    HintId GetHintId (void) const;

private:
    HintId meHintId;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
