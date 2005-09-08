/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urlbmk.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:13:54 $
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

#ifndef _URLBMK_HXX
#define _URLBMK_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

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
};


#endif

