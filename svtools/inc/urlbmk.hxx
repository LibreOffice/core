/*************************************************************************
 *
 *  $RCSfile: urlbmk.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:54 $
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

#ifndef _URLBMK_HXX
#define _URLBMK_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class SvData;
class SotDataObject;

//=========================================================================

class INetBookmark

/*  [Beschreibung]

    Diese Klasse stellt ein Bookmark dar, welches aus einer URL und
    einem dazuge"horigen Beschreibungstext besteht.

    Es gibt ein eigenes Clipboardformat und Hilfsmethoden zum Kopieren
    und Einf"ugen in und aus Clipboard und DragServer.
*/

{
    String          aUrl;
    String          aDescr;

protected:
    String          CopyExchange() const;
    void            PasteExchange( String aString );

    void            SetURL( const String& rS )          { aUrl = rS; }
    void            SetDescription( const String& rS )  { aDescr = rS; }

public:
                    INetBookmark( const String &rUrl, const String &rDescr )
                        : aUrl( rUrl ), aDescr( rDescr )
                    {}
                    INetBookmark()
                    {}

    const String&   GetURL() const          { return aUrl; }
    const String&   GetDescription() const  { return aDescr; }

    static BOOL     ClipboardHasFormat();
    static BOOL     DragServerHasFormat( USHORT nItem );

    BOOL            CopyClipboard() const;
    BOOL            CopyDragServer() const;
    BOOL            PasteClipboard();
    BOOL            PasteDragServer( USHORT nItem );

    BOOL            SetData( SvData& rData ) const;
    static ULONG    HasFormat( const SotDataObject& rObj );
    BOOL            Copy( SotDataObject& rObj ) const;
    BOOL            Paste( SotDataObject& rObj, ULONG nFormat );

    BOOL            _SetData( SvData& rData ) const
                        {   return INetBookmark::SetData( rData ); }
    static ULONG    _HasFormat( const SotDataObject& rObj )
                        {   return INetBookmark::HasFormat( rObj ); }
    BOOL            _Copy( SotDataObject& rObj ) const
                        {   return INetBookmark::Copy( rObj ); }
    BOOL            _Paste( SotDataObject& rObj, ULONG nFormat )
                        {   return INetBookmark::Paste( rObj, nFormat ); }

};


#endif

