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

#ifndef INCLUDED_SVTOOLS_IMAP_HXX
#define INCLUDED_SVTOOLS_IMAP_HXX

#include <svtools/imapobj.hxx>
#include <svtools/svtdllapi.h>
#include <tools/stream.hxx>
#include <vector>

class Point;
class Rectangle;
class Size;
class Fraction;
class IMapObject;

class SVT_DLLPUBLIC ImageMap
{
private:

    ::std::vector< IMapObject* > maList;
    OUString                     aName;

protected:

    // binary saving/loading
    void                ImpWriteImageMap( SvStream& rOStm, const OUString& ) const ;
    void                ImpReadImageMap( SvStream& rIStm, size_t nCount, const OUString& );

    // Import/Export
    void                ImpWriteCERN( SvStream& rOStm, const OUString& rBaseURL ) const;
    void                ImpWriteNCSA( SvStream& rOStm, const OUString& rBaseURL ) const;
    sal_uLong           ImpReadCERN( SvStream& rOStm, const OUString& rBaseURL );
    sal_uLong           ImpReadNCSA( SvStream& rOStm, const OUString& rBaseURL );

    void                ImpReadCERNLine( const OString& rLine, const OUString& rBaseURL );
    static Point        ImpReadCERNCoords( const char** ppStr );
    static long         ImpReadCERNRadius( const char** ppStr );
    static OUString     ImpReadCERNURL( const char** ppStr, const OUString& rBaseURL );

    void                ImpReadNCSALine( const OString& rLine, const OUString& rBaseURL );
    static OUString     ImpReadNCSAURL( const char** ppStr, const OUString& rBaseURL );
    static Point        ImpReadNCSACoords( const char** ppStr );

    static sal_uLong    ImpDetectFormat( SvStream& rIStm );

public:

                        ImageMap() {};
                        ImageMap( const OUString& rName );
                        ImageMap( const ImageMap& rImageMap );

                        // all IMapObjects are destroyed in the destructor
    virtual             ~ImageMap();

    ImageMap&           operator=( const ImageMap& rImageMap );

    // comparison (everything is checked for equality)
    bool                operator==( const ImageMap& rImageMap );
    bool                operator!=( const ImageMap& rImageMap );

    // a new IMap object is inserted at the end of the Map
    void                InsertIMapObject( const IMapObject& rIMapObject );

    // access to the single ImapObjects; the objects may
    // not be destroyed from outside
    IMapObject*         GetIMapObject( size_t nPos ) const
                        {
                            return ( nPos < maList.size() ) ? maList[ nPos ] : NULL;
                        }

    // returns the object which was hit first or NULL;
    // size and position values are in 1/100mm;
    // rTotalSize is the original size of the image
    // rDisplaySize is the current size;
    // rRelPoint relates to the display size and the upper left
    // corner of the image
    IMapObject*         GetHitIMapObject( const Size& rOriginalSize,
                                          const Size& rDisplaySize,
                                          const Point& rRelHitPoint,
                                          sal_uLong nFlags = 0 );

    // returns the total amount of IMap objects
    size_t              GetIMapObjectCount() const { return maList.size(); }

    // deletes all internal objects
    void                ClearImageMap();

    // returns the current version number
    static sal_uInt16   GetVersion() { return IMAGE_MAP_VERSION; }

    const OUString&     GetName() const { return aName; }
    void                SetName( const OUString& rName ) { aName = rName; }

    // scales all objects of the ImageMap according to the given factor
    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    // Import/Export
    void                Write ( SvStream& rOStm, const OUString& rBaseURL ) const;
    void                Read( SvStream& rIStm, const OUString& rBaseURL );

    void                Write( SvStream& rOStm, sal_uLong nFormat, const OUString& rBaseURL ) const;
    sal_uLong               Read( SvStream& rIStm, sal_uLong nFormat, const OUString& rBaseURL );
};

class IMapCompat
{
    SvStream*       pRWStm;
    sal_uLong       nCompatPos;
    sal_uLong       nTotalSize;
    StreamMode      nStmMode;

                    IMapCompat() {}
                    IMapCompat( const IMapCompat& ) {}
    IMapCompat&     operator=( const IMapCompat& ) { return *this; }
    bool            operator==( const IMapCompat& ) { return false; }

public:

                    IMapCompat( SvStream& rStm, const StreamMode nStreamMode );
                    ~IMapCompat();
};

#endif // INCLUDED_SVTOOLS_IMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
