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



#ifndef _GOODIES_IMAPOBJ_HXX
#define _GOODIES_IMAPOBJ_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <svl/macitem.hxx>

class Point;
class Rectangle;
class SvStream;

#define IMAP_OBJ_NONE       ((sal_uInt16)0x0000)
#define IMAP_OBJ_RECTANGLE  ((sal_uInt16)0x0001)
#define IMAP_OBJ_CIRCLE     ((sal_uInt16)0x0002)
#define IMAP_OBJ_POLYGON    ((sal_uInt16)0x0003)
#define IMAP_OBJ_VERSION    ((sal_uInt16)0x0005)

#define IMAGE_MAP_VERSION   ((sal_uInt16)0x0001)

#define IMAPMAGIC           "SDIMAP"

#define IMAP_MIRROR_HORZ    0x00000001L
#define IMAP_MIRROR_VERT    0x00000002L

#define IMAP_FORMAT_BIN     0x00000001L
#define IMAP_FORMAT_CERN    0x00000002L
#define IMAP_FORMAT_NCSA    0x00000004L
#define IMAP_FORMAT_DETECT  0xffffffffL

#define IMAP_ERR_OK         0x00000000L
#define IMAP_ERR_FORMAT     0x00000001L

/******************************************************************************
|*
|*
|*
\******************************************************************************/

class SVT_DLLPUBLIC IMapObject
{
    friend class        ImageMap;

    String              aURL;
    String              aAltText;
    String              aDesc;
    String              aTarget;
    String              aName;
    SvxMacroTableDtor   aEventList;
    sal_Bool                bActive;

protected:

    sal_uInt16              nReadVersion;

    // Binaer-Im-/Export
    virtual void        WriteIMapObject( SvStream& rOStm ) const = 0;
    virtual void        ReadIMapObject(  SvStream& rIStm ) = 0;

    // Hilfsmethoden
    void                AppendCERNCoords( const Point& rPoint100, ByteString& rStr ) const;
    void                AppendCERNURL( ByteString& rStr, const String& rBaseURL ) const;
    void                AppendNCSACoords( const Point& rPoint100, ByteString& rStr ) const;
    void                AppendNCSAURL( ByteString& rStr, const String& rBaseURL ) const;

public:

    static rtl_TextEncoding nActualTextEncoding;

                        IMapObject();
                        IMapObject( const String& rURL,
                                    const String& rAltText,
                                    const String& rDesc,
                                    const String& rTarget,
                                    const String& rName,
                                    sal_Bool bActive );
    virtual             ~IMapObject() {};

    virtual sal_uInt16      GetVersion() const;
    virtual sal_uInt16      GetType() const = 0;
    virtual sal_Bool        IsHit( const Point& rPoint ) const = 0;

    void                Write ( SvStream& rOStm, const String& rBaseURL ) const;
    void                Read( SvStream& rIStm, const String& rBaseURL );

    virtual Rectangle   GetBoundRect() const = 0;

    const String&       GetURL() const { return aURL; }
    void                SetURL( const String& rURL ) { aURL = rURL; }

    const String&       GetAltText() const { return aAltText; }
    void                SetAltText( const String& rAltText) { aAltText = rAltText; }

    const String&       GetDesc() const { return aDesc; }
    void                SetDesc( const String& rDesc ) { aDesc = rDesc; }

    const String&       GetTarget() const { return aTarget; }
    void                SetTarget( const String& rTarget ) { aTarget = rTarget; }

    const String&       GetName() const { return aName; }
    void                SetName( const String& rName ) { aName = rName; }

    sal_Bool                IsActive() const { return bActive; }
    void                SetActive( sal_Bool bSetActive = sal_True ) { bActive = bSetActive; }

    static Point        GetPixelPoint( const Point& rLogPoint );
    static Point        GetLogPoint( const Point& rPixelPoint );

    sal_Bool                IsEqual( const IMapObject& rEqObj );

    // IMap-Events
    inline const SvxMacroTableDtor& GetMacroTable() const { return aEventList;}
    inline void SetMacroTable( const SvxMacroTableDtor& rTbl ) { aEventList = rTbl; }

    inline const SvxMacro&  GetEvent( sal_uInt16 nEvent ) const;
    inline sal_Bool             HasEvent( sal_uInt16 nEvent ) const;
           void             SetEvent( sal_uInt16 nEvent, const SvxMacro& );
    inline sal_Bool             DelEvent( sal_uInt16 nEvent );
};

inline sal_Bool IMapObject::HasEvent( sal_uInt16 nEvent ) const
{
    return aEventList.IsKeyValid( nEvent );
}
inline const SvxMacro& IMapObject::GetEvent( sal_uInt16 nEvent ) const
{
    return *(aEventList.Get( nEvent ));
}
inline sal_Bool IMapObject::DelEvent( sal_uInt16 nEvent )
{
    SvxMacro *pMacro = aEventList.Remove( nEvent );
    delete pMacro;
    return ( pMacro != 0 );
}

#endif

