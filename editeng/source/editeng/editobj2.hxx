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


class SfxStyleSheetPool;

class XEditAttribute
{
    friend class ContentInfo;   // fuer DTOR
    friend class BinTextObject; // fuer DTOR

private:
    const SfxPoolItem*  pItem;
    sal_uInt16              nStart;
    sal_uInt16              nEnd;

                        XEditAttribute();
                        XEditAttribute( const XEditAttribute& rCopyFrom );

                        ~XEditAttribute();

public:
                        XEditAttribute( const SfxPoolItem& rAttr );
                        XEditAttribute( const SfxPoolItem& rAttr, sal_uInt16 nStart, sal_uInt16 nEnd );

    const SfxPoolItem*  GetItem() const             { return pItem; }

    sal_uInt16&             GetStart()                  { return nStart; }
    sal_uInt16&             GetEnd()                    { return nEnd; }

    sal_uInt16              GetStart() const            { return nStart; }
    sal_uInt16              GetEnd() const              { return nEnd; }

    sal_uInt16              GetLen() const              { return nEnd-nStart; }

    inline sal_Bool         IsFeature();

    inline bool         operator==( const XEditAttribute& rCompare );
};

inline bool XEditAttribute::operator==( const XEditAttribute& rCompare )
{
    return  (nStart == rCompare.nStart) &&
            (nEnd == rCompare.nEnd) &&
            ( (pItem == rCompare.pItem) ||
            ( pItem->Which() != rCompare.pItem->Which()) ||
            (*pItem == *rCompare.pItem));
}

inline sal_Bool XEditAttribute::IsFeature()
{
    sal_uInt16 nWhich = pItem->Which();
    return  ( ( nWhich >= EE_FEATURE_START ) &&
              ( nWhich <=  EE_FEATURE_END ) );
}

typedef XEditAttribute* XEditAttributePtr;
SV_DECL_PTRARR( XEditAttributeListImpl, XEditAttributePtr, 0, 4 )

class XEditAttributeList : public XEditAttributeListImpl
{
public:
    XEditAttribute* FindAttrib( sal_uInt16 nWhich, sal_uInt16 nChar ) const;
};

struct XParaPortion
{
    long                nHeight;
    sal_uInt16              nFirstLineOffset;

    EditLineList        aLines;
    TextPortionList     aTextPortions;
};

typedef XParaPortion* XParaPortionPtr;
SV_DECL_PTRARR( XBaseParaPortionList, XParaPortionPtr, 0, 4 )

class XParaPortionList : public  XBaseParaPortionList
{
    sal_uIntPtr         nRefDevPtr;
    OutDevType  eRefDevType;
    MapMode     aRefMapMode;
    sal_uLong       nPaperWidth;


public:
            XParaPortionList( OutputDevice* pRefDev, sal_uLong nPW ) :
                aRefMapMode( pRefDev->GetMapMode() )
                {
                    nRefDevPtr = (sal_uIntPtr)pRefDev; nPaperWidth = nPW;
                    eRefDevType = pRefDev->GetOutDevType();
                }

    sal_uIntPtr         GetRefDevPtr() const        { return nRefDevPtr; }
    sal_uLong           GetPaperWidth() const       { return nPaperWidth; }
    OutDevType      GetRefDevType() const       { return eRefDevType; }
    const MapMode&  GetRefMapMode() const       { return aRefMapMode; }
};

/* cl removed because not needed anymore since binfilter
struct LoadStoreTempInfos
{
    ByteString              aOrgString_Load;

    FontToSubsFontConverter hOldSymbolConv_Store;
    sal_Bool                    bSymbolParagraph_Store;


    LoadStoreTempInfos() { bSymbolParagraph_Store = sal_False; hOldSymbolConv_Store = NULL; }
};
*/

class ContentInfo
{
    friend class BinTextObject;

private:
    String              aText;
    String              aStyle;
    XEditAttributeList  aAttribs;
    SfxStyleFamily      eFamily;
    SfxItemSet          aParaAttribs;
    WrongList*          pWrongs;

/* cl removed because not needed anymore since binfilter
    LoadStoreTempInfos* pTempLoadStoreInfos;
*/

                        ContentInfo( SfxItemPool& rPool );
                        ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse  );

public:
                        ~ContentInfo();

    const String&       GetText()           const   { return aText; }
    const String&       GetStyle()          const   { return aStyle; }
    const XEditAttributeList& GetAttribs()  const   { return aAttribs; }
    const SfxItemSet&   GetParaAttribs()    const   { return aParaAttribs; }
    SfxStyleFamily      GetFamily()         const   { return eFamily; }

    String&             GetText()           { return aText; }
    String&             GetStyle()          { return aStyle; }
    XEditAttributeList& GetAttribs()        { return aAttribs; }
    SfxItemSet&         GetParaAttribs()    { return aParaAttribs; }
    SfxStyleFamily&     GetFamily()         { return eFamily; }

    WrongList*          GetWrongList() const            { return pWrongs; }
    void                SetWrongList( WrongList* p )    { pWrongs = p; }
    bool                operator==( const ContentInfo& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const ContentInfo& rCompare) const;
};

