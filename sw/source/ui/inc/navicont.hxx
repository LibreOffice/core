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

#ifndef _NAVICONT_HXX
#define _NAVICONT_HXX

#include <tools/string.hxx>

class SwDocShell;
class TransferDataContainer;
class TransferableDataHelper;

/*  [Beschreibung]
    Navigator-Bookmark zur eindeutigen Identifizierung im Sw
*/

class NaviContentBookmark
{
    String          aUrl;       // URL inkl. Sprungmarke
    String          aDescr;     // Description
    long            nDocSh;     // Adresse der DocShell
    sal_uInt16          nDefDrag;   // Description enthaelt defaultDragType

public:
    NaviContentBookmark();
    NaviContentBookmark( const String &rUrl, const String& rDesc,
                            sal_uInt16 nDragType, const SwDocShell* );

    const String&   GetURL() const              { return aUrl; }
    const String&   GetDescription() const      { return aDescr; }
    sal_uInt16          GetDefaultDragType() const  { return nDefDrag; }
    long            GetDocShell() const         { return nDocSh; }
    void            Copy( TransferDataContainer& rData ) const;
    sal_Bool            Paste( TransferableDataHelper& rData );
};

#endif
