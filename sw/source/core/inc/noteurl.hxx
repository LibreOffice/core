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

#ifndef _NOTEURL_HXX
#define _NOTEURL_HXX


#include <tools/string.hxx>
#include "swrect.hxx"
#include <boost/ptr_container/ptr_vector.hpp>

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
    sal_Bool operator==( const SwURLNote& rSwURLNote ) const
    { return aRect == rSwURLNote.aRect; }
};

typedef boost::ptr_vector<SwURLNote> SwURLNoteList;

class SwNoteURL
{
    SwURLNoteList aList;
public:
    SwNoteURL() {}
    sal_uInt16 Count() const { return aList.size(); }
    void InsertURLNote( const String& rURL, const String& rTarget,
                 const SwRect& rRect );
    const SwURLNote& GetURLNote( sal_uInt16 nPos ) const
        { return aList[ nPos ]; }
    void FillImageMap( ImageMap* pMap, const Point& rPos, const MapMode& rMap );
};

// globale Variable, in NoteURL.Cxx angelegt
extern SwNoteURL *pNoteURL;


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
