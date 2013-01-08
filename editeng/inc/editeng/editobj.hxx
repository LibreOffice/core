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
#include <editeng/eeitem.hxx>
#include <editeng/editdata.hxx>
#include "editeng/editengdllapi.h"

#include <com/sun/star/text/textfield/Type.hpp>

#include <vector>

DBG_NAMEEX( EE_EditTextObject )

class SfxItemPool;
class SfxStyleSheetPool;
class SvxFieldItem;

namespace editeng {

class FieldUpdater;

}

class EDITENG_DLLPUBLIC EditTextObject
{
private:
    sal_uInt16              nWhich;
    EDITENG_DLLPRIVATE EditTextObject&      operator=( const EditTextObject& );

protected:
                        EditTextObject( sal_uInt16 nWhich );
                        EditTextObject( const EditTextObject& r );

    virtual void        StoreData( SvStream& rOStream ) const;
    virtual void        CreateData( SvStream& rIStream );

public:
    virtual             ~EditTextObject();

    sal_uInt16              Which() const { return nWhich; }

    virtual sal_uInt16      GetUserType() const;    // For OutlinerMode, it can however not save in compatible format
    virtual void        SetUserType( sal_uInt16 n );

    virtual sal_uLong       GetObjectSettings() const;
    virtual void        SetObjectSettings( sal_uLong n );

    virtual bool        IsVertical() const;
    virtual void        SetVertical( bool bVertical );

    virtual sal_uInt16      GetScriptType() const;

    virtual sal_uInt16      GetVersion() const; // As long as the outliner does not store any record length.

    virtual EditTextObject* Clone() const = 0;

    sal_Bool                    Store( SvStream& rOStream ) const;
    static EditTextObject*  Create( SvStream& rIStream,
                                SfxItemPool* pGlobalTextObjectPool = 0 );

    virtual size_t GetParagraphCount() const;

    virtual String GetText(size_t nParagraph) const;
    virtual void Insert(const EditTextObject& rObj, size_t nPara);
    virtual void RemoveParagraph(size_t nPara);
    virtual EditTextObject* CreateTextObject(size_t nPara, size_t nParas = 1) const;

    virtual sal_Bool        HasPortionInfo() const;
    virtual void        ClearPortionInfo();

    virtual sal_Bool        HasOnlineSpellErrors() const;

    virtual sal_Bool        HasCharAttribs( sal_uInt16 nWhich = 0 ) const;
    virtual void        GetCharAttribs( sal_uInt16 nPara, std::vector<EECharAttrib>& rLst ) const;

    virtual sal_Bool        RemoveCharAttribs( sal_uInt16 nWhich = 0 );
    virtual sal_Bool        RemoveParaAttribs( sal_uInt16 nWhich = 0 );

    virtual void        MergeParaAttribs( const SfxItemSet& rAttribs, sal_uInt16 nStart = EE_CHAR_START, sal_uInt16 nEnd = EE_CHAR_END );

    virtual sal_Bool        IsFieldObject() const;
    virtual const SvxFieldItem* GetField() const;
    virtual bool HasField( sal_Int32 nType = com::sun::star::text::textfield::Type::UNSPECIFIED ) const = 0;

    virtual SfxItemSet GetParaAttribs(size_t nPara) const;
    virtual void SetParaAttribs(size_t nPara, const SfxItemSet& rAttribs);

    virtual sal_Bool        HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const;
    virtual void GetStyleSheet(size_t nPara, String& rName, SfxStyleFamily& eFamily) const;
    virtual void SetStyleSheet(size_t nPara, const String& rName, const SfxStyleFamily& eFamily);
    virtual sal_Bool        ChangeStyleSheets(  const XubString& rOldName, SfxStyleFamily eOldFamily,
                                            const XubString& rNewName, SfxStyleFamily eNewFamily );
    virtual void        ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    virtual editeng::FieldUpdater GetFieldUpdater() = 0;

    bool                operator==( const EditTextObject& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObject& rCompare) const;
};

#endif  // _EDITOBJ_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
