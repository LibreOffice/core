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

#ifndef SD_FACTORY_IDS_HXX
#define SD_FACTORY_IDS_HXX

namespace sd {

/** This are ids used by SfxTopFrame::Create() or CreateViewFrame() to
    select a factory to create an instance of an SfxViewShell super
    class.  This allows the caller to create a view shell directly
    with a certain view instead of first create a default view and
    then switch to the desired view.
*/
enum ViewShellFactoryIds
{
    IMPRESS_FACTORY_ID = 1,
    DRAW_FACTORY_ID = 1,
    SLIDE_SORTER_FACTORY_ID = 2,
    OUTLINE_FACTORY_ID = 3,
    PRESENTATION_FACTORY_ID = 4
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
