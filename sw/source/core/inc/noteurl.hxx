/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _NOTEURL_HXX
#define _NOTEURL_HXX


#include <tools/string.hxx>
#include "swrect.hxx"
#include <boost/ptr_container/ptr_vector.hpp>

class ImageMap;
class MapMode;

class SwURLNote
{
    OUString aURL;
    OUString aTarget;
    SwRect aRect;
public:
    SwURLNote( const OUString& rURL, const OUString& rTarget, const SwRect& rRect )
    : aURL( rURL ), aTarget( rTarget ), aRect( rRect )
    {}
    const OUString& GetURL()      const { return aURL; }
    const OUString& GetTarget()   const { return aTarget; }
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
    void InsertURLNote( const OUString& rURL, const OUString& rTarget,
                 const SwRect& rRect );
    const SwURLNote& GetURLNote( sal_uInt16 nPos ) const
        { return aList[ nPos ]; }
    void FillImageMap( ImageMap* pMap, const Point& rPos, const MapMode& rMap );
};

// globale Variable, in NoteURL.Cxx angelegt
extern SwNoteURL *pNoteURL;


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
