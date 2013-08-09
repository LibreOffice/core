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

#ifndef _EDITOBJ2_HXX
#define _EDITOBJ2_HXX

#include <editeng/editobj.hxx>
#include <editdoc.hxx>

#include <unotools/fontcvt.hxx>

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>

class XEditAttribute
{
private:
    const SfxPoolItem*  pItem;
    sal_uInt16              nStart;
    sal_uInt16              nEnd;

                        XEditAttribute();
                        XEditAttribute( const XEditAttribute& rCopyFrom );

public:
    XEditAttribute( const SfxPoolItem& rAttr, sal_uInt16 nStart, sal_uInt16 nEnd );
    ~XEditAttribute();

    const SfxPoolItem*  GetItem() const             { return pItem; }

    sal_uInt16&             GetStart()                  { return nStart; }
    sal_uInt16&             GetEnd()                    { return nEnd; }

    sal_uInt16              GetStart() const            { return nStart; }
    sal_uInt16              GetEnd() const              { return nEnd; }

    sal_uInt16              GetLen() const              { return nEnd-nStart; }

    bool IsFeature() const;
    void SetItem(const SfxPoolItem& rNew);

    inline bool operator==( const XEditAttribute& rCompare ) const;

    bool operator!= (const XEditAttribute& r) const
    {
        return !operator==(r);
    }
};

inline bool XEditAttribute::operator==( const XEditAttribute& rCompare ) const
{
    return  (nStart == rCompare.nStart) &&
            (nEnd == rCompare.nEnd) &&
            ( (pItem == rCompare.pItem) ||
            ( pItem->Which() != rCompare.pItem->Which()) ||
            (*pItem == *rCompare.pItem));
}

struct XParaPortion
{
    long                nHeight;
    sal_uInt16              nFirstLineOffset;

    EditLineList        aLines;
    TextPortionList     aTextPortions;
};

class XParaPortionList
{
    typedef boost::ptr_vector<XParaPortion> ListType;
    ListType maList;

    sal_uIntPtr nRefDevPtr;
    OutDevType  eRefDevType;
    MapMode     aRefMapMode;
    sal_uInt16  nStretchX;
    sal_uInt16  nStretchY;
    sal_uLong   nPaperWidth;

public:
    XParaPortionList(OutputDevice* pRefDev, sal_uLong nPW, sal_uInt16 _nStretchX, sal_uInt16 _nStretchY);

    void push_back(XParaPortion* p);
    const XParaPortion& operator[](size_t i) const;

    sal_uIntPtr         GetRefDevPtr() const        { return nRefDevPtr; }
    sal_uLong           GetPaperWidth() const       { return nPaperWidth; }
    OutDevType      GetRefDevType() const       { return eRefDevType; }
    const MapMode&  GetRefMapMode() const       { return aRefMapMode; }
    sal_uInt16  GetStretchX() const         { return nStretchX; }
    sal_uInt16  GetStretchY() const         { return nStretchY; }

};

class ContentInfo
{
    friend class EditTextObjectImpl;
public:
    typedef boost::ptr_vector<XEditAttribute> XEditAttributesType;

private:
    String              aText;
    String              aStyle;

    XEditAttributesType aAttribs;
    SfxStyleFamily      eFamily;
    SfxItemSet          aParaAttribs;
    WrongList*          pWrongs;

                        ContentInfo( SfxItemPool& rPool );
                        ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse  );

public:
                        ~ContentInfo();

    const XEditAttributesType& GetAttribs() const { return aAttribs; }
    XEditAttributesType& GetAttribs() { return aAttribs; }

    const String&       GetText()           const   { return aText; }
    const String&       GetStyle()          const   { return aStyle; }
    const SfxItemSet&   GetParaAttribs()    const   { return aParaAttribs; }
    SfxStyleFamily      GetFamily()         const   { return eFamily; }

    String&             GetText()           { return aText; }
    String&             GetStyle()          { return aStyle; }
    SfxItemSet&         GetParaAttribs()    { return aParaAttribs; }
    SfxStyleFamily&     GetFamily()         { return eFamily; }

    WrongList*          GetWrongList() const            { return pWrongs; }
    void                SetWrongList( WrongList* p )    { pWrongs = p; }
    bool operator==( const ContentInfo& rCompare ) const;
    bool operator!=( const ContentInfo& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const ContentInfo& rCompare) const;

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
};

