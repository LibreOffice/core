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

#ifndef _IMAP_HXX
#define _IMAP_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
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
    String              aName;

protected:

    // Binaer laden/speichern
    void                ImpWriteImageMap( SvStream& rOStm, const String& ) const ;
    void                ImpReadImageMap( SvStream& rIStm, size_t nCount, const String& );

    // Im-/Export
    void                ImpWriteCERN( SvStream& rOStm, const String& rBaseURL ) const;
    void                ImpWriteNCSA( SvStream& rOStm, const String& rBaseURL ) const;
    sal_uLong           ImpReadCERN( SvStream& rOStm, const String& rBaseURL );
    sal_uLong           ImpReadNCSA( SvStream& rOStm, const String& rBaseURL );

    void                ImpReadCERNLine( const rtl::OString& rLine, const String& rBaseURL );
    Point               ImpReadCERNCoords( const char** ppStr );
    long                ImpReadCERNRadius( const char** ppStr );
    String              ImpReadCERNURL( const char** ppStr, const String& rBaseURL );

    void                ImpReadNCSALine( const rtl::OString& rLine, const String& rBaseURL );
    String              ImpReadNCSAURL( const char** ppStr, const String& rBaseURL );
    Point               ImpReadNCSACoords( const char** ppStr );

    sal_uLong           ImpDetectFormat( SvStream& rIStm );

public:

    TYPEINFO();

                        ImageMap() {};
                        ImageMap( const String& rName );
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
    const String&       GetName() const { return aName; }
    void                SetName( const String& rName ) { aName = rName; }

    // skaliert alle Objekte der ImageMap entpr. dem uebergebenen Faktor
    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    // Im-/Export
    void                Write ( SvStream& rOStm, const String& rBaseURL ) const;
    void                Read( SvStream& rIStm, const String& rBaseURL );

    void                Write( SvStream& rOStm, sal_uLong nFormat, const String& rBaseURL ) const;
    sal_uLong               Read( SvStream& rIStm, sal_uLong nFormat, const String& rBaseURL );
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
