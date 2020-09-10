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

#ifndef INCLUDED_VCL_IMAP_HXX
#define INCLUDED_VCL_IMAP_HXX

#include <vcl/imapobj.hxx>
#include <vcl/dllapi.h>
#include <tools/stream.hxx>
#include <tools/solar.h>
#include <memory>
#include <vector>

class Point;
class Size;
class Fraction;
class IMapObject;

class VCL_DLLPUBLIC ImageMap final
{
private:

    std::vector<std::unique_ptr<IMapObject>> maList;
    OUString                     aName;

    // binary saving/loading
    void                ImpWriteImageMap( SvStream& rOStm ) const ;
    void                ImpReadImageMap( SvStream& rIStm, size_t nCount );

    // Import/Export
    void                ImpWriteCERN( SvStream& rOStm ) const;
    void                ImpWriteNCSA( SvStream& rOStm ) const;
    void                ImpReadCERN( SvStream& rOStm );
    void                ImpReadNCSA( SvStream& rOStm );

    void                ImpReadCERNLine( const OString& rLine );
    static Point        ImpReadCERNCoords( const char** ppStr );
    static long         ImpReadCERNRadius( const char** ppStr );
    static OUString     ImpReadCERNURL( const char** ppStr );

    void                ImpReadNCSALine( const OString& rLine );
    static OUString     ImpReadNCSAURL( const char** ppStr );
    static Point        ImpReadNCSACoords( const char** ppStr );

    static IMapFormat   ImpDetectFormat( SvStream& rIStm );

public:

                        ImageMap() {};
                        ImageMap( const OUString& rName );
                        ImageMap( const ImageMap& rImageMap );

                        // all IMapObjects are destroyed in the destructor
                        ~ImageMap();

    ImageMap&           operator=( const ImageMap& rImageMap );

    // comparison (everything is checked for equality)
    bool                operator==( const ImageMap& rImageMap );
    bool                operator!=( const ImageMap& rImageMap );

    // a new IMap object is inserted at the end of the Map
    void                InsertIMapObject( const IMapObject& rIMapObject );
    void                InsertIMapObject( std::unique_ptr<IMapObject> rIMapObject );

    // access to the single ImapObjects; the objects may
    // not be destroyed from outside
    IMapObject*         GetIMapObject( size_t nPos ) const
                        {
                            return ( nPos < maList.size() ) ? maList[ nPos ].get() : nullptr;
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

    const OUString&     GetName() const { return aName; }
    void                SetName( const OUString& rName ) { aName = rName; }

    // scales all objects of the ImageMap according to the given factor
    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    // Import/Export
    void                Write ( SvStream& rOStm ) const;
    void                Read( SvStream& rIStm );

    void                Write( SvStream& rOStm, IMapFormat nFormat ) const;
    sal_uLong           Read( SvStream& rIStm, IMapFormat nFormat );
};

class IMapCompat
{
    SvStream*       pRWStm;
    sal_uInt64      nCompatPos;
    sal_uInt64      nTotalSize;
    StreamMode      nStmMode;

                    IMapCompat( const IMapCompat& ) = delete;
    IMapCompat&     operator=( const IMapCompat& ) { return *this; }

public:

                    IMapCompat( SvStream& rStm, const StreamMode nStreamMode );
                    ~IMapCompat();
};

#endif // INCLUDED_VCL_IMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
