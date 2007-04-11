/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imapobj.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 19:22:45 $
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

#ifndef _GOODIES_IMAPOBJ_HXX
#define _GOODIES_IMAPOBJ_HXX

#ifndef INCLUDED_SVTDLLAPI_H
#include "svtools/svtdllapi.h"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SFXMACITEM_HXX //autogen
#include <svtools/macitem.hxx>
#endif

class Point;
class Rectangle;
class SvStream;

#define IMAP_OBJ_NONE       ((UINT16)0x0000)
#define IMAP_OBJ_RECTANGLE  ((UINT16)0x0001)
#define IMAP_OBJ_CIRCLE     ((UINT16)0x0002)
#define IMAP_OBJ_POLYGON    ((UINT16)0x0003)
#define IMAP_OBJ_VERSION    ((UINT16)0x0005)

#define IMAGE_MAP_VERSION   ((UINT16)0x0001)

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
    String              aDescription;
    String              aTarget;
    String              aName;
    SvxMacroTableDtor   aEventList;
    BOOL                bActive;

protected:

    UINT16              nReadVersion;

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

                        IMapObject() {};
                        IMapObject( const String& rURL,
                                    const String& rDescription,
                                    BOOL bActive );
                        IMapObject( const String& rURL,
                                    const String& rDescription,
                                    const String& rTarget,
                                    BOOL bActive );
                        IMapObject( const String& rURL,
                                    const String& rDescription,
                                    const String& rTarget,
                                    const String& rName,
                                    BOOL bActive );
    virtual             ~IMapObject() {};

    virtual UINT16      GetVersion() const;
    virtual UINT16      GetType() const = 0;
    virtual BOOL        IsHit( const Point& rPoint ) const = 0;

    void                Write ( SvStream& rOStm, const String& rBaseURL ) const;
    void                Read( SvStream& rIStm, const String& rBaseURL );

    virtual Rectangle   GetBoundRect() const = 0;

    const String&       GetURL() const { return aURL; }
    void                SetURL( const String& rURL ) { aURL = rURL; }

    const String&       GetDescription() const { return aDescription; }
    void                SetDescription( const String& rDescription ) { aDescription = rDescription; }

    const String&       GetTarget() const { return aTarget; }
    void                SetTarget( const String& rTarget ) { aTarget = rTarget; }

    const String&       GetName() const { return aName; }
    void                SetName( const String& rName ) { aName = rName; }

    BOOL                IsActive() const { return bActive; }
    void                SetActive( BOOL bSetActive = TRUE ) { bActive = bSetActive; }

    static Point        GetPixelPoint( const Point& rLogPoint );
    static Point        GetLogPoint( const Point& rPixelPoint );

    BOOL                IsEqual( const IMapObject& rEqObj );

    // IMap-Events
    inline const SvxMacroTableDtor& GetMacroTable() const { return aEventList;}
    inline void SetMacroTable( const SvxMacroTableDtor& rTbl ) { aEventList = rTbl; }

    inline const SvxMacro&  GetEvent( USHORT nEvent ) const;
    inline BOOL             HasEvent( USHORT nEvent ) const;
           void             SetEvent( USHORT nEvent, const SvxMacro& );
    inline BOOL             DelEvent( USHORT nEvent );
};

inline BOOL IMapObject::HasEvent( USHORT nEvent ) const
{
    return aEventList.IsKeyValid( nEvent );
}
inline const SvxMacro& IMapObject::GetEvent( USHORT nEvent ) const
{
    return *(aEventList.Get( nEvent ));
}
inline BOOL IMapObject::DelEvent( USHORT nEvent )
{
    SvxMacro *pMacro = aEventList.Remove( nEvent );
    delete pMacro;
    return ( pMacro != 0 );
}

#endif

