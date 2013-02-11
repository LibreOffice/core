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

#ifndef _EDITOBJ_HXX
#define _EDITOBJ_HXX

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <rsc/rscsfx.hxx>
#include <svl/itemset.hxx>
#include "svl/itempool.hxx"
#include <editeng/eeitem.hxx>
#include <editeng/editdata.hxx>
#include "editeng/editengdllapi.h"
#include "editeng/macros.hxx"

#include <com/sun/star/text/textfield/Type.hpp>

#include <vector>

DBG_NAMEEX( EE_EditTextObject )

class SfxItemPool;
class SfxStyleSheetPool;
class SvxFieldItem;

namespace editeng {

class FieldUpdater;
class FieldUpdaterImpl;

}

class EditTextObjectImpl;

class EDITENG_DLLPUBLIC EditTextObject : public SfxItemPoolUser
{
    friend class EditTextObjectImpl;
    friend class editeng::FieldUpdaterImpl;
    friend class ImpEditEngine;

    EditTextObjectImpl* mpImpl;

    EDITENG_DLLPRIVATE EditTextObject&      operator=( const EditTextObject& );

    EditTextObject(); // disabled

    EditTextObject( SfxItemPool* pPool );
    EditTextObject( const EditTextObject& r );

    void StoreData( SvStream& rStrm ) const;
    void CreateData( SvStream& rStrm );

public:
    virtual ~EditTextObject();

    sal_uInt16 GetUserType() const;    // For OutlinerMode, it can however not save in compatible format
    void SetUserType( sal_uInt16 n );

    bool IsVertical() const;
    void SetVertical( bool bVertical );

    sal_uInt16 GetScriptType() const;

    EditTextObject* Clone() const;

    bool Store( SvStream& rOStream ) const;

    static EditTextObject* Create(
        SvStream& rIStream, SfxItemPool* pGlobalTextObjectPool = NULL );

    size_t GetParagraphCount() const;

    String GetText(size_t nPara) const;

    void ClearPortionInfo();

    bool HasOnlineSpellErrors() const;

    void GetCharAttribs( sal_uInt16 nPara, std::vector<EECharAttrib>& rLst ) const;

    bool RemoveCharAttribs( sal_uInt16 nWhich = 0 );
    bool RemoveParaAttribs( sal_uInt16 nWhich = 0 );

    bool IsFieldObject() const;
    const SvxFieldItem* GetField() const;
    bool HasField( sal_Int32 nType = com::sun::star::text::textfield::Type::UNSPECIFIED ) const;

    const SfxItemSet& GetParaAttribs(size_t nPara) const;
    void SetParaAttribs(size_t nPara, const SfxItemSet& rAttribs);

    void GetStyleSheet(size_t nPara, String& rName, SfxStyleFamily& eFamily) const;
    void SetStyleSheet(size_t nPara, const String& rName, const SfxStyleFamily& eFamily);
    bool ChangeStyleSheets(
        const XubString& rOldName, SfxStyleFamily eOldFamily, const XubString& rNewName, SfxStyleFamily eNewFamily );
    void ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    editeng::FieldUpdater GetFieldUpdater();

    bool operator==( const EditTextObject& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObject& rCompare) const;

    virtual void ObjectInDestruction(const SfxItemPool& rSfxItemPool);

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
};

#endif  // _EDITOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
