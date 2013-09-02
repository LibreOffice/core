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

#ifndef _IMAP_HXX
#define _IMAP_HXX

#include "svtools/svtdllapi.h"
#include <tools/stream.hxx>
#include <vector>

class Point;
class Rectangle;
class Size;
class Fraction;
class IMapObject;

typedef ::std::vector< IMapObject* > IMapObjectList_impl;

class SVT_DLLPUBLIC ImageMap
{
private:

    IMapObjectList_impl maList;
    OUString            aName;

protected:

    // Binaer laden/speichern
    void                ImpWriteImageMap( SvStream& rOStm, const OUString& ) const ;
    void                ImpReadImageMap( SvStream& rIStm, size_t nCount, const OUString& );

    // Im-/Export
    void                ImpWriteCERN( SvStream& rOStm, const OUString& rBaseURL ) const;
    void                ImpWriteNCSA( SvStream& rOStm, const OUString& rBaseURL ) const;
    sal_uLong           ImpReadCERN( SvStream& rOStm, const OUString& rBaseURL );
    sal_uLong           ImpReadNCSA( SvStream& rOStm, const OUString& rBaseURL );

    void                ImpReadCERNLine( const OString& rLine, const OUString& rBaseURL );
    Point               ImpReadCERNCoords( const char** ppStr );
    long                ImpReadCERNRadius( const char** ppStr );
    OUString            ImpReadCERNURL( const char** ppStr, const OUString& rBaseURL );

    void                ImpReadNCSALine( const OString& rLine, const OUString& rBaseURL );
    OUString            ImpReadNCSAURL( const char** ppStr, const OUString& rBaseURL );
    Point               ImpReadNCSACoords( const char** ppStr );

    sal_uLong           ImpDetectFormat( SvStream& rIStm );

public:

    TYPEINFO();

                        ImageMap() {};
                        ImageMap( const OUString& rName );
                        ImageMap( const ImageMap& rImageMap );

                        // Der Dtor gibt den intern belegten
                        // Speicher wieder frei;
                        //  alle IMapObjekte werden im Dtor zerstoert;
    virtual             ~ImageMap();

    // Zuweisungsoperator
    ImageMap&           operator=( const ImageMap& rImageMap );

    // Vergleichsoperator (es wird alles auf Gleichheit geprueft)
    sal_Bool                operator==( const ImageMap& rImageMap );
    sal_Bool                operator!=( const ImageMap& rImageMap );

    // In die Map wird ein neues IMap-Obkekt ans Ende eingefuegt
    void                InsertIMapObject( const IMapObject& rIMapObject );

    // Zugriff auf einzelne IMapObjekte; die Objekte
    // duerfen von aussen _nicht_ zerstoert werden
    IMapObject*         GetIMapObject( size_t nPos ) const
                        {
                            return ( nPos < maList.size() ) ? maList[ nPos ] : NULL;
                        }

    // Gibt das Objekt zurueck, das zuerst getroffen wurde oder NULL;
    // Groessen- und Positionsangaben sind in 1/100mm;
    // rTotalSize ist die Originalgroesse des Bildes;
    // rDisplaySize die aktuelle Darstellungsgroesse;
    // rRelPoint bezieht sich auf die Darstellungsgroesse
    // und die linke oebere Ecke des Bildes
    IMapObject*         GetHitIMapObject( const Size& rOriginalSize,
                                          const Size& rDisplaySize,
                                          const Point& rRelHitPoint,
                                          sal_uLong nFlags = 0 );

    // Gibt die Gesamtanzahl der IMap-Objekte zurueck
    size_t              GetIMapObjectCount() const { return maList.size(); }

    // Loescht alle internen Objekte
    void                ClearImageMap();

    // liefert die aktuelle Versionsnummer
    sal_uInt16          GetVersion() const;

    // liefert / setzt den Namen der ImageMap
    const OUString&     GetName() const { return aName; }
    void                SetName( const OUString& rName ) { aName = rName; }

    // skaliert alle Objekte der ImageMap entpr. dem uebergebenen Faktor
    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    // Im-/Export
    void                Write ( SvStream& rOStm, const OUString& rBaseURL ) const;
    void                Read( SvStream& rIStm, const OUString& rBaseURL );

    void                Write( SvStream& rOStm, sal_uLong nFormat, const OUString& rBaseURL ) const;
    sal_uLong               Read( SvStream& rIStm, sal_uLong nFormat, const OUString& rBaseURL );
};

class IMapCompat
{
    SvStream*       pRWStm;
    sal_uLong           nCompatPos;
    sal_uLong           nTotalSize;
    sal_uInt16          nStmMode;

                    IMapCompat() {}
                    IMapCompat( const IMapCompat& ) {}
    IMapCompat&     operator=( const IMapCompat& ) { return *this; }
    sal_Bool            operator==( const IMapCompat& ) { return sal_False; }

public:

                    IMapCompat( SvStream& rStm, const sal_uInt16 nStreamMode );
                    ~IMapCompat();
};

#endif // _IMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
