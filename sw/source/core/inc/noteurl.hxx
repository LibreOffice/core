/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: noteurl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:51:38 $
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

#ifndef _NOTEURL_HXX
#define _NOTEURL_HXX


#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SWRECT_HXX
#include "swrect.hxx"
#endif

class ImageMap;
class MapMode;

class SwURLNote
{
    String aURL;
    String aTarget;
    SwRect aRect;
public:
    SwURLNote( const String& rURL, const String& rTarget, const SwRect& rRect )
    : aURL( rURL ), aTarget( rTarget ), aRect( rRect )
    {}
    const String& GetURL()      const { return aURL; }
    const String& GetTarget()   const { return aTarget; }
    const SwRect& GetRect()     const { return aRect; }
    BOOL operator==( const SwURLNote& rSwURLNote ) const
    { return aRect == rSwURLNote.aRect; }
};

typedef SwURLNote* SwURLNotePtr;
SV_DECL_PTRARR_DEL(SwURLNoteList, SwURLNotePtr, 0, 5)

class SwNoteURL
{
    SwURLNoteList aList;
public:
    SwNoteURL() {}
    USHORT Count() const { return aList.Count(); }
    void InsertURLNote( const String& rURL, const String& rTarget,
                 const SwRect& rRect );
    const SwURLNote& GetURLNote( USHORT nPos ) const
        { return *aList.GetObject( nPos ); }
    void FillImageMap( ImageMap* pMap, const Point& rPos, const MapMode& rMap );
};

// globale Variable, in NoteURL.Cxx angelegt
extern SwNoteURL *pNoteURL;


#endif

