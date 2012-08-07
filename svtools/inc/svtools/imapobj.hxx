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

#ifndef _GOODIES_IMAPOBJ_HXX
#define _GOODIES_IMAPOBJ_HXX

#include "svtools/svtdllapi.h"
#include <tools/string.hxx>
#include <svl/macitem.hxx>
#include <rtl/strbuf.hxx>

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
    void AppendCERNCoords(rtl::OStringBuffer& rBuf, const Point& rPoint100) const;
    void AppendCERNURL(rtl::OStringBuffer& rBuf, const String& rBaseURL) const;
    void AppendNCSACoords(rtl::OStringBuffer& rBuf, const Point& rPoint100) const;
    void AppendNCSAURL(rtl::OStringBuffer&rBuf, const String& rBaseURL) const;

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
    return aEventList.Erase( nEvent );
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