typedef ContentInfo* ContentInfoPtr;
SV_DECL_PTRARR( ContentInfoList, ContentInfoPtr, 1, 4 )

// MT 05/00: Sollte mal direkt EditTextObjekt werden => keine virtuellen Methoden mehr.

class BinTextObject : public EditTextObject, public SfxItemPoolUser
{
    using EditTextObject::operator==;
    using EditTextObject::isWrongListEqual;

private:
    ContentInfoList         aContents;
    SfxItemPool*            pPool;
    sal_Bool                    bOwnerOfPool;
    XParaPortionList*       pPortionInfo;

    sal_uInt32              nObjSettings;
    sal_uInt16                  nMetric;
    sal_uInt16                  nVersion;
    sal_uInt16                  nUserType;
    sal_uInt16                  nScriptType;

    sal_Bool                    bVertical;
    sal_Bool                    bStoreUnicodeStrings;

protected:
    void                    DeleteContents();
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

    sal_Bool                    IsVertical() const;
    void                    SetVertical( sal_Bool b );

    sal_uInt16                  GetScriptType() const;
    void                    SetScriptType( sal_uInt16 nType );

    sal_uInt16                  GetVersion() const; // Solange der Outliner keine Recordlaenge speichert

    ContentInfo*            CreateAndInsertContent();
    XEditAttribute*         CreateAttrib( const SfxPoolItem& rItem, sal_uInt16 nStart, sal_uInt16 nEnd );
    void                    DestroyAttrib( XEditAttribute* pAttr );

    ContentInfoList&        GetContents()           { return aContents; }
    const ContentInfoList&  GetContents() const     { return aContents; }
    SfxItemPool*            GetPool() const         { return pPool; }
    XParaPortionList*       GetPortionInfo() const  { return pPortionInfo; }
    void                    SetPortionInfo( XParaPortionList* pP )
                                { pPortionInfo = pP; }

    virtual sal_uInt16          GetParagraphCount() const;
    virtual String          GetText( sal_uInt16 nParagraph ) const;
    virtual void            Insert( const EditTextObject& rObj, sal_uInt16 nPara );
    virtual EditTextObject* CreateTextObject( sal_uInt16 nPara, sal_uInt16 nParas = 1 ) const;
    virtual void            RemoveParagraph( sal_uInt16 nPara );

    virtual sal_Bool            HasPortionInfo() const;
    virtual void            ClearPortionInfo();

    virtual sal_Bool            HasOnlineSpellErrors() const;

    virtual sal_Bool            HasCharAttribs( sal_uInt16 nWhich = 0 ) const;
    virtual void            GetCharAttribs( sal_uInt16 nPara, EECharAttribArray& rLst ) const;

    virtual sal_Bool            RemoveCharAttribs( sal_uInt16 nWhich = 0 );
    virtual sal_Bool            RemoveParaAttribs( sal_uInt16 nWhich = 0 );

    virtual void            MergeParaAttribs( const SfxItemSet& rAttribs, sal_uInt16 nStart, sal_uInt16 nEnd );

    virtual sal_Bool            IsFieldObject() const;
    virtual const SvxFieldItem* GetField() const;
    virtual sal_Bool            HasField( TypeId Type = NULL ) const;

    SfxItemSet              GetParaAttribs( sal_uInt16 nPara ) const;
    void                    SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rAttribs );

    virtual sal_Bool            HasStyleSheet( const XubString& rName, SfxStyleFamily eFamily ) const;
    virtual void            GetStyleSheet( sal_uInt16 nPara, XubString& rName, SfxStyleFamily& eFamily ) const;
    virtual void            SetStyleSheet( sal_uInt16 nPara, const XubString& rName, const SfxStyleFamily& eFamily );
    virtual sal_Bool            ChangeStyleSheets(  const XubString& rOldName, SfxStyleFamily eOldFamily,
                                                const String& rNewName, SfxStyleFamily eNewFamily );
    virtual void            ChangeStyleSheetName( SfxStyleFamily eFamily, const XubString& rOldName, const XubString& rNewName );

    void                    CreateData300( SvStream& rIStream );

    sal_Bool                    HasMetric() const           { return nMetric != 0xFFFF; }
    sal_uInt16                  GetMetric() const           { return nMetric; }
    void                    SetMetric( sal_uInt16 n )       { nMetric = n; }

    sal_Bool                    IsOwnerOfPool() const       { return bOwnerOfPool; }
    void                    StoreUnicodeStrings( sal_Bool b ) { bStoreUnicodeStrings = b; }

/* cl removed because not needed anymore since binfilter
    void                    PrepareStore( SfxStyleSheetPool* pStyleSheetPool );
    void                    FinishStore();
    void                    FinishLoad( SfxStyleSheetPool* pStyleSheetPool );
*/

    bool                    operator==( const BinTextObject& rCompare ) const;

    // #i102062#
    bool isWrongListEqual(const BinTextObject& rCompare) const;

    // from SfxItemPoolUser
    virtual void ObjectInDestruction(const SfxItemPool& rSfxItemPool);
};

#endif  // _EDITOBJ2_HXX

