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

#ifndef _EDITOBJ_HXX
#define _EDITOBJ_HXX

#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <rsc/rscsfx.hxx>
#include <svl/itemset.hxx>
#include <editeng/eeitem.hxx>
#include "editeng/editengdllapi.h"

DBG_NAMEEX( EE_EditTextObject )

class SfxItemPool;
class SfxStyleSheetPool;
class SvxFieldItem;
class EECharAttribArray;

#define EDTOBJ_SETTINGS_ULITEMSUMMATION     0x00000001
#define EDTOBJ_SETTINGS_ULITEMFIRSTPARA     0x00000002

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

    virtual sal_uInt16      GetUserType() const;    // Fuer OutlinerMode, der kann das aber nicht kompatibel speichern
    virtual void        SetUserType( sal_uInt16 n );

    virtual sal_uLong       GetObjectSettings() const;
    virtual void        SetObjectSettings( sal_uLong n );

    virtual sal_Bool        IsVertical() const;
    virtual void        SetVertical( sal_Bool bVertical );

    virtual sal_uInt16      GetScriptType() const;

    virtual sal_uInt16      GetVersion() const; // Solange der Outliner keine Recordlaenge speichert

    virtual EditTextObject* Clone() const = 0;

    sal_Bool                    Store( SvStream& rOStream ) const;
    static EditTextObject*  Create( SvStream& rIStream,
                                SfxItemPool* pGlobalTextObjectPool = 0 );
    void                    Skip( SvStream& rIStream );

    virtual sal_uInt16      GetParagraphCount() const;

    virtual XubString   GetText( sal_uInt16 nParagraph ) const;
    virtual void        Insert( const EditTextObject& rObj, sal_uInt16 nPara );
    virtual void        RemoveParagraph( sal_uInt16 nPara );
    virtual EditTextObject* CreateTextObject( sal_uInt16 nPara, sal_uInt16 nParas = 1 ) const;

    virtual sal_Bool        HasPortionInfo() const;
    virtual void        ClearPortionInfo();

    virtual sal_Bool        HasOnlineSpellErrors() const;

    virtual sal_Bool        HasCharAttribs( sal_uInt16 nWhich = 0 ) const;
    virtual void        GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const;

    virtual sal_Bool        RemoveCharAttribs( sal_uInt16 nWhich = 0 );
    virtual sal_Bool        RemoveParaAttribs( sal_uInt16 nWhich = 0 );

    virtual void        MergeParaAttribs( const SfxItemSet& rAttribs, sal_uInt16 nStart = EE_CHAR_START, sal_uInt16 nEnd = EE_CHAR_END );

    virtual sal_Bool        IsFieldObject() const;
    virtual const SvxFieldItem* GetField() const;
    virtual sal_Bool        HasField( TypeId aType = NULL ) const;

    virtual SfxItemSet  GetParaAttribs( sal_uInt16 nPara ) const;
    virtual void        SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rAttribs );

    virtual sal_Bool        HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const;
    virtual void        GetStyleSheet( sal_uInt16 nPara, XubString& rName, SfxStyleFamily& eFamily ) const;
    virtual void        SetStyleSheet( sal_uInt16 nPara, const XubString& rName, const SfxStyleFamily& eFamily );
    virtual sal_Bool        ChangeStyleSheets(  const XubString& rOldName, SfxStyleFamily eOldFamily,
                                            const XubString& rNewName, SfxStyleFamily eNewFamily );
    virtual void        ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    bool                operator==( const EditTextObject& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObject& rCompare) const;
};

#endif  // _EDITOBJ_HXX
