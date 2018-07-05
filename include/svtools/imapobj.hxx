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

#ifndef INCLUDED_SVTOOLS_IMAPOBJ_HXX
#define INCLUDED_SVTOOLS_IMAPOBJ_HXX

#include <svtools/svtdllapi.h>
#include <svl/macitem.hxx>
#include <rtl/strbuf.hxx>

class Point;
namespace tools { class Rectangle; }
class SvStream;

#define IMAP_OBJ_RECTANGLE  (sal_uInt16(0x0001))
#define IMAP_OBJ_CIRCLE     (sal_uInt16(0x0002))
#define IMAP_OBJ_POLYGON    (sal_uInt16(0x0003))
#define IMAP_OBJ_VERSION    (sal_uInt16(0x0005))

#define IMAGE_MAP_VERSION   (sal_uInt16(0x0001))

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

    OUString            aURL;
    OUString            aAltText;
    OUString            aDesc;
    OUString            aTarget;
    OUString            aName;
    SvxMacroTableDtor   aEventList;
    bool                bActive;

protected:
    sal_uInt16              nReadVersion;

    // binary import/export
    virtual void        WriteIMapObject( SvStream& rOStm ) const = 0;
    virtual void        ReadIMapObject(  SvStream& rIStm ) = 0;

    // helper methods
    static void AppendCERNCoords(OStringBuffer& rBuf, const Point& rPoint100);
    void AppendCERNURL(OStringBuffer& rBuf) const;
    static void AppendNCSACoords(OStringBuffer& rBuf, const Point& rPoint100);
    void AppendNCSAURL(OStringBuffer&rBuf) const;

public:

    static rtl_TextEncoding nActualTextEncoding;

                        IMapObject();
                        IMapObject( const OUString& rURL,
                                    const OUString& rAltText,
                                    const OUString& rDesc,
                                    const OUString& rTarget,
                                    const OUString& rName,
                                    bool bActive );
    virtual             ~IMapObject() {};

    IMapObject(IMapObject const &) = default;
    IMapObject(IMapObject &&) = default;
    IMapObject & operator =(IMapObject const &) = default;
    IMapObject & operator =(IMapObject &&) = default;

    virtual sal_uInt16  GetType() const = 0;
    virtual bool        IsHit( const Point& rPoint ) const = 0;

    void                Write ( SvStream& rOStm ) const;
    void                Read( SvStream& rIStm );

    const OUString&     GetURL() const { return aURL; }
    void                SetURL( const OUString& rURL ) { aURL = rURL; }

    const OUString&     GetAltText() const { return aAltText; }
    void                SetAltText( const OUString& rAltText) { aAltText = rAltText; }

    const OUString&     GetDesc() const { return aDesc; }
    void                SetDesc( const OUString& rDesc ) { aDesc = rDesc; }

    const OUString&     GetTarget() const { return aTarget; }
    void                SetTarget( const OUString& rTarget ) { aTarget = rTarget; }

    const OUString&     GetName() const { return aName; }
    void                SetName( const OUString& rName ) { aName = rName; }

    bool                IsActive() const { return bActive; }
    void                SetActive( bool bSetActive ) { bActive = bSetActive; }

    bool                IsEqual( const IMapObject& rEqObj );

    // IMap-Events
    const SvxMacroTableDtor& GetMacroTable() const { return aEventList;}
    void SetMacroTable( const SvxMacroTableDtor& rTbl ) { aEventList = rTbl; }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
