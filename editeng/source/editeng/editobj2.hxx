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

#ifndef _EDITOBJ2_HXX
#define _EDITOBJ2_HXX

#include <editeng/editobj.hxx>
#include <editdoc.hxx>

#include <unotools/fontcvt.hxx>

#include <boost/ptr_container/ptr_vector.hpp>

class SfxStyleSheetPool;

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
    friend class BinTextObject;
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
};

class BinTextObject : public EditTextObject, public SfxItemPoolUser
{
    using EditTextObject::operator==;
    using EditTextObject::isWrongListEqual;

public:
    typedef boost::ptr_vector<ContentInfo> ContentInfosType;

private:
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

protected:
    virtual void            StoreData( SvStream& rOStream ) const;
    virtual void            CreateData( SvStream& rIStream );
    sal_Bool                    ImpChangeStyleSheets( const String& rOldName, SfxStyleFamily eOldFamily,
                                        const String& rNewName, SfxStyleFamily eNewFamily );

public:
                            BinTextObject( SfxItemPool* pPool );
                            BinTextObject( const BinTextObject& );
    virtual                 ~BinTextObject();

    virtual EditTextObject* Clone() const;

    sal_uInt16                  GetUserType() const;
    void                    SetUserType( sal_uInt16 n );

    sal_uLong                   GetObjectSettings() const;
    void                    SetObjectSettings( sal_uLong n );

    bool                    IsVertical() const;
    void                    SetVertical( bool b );

    sal_uInt16                  GetScriptType() const;
    void                    SetScriptType( sal_uInt16 nType );

    sal_uInt16                  GetVersion() const; // As long as the outliner does not store any record length

    ContentInfo*            CreateAndInsertContent();
    XEditAttribute*         CreateAttrib( const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd );
    void                    DestroyAttrib( XEditAttribute* pAttr );

    ContentInfosType&       GetContents();
    const ContentInfosType& GetContents() const;
    SfxItemPool*            GetPool() const         { return pPool; }
    XParaPortionList*       GetPortionInfo() const  { return pPortionInfo; }
    void                    SetPortionInfo( XParaPortionList* pP )
                                { pPortionInfo = pP; }

    virtual size_t GetParagraphCount() const;
    virtual String GetText(size_t nParagraph) const;
    virtual void Insert(const EditTextObject& rObj, size_t nPara);
    virtual EditTextObject* CreateTextObject(size_t nPara, size_t nParas = 1) const;
    virtual void RemoveParagraph(size_t nPara);

    virtual sal_Bool            HasPortionInfo() const;
    virtual void            ClearPortionInfo();

    virtual sal_Bool            HasOnlineSpellErrors() const;

    virtual sal_Bool            HasCharAttribs( sal_uInt16 nWhich = 0 ) const;
    virtual void            GetCharAttribs( sal_uInt16 nPara, std::vector<EECharAttrib>& rLst ) const;

    virtual sal_Bool            RemoveCharAttribs( sal_uInt16 nWhich = 0 );
    virtual sal_Bool            RemoveParaAttribs( sal_uInt16 nWhich = 0 );

    virtual void            MergeParaAttribs( const SfxItemSet& rAttribs, sal_uInt16 nStart, sal_uInt16 nEnd );

    virtual sal_Bool            IsFieldObject() const;
    virtual const SvxFieldItem* GetField() const;
    virtual sal_Bool            HasField( TypeId Type = NULL ) const;

    virtual SfxItemSet GetParaAttribs(size_t nPara) const;
    virtual void SetParaAttribs(size_t nPara, const SfxItemSet& rAttribs);

    virtual sal_Bool            HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const;
    virtual void GetStyleSheet(size_t nPara, String& rName, SfxStyleFamily& eFamily) const;
    virtual void SetStyleSheet(size_t nPara, const String& rName, const SfxStyleFamily& eFamily);
    virtual sal_Bool            ChangeStyleSheets(  const XubString& rOldName, SfxStyleFamily eOldFamily,
                                                const String& rNewName, SfxStyleFamily eNewFamily );
    virtual void            ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    virtual editeng::FieldUpdater GetFieldUpdater();

    void                    CreateData300( SvStream& rIStream );

    sal_Bool                    HasMetric() const           { return nMetric != 0xFFFF; }
    sal_uInt16                  GetMetric() const           { return nMetric; }
    void                    SetMetric( sal_uInt16 n )       { nMetric = n; }

    bool                    IsOwnerOfPool() const       { return bOwnerOfPool; }
    void                    StoreUnicodeStrings( sal_Bool b ) { bStoreUnicodeStrings = b; }

    bool                    operator==( const BinTextObject& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const BinTextObject& rCompare) const;

    // from SfxItemPoolUser
    virtual void ObjectInDestruction(const SfxItemPool& rSfxItemPool);
};

#endif  // _EDITOBJ2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
