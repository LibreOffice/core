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

#include "bf_svtools/svtdllapi.h"

#include <tools/string.hxx>
#include <tools/stream.hxx>

class Point;
class Rectangle;
class Size;
class Fraction;

namespace binfilter
{

class IMapObject;

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class  ImageMap
{
    List				maList;
    String				aName;

protected:

    // Binaer laden/speichern
    void                ImpWriteImageMap( SvStream& rOStm, const String& ) const ;
    void                ImpReadImageMap( SvStream& rIStm, USHORT nCount, const String& );

public:

    TYPEINFO();

                        ImageMap() {};
                        ImageMap( const ImageMap& rImageMap );

                        // Der Dtor gibt den intern belegten
                        // Speicher wieder frei;
                        //  alle IMapObjekte werden im Dtor zerstoert;
    virtual				~ImageMap();

    // Zuweisungsoperator
    ImageMap&			operator=( const ImageMap& rImageMap );

    // Vergleichsoperator (es wird alles auf Gleichheit geprueft)
    BOOL				operator==( const ImageMap& rImageMap );
    BOOL				operator!=( const ImageMap& rImageMap );

    // In die Map wird ein neues IMap-Obkekt ans Ende eingefuegt
    void				InsertIMapObject( const IMapObject& rIMapObject );

    // Zugriff auf einzelne IMapObjekte; die Objekte
    // duerfen von aussen _nicht_ zerstoert werden
    IMapObject*			GetFirstIMapObject() { return (IMapObject*) maList.First(); }
    IMapObject*			GetNextIMapObject() { return (IMapObject*) maList.Next(); }
    IMapObject*			GetLastIMapObject() { return (IMapObject*) maList.Last(); }
    IMapObject*			GetPrevIMapObject() { return (IMapObject*) maList.Prev(); }
    IMapObject*			GetIMapObject( USHORT nPos ) const { return (IMapObject*) maList.GetObject( nPos ); }

    // Gibt die Gesamtanzahl der IMap-Objekte zurueck
    UINT16				GetIMapObjectCount() const { return (UINT16) maList.Count(); }

    // Loescht alle internen Objekte
    void				ClearImageMap();

    // liefert die aktuelle Versionsnummer
    UINT16				GetVersion() const;

    // liefert / setzt den Namen der ImageMap
    const String&		GetName() const { return aName; }
    void				SetName( const String& rName ) { aName = rName; }

    // skaliert alle Objekte der ImageMap entpr. dem uebergebenen Faktor
    void				Scale( const Fraction& rFractX, const Fraction& rFracY );

    // Im-/Export
    void                Write ( SvStream& rOStm, const String& rBaseURL ) const;
    void                Read( SvStream& rIStm, const String& rBaseURL );
};

class IMapCompat
{
    SvStream*		pRWStm;
    ULONG			nCompatPos;
    ULONG			nTotalSize;
    USHORT			nStmMode;

                    IMapCompat() {}
                    IMapCompat( const IMapCompat& ) {}
    IMapCompat&		operator=( const IMapCompat& ) { return *this; }
    BOOL			operator==( const IMapCompat& ) { return FALSE; }

public:

                    IMapCompat( SvStream& rStm, const USHORT nStreamMode );
                    ~IMapCompat();
};

}

#endif // _IMAP_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
