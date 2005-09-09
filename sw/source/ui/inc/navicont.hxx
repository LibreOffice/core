/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: navicont.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:52:04 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _NAVICONT_HXX
#define _NAVICONT_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

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
    USHORT          nDefDrag;   // Description enthaelt defaultDragType

public:
    NaviContentBookmark();
    NaviContentBookmark( const String &rUrl, const String& rDesc,
                            USHORT nDragType, const SwDocShell* );

    const String&   GetURL() const              { return aUrl; }
    const String&   GetDescription() const      { return aDescr; }
    USHORT          GetDefaultDragType() const  { return nDefDrag; }
    long            GetDocShell() const         { return nDocSh; }
    void            Copy( TransferDataContainer& rData ) const;
    BOOL            Paste( TransferableDataHelper& rData );
};

#endif
