/*************************************************************************
 *
 *  $RCSfile: navicont.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:14:41 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _NAVICONT_HXX
#define _NAVICONT_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
class SwDocShell;
class SotDataObject;

/*-----------------06.02.97 19.10-------------------

--------------------------------------------------*/


class NaviContentBookmark

/*  [Beschreibung]

    Navigator-Bookmark zur eindeutigen Identifizierung im Sw

*/

{
    String          aUrl;       // URL inkl. Sprungmarke
    String          aDescr;     // Description
    long            nDocSh;     // Adresse der DocShell
    USHORT          nDefDrag;   // Description enthaelt defaultDragType

protected:

public:
                    NaviContentBookmark( const String &rUrl, const String& rDesc,
                                            USHORT nDragType, const SwDocShell* );
                    NaviContentBookmark();

    const String&   GetURL() const { return aUrl; }
    const String&   GetDescription() const { return aDescr; }
    USHORT          GetDefaultDragType() const { return nDefDrag; }
    long            GetDocShell() const {return(nDocSh);}

    static BOOL     DragServerHasFormat( USHORT nItem, const SwDocShell* pDocSh );
    static ULONG    HasFormat( SotDataObject& rObj );


    BOOL            CopyDragServer() const;
    BOOL            PasteDragServer( USHORT nItem );
    BOOL            Paste( SotDataObject& rObj, ULONG nFormat );
};

#endif
