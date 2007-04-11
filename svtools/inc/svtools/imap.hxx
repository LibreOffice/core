/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:22:10 $
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

#ifndef _IMAP_HXX
#define _IMAP_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

class Point;
class Rectangle;
class Size;
class Fraction;
class IMapObject;

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class SVT_DLLPUBLIC ImageMap
{
    List                maList;
    String              aName;

protected:

    // Binaer laden/speichern
    void                ImpWriteImageMap( SvStream& rOStm, const String& ) const ;
    void                ImpReadImageMap( SvStream& rIStm, USHORT nCount, const String& );

    // Im-/Export
    void                ImpWriteCERN( SvStream& rOStm, const String& rBaseURL ) const;
    void                ImpWriteNCSA( SvStream& rOStm, const String& rBaseURL ) const;
    ULONG               ImpReadCERN( SvStream& rOStm, const String& rBaseURL );
    ULONG               ImpReadNCSA( SvStream& rOStm, const String& rBaseURL );

    void                ImpReadCERNLine( const ByteString& rLine, const String& rBaseURL );
    Point               ImpReadCERNCoords( const char** ppStr );
    long                ImpReadCERNRadius( const char** ppStr );
    String              ImpReadCERNURL( const char** ppStr, const String& rBaseURL );

    void                ImpReadNCSALine( const ByteString& rLine, const String& rBaseURL );
    String              ImpReadNCSAURL( const char** ppStr, const String& rBaseURL );
    Point               ImpReadNCSACoords( const char** ppStr );

    ULONG               ImpDetectFormat( SvStream& rIStm );

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
    BOOL                operator==( const ImageMap& rImageMap );
    BOOL                operator!=( const ImageMap& rImageMap );

    // In die Map wird ein neues IMap-Obkekt ans Ende eingefuegt
    void                InsertIMapObject( const IMapObject& rIMapObject );

    // Zugriff auf einzelne IMapObjekte; die Objekte
    // duerfen von aussen _nicht_ zerstoert werden
    IMapObject*         GetFirstIMapObject() { return (IMapObject*) maList.First(); }
    IMapObject*         GetNextIMapObject() { return (IMapObject*) maList.Next(); }
    IMapObject*         GetLastIMapObject() { return (IMapObject*) maList.Last(); }
    IMapObject*         GetPrevIMapObject() { return (IMapObject*) maList.Prev(); }
    IMapObject*         GetIMapObject( USHORT nPos ) const { return (IMapObject*) maList.GetObject( nPos ); }

    // Gibt das Objekt zurueck, das zuerst getroffen wurde oder NULL;
    // Groessen- und Positionsangaben sind in 1/100mm;
    // rTotalSize ist die Originalgroesse des Bildes;
    // rDisplaySize die aktuelle Darstellungsgroesse;
    // rRelPoint bezieht sich auf die Darstellungsgroesse
    // und die linke oebere Ecke des Bildes
    IMapObject*         GetHitIMapObject( const Size& rOriginalSize,
                                          const Size& rDisplaySize,
                                          const Point& rRelHitPoint,
                                          ULONG nFlags = 0 );

    // Gibt die Gesamtanzahl der IMap-Objekte zurueck
    UINT16              GetIMapObjectCount() const { return (UINT16) maList.Count(); }

    // Loescht alle internen Objekte
    void                ClearImageMap();

    // liefert die aktuelle Versionsnummer
    UINT16              GetVersion() const;

    // liefert / setzt den Namen der ImageMap
    const String&       GetName() const { return aName; }
    void                SetName( const String& rName ) { aName = rName; }

    // gibt das BoundRect aller IMap-Objekte in 1/100mm zurueck
    Rectangle           GetBoundRect() const;

    // skaliert alle Objekte der ImageMap entpr. dem uebergebenen Faktor
    void                Scale( const Fraction& rFractX, const Fraction& rFracY );

    // Im-/Export
    void                Write ( SvStream& rOStm, const String& rBaseURL ) const;
    void                Read( SvStream& rIStm, const String& rBaseURL );

    void                Write( SvStream& rOStm, ULONG nFormat, const String& rBaseURL ) const;
    ULONG               Read( SvStream& rIStm, ULONG nFormat, const String& rBaseURL );
};

/******************************************************************************
|*
|*
|*
\******************************************************************************/

//#if 0 // _SOLAR__PRIVATE

class IMapCompat
{
    SvStream*       pRWStm;
    ULONG           nCompatPos;
    ULONG           nTotalSize;
    USHORT          nStmMode;

                    IMapCompat() {}
                    IMapCompat( const IMapCompat& ) {}
    IMapCompat&     operator=( const IMapCompat& ) { return *this; }
    BOOL            operator==( const IMapCompat& ) { return FALSE; }

public:

                    IMapCompat( SvStream& rStm, const USHORT nStreamMode );
                    ~IMapCompat();
};

//#endif // __PRIVATE
#endif // _IMAP_HXX
