/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _IMAP_HXX
#define _IMAP_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <tools/stream.hxx>

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
    void                ImpReadImageMap( SvStream& rIStm, sal_uInt16 nCount, const String& );

    // Im-/Export
    void                ImpWriteCERN( SvStream& rOStm, const String& rBaseURL ) const;
    void                ImpWriteNCSA( SvStream& rOStm, const String& rBaseURL ) const;
    sal_uLong               ImpReadCERN( SvStream& rOStm, const String& rBaseURL );
    sal_uLong               ImpReadNCSA( SvStream& rOStm, const String& rBaseURL );

    void                ImpReadCERNLine( const ByteString& rLine, const String& rBaseURL );
    Point               ImpReadCERNCoords( const char** ppStr );
    long                ImpReadCERNRadius( const char** ppStr );
    String              ImpReadCERNURL( const char** ppStr, const String& rBaseURL );

    void                ImpReadNCSALine( const ByteString& rLine, const String& rBaseURL );
    String              ImpReadNCSAURL( const char** ppStr, const String& rBaseURL );
    Point               ImpReadNCSACoords( const char** ppStr );

    sal_uLong               ImpDetectFormat( SvStream& rIStm );

public:

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
    IMapObject*         GetFirstIMapObject() { return (IMapObject*) maList.First(); }
    IMapObject*         GetNextIMapObject() { return (IMapObject*) maList.Next(); }
    IMapObject*         GetLastIMapObject() { return (IMapObject*) maList.Last(); }
    IMapObject*         GetPrevIMapObject() { return (IMapObject*) maList.Prev(); }
    IMapObject*         GetIMapObject( sal_uInt16 nPos ) const { return (IMapObject*) maList.GetObject( nPos ); }

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
    sal_uInt16              GetIMapObjectCount() const { return (sal_uInt16) maList.Count(); }

    // Loescht alle internen Objekte
    void                ClearImageMap();

    // liefert die aktuelle Versionsnummer
    sal_uInt16              GetVersion() const;

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

    void                Write( SvStream& rOStm, sal_uLong nFormat, const String& rBaseURL ) const;
    sal_uLong               Read( SvStream& rIStm, sal_uLong nFormat, const String& rBaseURL );
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

//#endif // __PRIVATE
#endif // _IMAP_HXX
