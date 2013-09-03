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
    OUString sName;
    OUString sURL;
    OUString sTarget;
    SvxLinkInsertMode eType;

    OUString sIntName;
    SvxMacroTableDtor*  pMacroTable;

    sal_uInt16 nMacroEvents;

public:
    TYPEINFO();

    SvxHyperlinkItem( sal_uInt16 _nWhich = SID_HYPERLINK_GETLINK ):
                SfxPoolItem(_nWhich), pMacroTable(NULL) { eType = HLINK_DEFAULT; nMacroEvents=0; };
    SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem );
    SvxHyperlinkItem( sal_uInt16 nWhich, const OUString& rName, const OUString& rURL,
                                    const OUString& rTarget, const OUString& rIntName,
                                    SvxLinkInsertMode eTyp = HLINK_FIELD,
                                    sal_uInt16 nEvents = 0,
                                    SvxMacroTableDtor *pMacroTbl =NULL );
    virtual ~SvxHyperlinkItem () { delete pMacroTable; }

    inline SvxHyperlinkItem& operator=( const SvxHyperlinkItem &rItem );

    virtual int              operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = 0 ) const;
    virtual bool             QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool             PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    const   OUString& GetName() const { return sName; }
    void    SetName(const OUString& rName) { sName = rName; }

    const   OUString& GetURL() const { return sURL; }
    void    SetURL(const OUString& rURL) { sURL = rURL; }

    const   OUString& GetIntName () const { return sIntName; }
    void    SetIntName(const OUString& rIntName) { sIntName = rIntName; }

    const   OUString& GetTargetFrame() const { return sTarget; }
    void    SetTargetFrame(const OUString& rTarget) { sTarget = rTarget; }

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
