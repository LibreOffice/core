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
#ifndef _SVX_HLNKITEM_HXX
#define _SVX_HLNKITEM_HXX

#include <tools/string.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include "svx/svxdllapi.h"

#define HYPERDLG_EVENT_MOUSEOVER_OBJECT     0x0001
#define HYPERDLG_EVENT_MOUSECLICK_OBJECT    0x0002
#define HYPERDLG_EVENT_MOUSEOUT_OBJECT      0x0004

enum SvxLinkInsertMode
{
    HLINK_DEFAULT,
    HLINK_FIELD,
    HLINK_BUTTON,
    HLINK_HTMLMODE = 0x0080
};

class SVX_DLLPUBLIC SvxHyperlinkItem : public SfxPoolItem
{
    String sName;
    String sURL;
    String sTarget;
    SvxLinkInsertMode eType;

    String sIntName;
    SvxMacroTableDtor*  pMacroTable;

    sal_uInt16 nMacroEvents;

public:
    TYPEINFO();

    SvxHyperlinkItem( sal_uInt16 _nWhich = SID_HYPERLINK_GETLINK ):
                SfxPoolItem(_nWhich), pMacroTable(NULL) { eType = HLINK_DEFAULT; nMacroEvents=0; };
    SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem );
    SvxHyperlinkItem( sal_uInt16 nWhich, String& rName, String& rURL,
                                    String& rTarget, String& rIntName,
                                    SvxLinkInsertMode eTyp = HLINK_FIELD,
                                    sal_uInt16 nEvents = 0,
                                    SvxMacroTableDtor *pMacroTbl =NULL );
    virtual ~SvxHyperlinkItem () { delete pMacroTable; }

    inline SvxHyperlinkItem& operator=( const SvxHyperlinkItem &rItem );

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const   String& GetName() const { return sName; }
    void    SetName(const String& rName) { sName = rName; }

    const   String& GetURL() const { return sURL; }
    void    SetURL(const String& rURL) { sURL = rURL; }

    const   String& GetIntName () const { return sIntName; }
    void    SetIntName(const String& rIntName) { sIntName = rIntName; }

    const   String& GetTargetFrame() const { return sTarget; }
    void    SetTargetFrame(const String& rTarget) { sTarget = rTarget; }

    SvxLinkInsertMode GetInsertMode() const { return eType; }
    void    SetInsertMode( SvxLinkInsertMode eNew ) { eType = eNew; }

    void SetMacro( sal_uInt16 nEvent, const SvxMacro& rMacro );

    void SetMacroTable( const SvxMacroTableDtor& rTbl );
    const SvxMacroTableDtor* GetMacroTbl() const    { return pMacroTable; }

    void SetMacroEvents (const sal_uInt16 nEvents) { nMacroEvents = nEvents; }
    sal_uInt16 GetMacroEvents() const { return nMacroEvents; }

    virtual SvStream&           Store( SvStream &, sal_uInt16 nItemVersion ) const;
    virtual SfxPoolItem*        Create( SvStream &, sal_uInt16 nVer ) const;

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
