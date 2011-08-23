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

/*************************************************************************
|* Enthielt frueher auch die Klasse SdPresentationLayoutDlg (wird jetzt nicht
|* mehr benoetigt). Dieses File ist auf ungeklaerte Weise aus dem PVCS
|* verschwunden und wird jetzt neu eingecheckt und enthaelt nur noch den enum.
\************************************************************************/

#ifndef _SD_PRLAYOUT_HXX_
#define _SD_PRLAYOUT_HXX_
namespace binfilter {

enum PresentationObjects
{
    PO_TITLE,
    PO_BACKGROUND,
    PO_BACKGROUNDOBJECTS,
    PO_OUTLINE_1,
    PO_OUTLINE_2,
    PO_OUTLINE_3,
    PO_OUTLINE_4,
    PO_OUTLINE_5,
    PO_OUTLINE_6,
    PO_OUTLINE_7,
    PO_OUTLINE_8,
    PO_OUTLINE_9,
    PO_NOTES,
    PO_SUBTITLE
};

} //namespace binfilter
#endif // _SD_PRLAYOUT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
