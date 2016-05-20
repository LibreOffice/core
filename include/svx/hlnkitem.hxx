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
#ifndef INCLUDED_SVX_HLNKITEM_HXX
#define INCLUDED_SVX_HLNKITEM_HXX

#include <rtl/ustring.hxx>
#include <svl/poolitem.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include <svx/svxdllapi.h>
#include <o3tl/typed_flags_set.hxx>

enum class HyperDialogEvent {
    NONE                = 0x0000,
    MouseOverObject     = 0x0001,
    MouseClickObject    = 0x0002,
    MouseOutObject      = 0x0004,
};
namespace o3tl {
    template<> struct typed_flags<HyperDialogEvent> : is_typed_flags<HyperDialogEvent, 0x07> {};
}

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

    HyperDialogEvent nMacroEvents;

public:
    static SfxPoolItem* CreateDefault();

    SvxHyperlinkItem( sal_uInt16 _nWhich = SID_HYPERLINK_GETLINK ):
                SfxPoolItem(_nWhich), pMacroTable(nullptr) { eType = HLINK_DEFAULT; nMacroEvents=HyperDialogEvent::NONE; };
    SvxHyperlinkItem( const SvxHyperlinkItem& rHyperlinkItem );
    SvxHyperlinkItem( sal_uInt16 nWhich, const OUString& rName, const OUString& rURL,
                                    const OUString& rTarget, const OUString& rIntName,
                                    SvxLinkInsertMode eTyp = HLINK_FIELD,
                                    HyperDialogEvent nEvents = HyperDialogEvent::NONE,
                                    SvxMacroTableDtor *pMacroTbl =nullptr );
    virtual ~SvxHyperlinkItem () { delete pMacroTable; }

    inline SvxHyperlinkItem& operator=( const SvxHyperlinkItem &rItem );

    virtual bool             operator==( const SfxPoolItem& ) const override;
    virtual SfxPoolItem*     Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool             QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool             PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

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

    void SetMacro( HyperDialogEvent nEvent, const SvxMacro& rMacro );

    void SetMacroTable( const SvxMacroTableDtor& rTbl );
    const SvxMacroTableDtor* GetMacroTable() const    { return pMacroTable; }

    void SetMacroEvents (const HyperDialogEvent nEvents) { nMacroEvents = nEvents; }
    HyperDialogEvent GetMacroEvents() const { return nMacroEvents; }

    virtual SvStream&           Store( SvStream &, sal_uInt16 nItemVersion ) const override;
    virtual SfxPoolItem*        Create( SvStream &, sal_uInt16 nVer ) const override;

};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