class EditTextObjectImpl : boost::noncopyable
{
public:
    typedef boost::ptr_vector<ContentInfo> ContentInfosType;

private:
    EditTextObject* mpFront;

    ContentInfosType        aContents;
    SfxItemPool*            pPool;
    XParaPortionList*       pPortionInfo;

    sal_uInt32              nObjSettings;
    sal_uInt16                  nMetric;
    sal_uInt16                  nVersion;
    sal_uInt16                  nUserType;
    sal_uInt16                  nScriptType;

    bool                    bOwnerOfPool:1;
    bool                    bVertical:1;
    bool                    bStoreUnicodeStrings:1;

    bool ImpChangeStyleSheets( const String& rOldName, SfxStyleFamily eOldFamily,
                               const String& rNewName, SfxStyleFamily eNewFamily );

public:
    void StoreData( SvStream& rOStream ) const;
    void CreateData( SvStream& rIStream );

    EditTextObjectImpl( EditTextObject* pFront, SfxItemPool* pPool );
    EditTextObjectImpl( EditTextObject* pFront, const EditTextObjectImpl& r );
    ~EditTextObjectImpl();

    sal_uInt16 GetUserType() const;
    void SetUserType( sal_uInt16 n );

    bool                    IsVertical() const;
    void                    SetVertical( bool b );

    sal_uInt16                  GetScriptType() const;
    void                    SetScriptType( sal_uInt16 nType );

    ContentInfo*            CreateAndInsertContent();
    XEditAttribute*         CreateAttrib( const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd );
    void                    DestroyAttrib( XEditAttribute* pAttr );

    ContentInfosType&       GetContents();
    const ContentInfosType& GetContents() const;
    SfxItemPool*            GetPool() const         { return pPool; }
    XParaPortionList*       GetPortionInfo() const  { return pPortionInfo; }
    void                    SetPortionInfo( XParaPortionList* pP )
                                { pPortionInfo = pP; }

    sal_Int32 GetParagraphCount() const;
    String GetText(sal_Int32 nParagraph) const;

    void ClearPortionInfo();

    bool HasOnlineSpellErrors() const;

    void GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    bool RemoveCharAttribs( sal_uInt16 nWhich = 0 );

    bool IsFieldObject() const;
    const SvxFieldItem* GetField() const;
    const SvxFieldData* GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const;

    bool HasField( sal_Int32 nType = com::sun::star::text::textfield::Type::UNSPECIFIED ) const;

    const SfxItemSet& GetParaAttribs(sal_Int32 nPara) const;
    void SetParaAttribs(sal_Int32 nPara, const SfxItemSet& rAttribs);

    void GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& eFamily) const;
    void SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& eFamily);
    bool ChangeStyleSheets(
        const OUString& rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const OUString& rOldName, const OUString& rNewName);

    editeng::FieldUpdater GetFieldUpdater();

    bool HasMetric() const { return nMetric != 0xFFFF; }
    sal_uInt16                  GetMetric() const           { return nMetric; }
    void                    SetMetric( sal_uInt16 n )       { nMetric = n; }

    bool                    IsOwnerOfPool() const       { return bOwnerOfPool; }
    void StoreUnicodeStrings( bool b ) { bStoreUnicodeStrings = b; }

    bool operator==( const EditTextObjectImpl& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObjectImpl& rCompare) const;

    // from SfxItemPoolUser
    void ObjectInDestruction(const SfxItemPool& rSfxItemPool);

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
};

#endif  // _EDITOBJ2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
