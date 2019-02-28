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

#ifndef INCLUDED_EDITENG_SOURCE_EDITENG_EDITOBJ2_HXX
#define INCLUDED_EDITENG_SOURCE_EDITENG_EDITOBJ2_HXX

#include <editeng/editobj.hxx>
#include <editeng/fieldupdater.hxx>
#include <editeng/outliner.hxx>
#include <editdoc.hxx>

#include <unotools/fontcvt.hxx>
#include <svl/sharedstring.hxx>
#include <svl/languageoptions.hxx>

#include <memory>
#include <vector>

namespace editeng {

struct Section;

}

namespace svl {

class SharedStringPool;

}

class XEditAttribute
{
private:
    const SfxPoolItem*  pItem;
    sal_Int32           nStart;
    sal_Int32           nEnd;

                        XEditAttribute( const XEditAttribute& rCopyFrom ) = delete;

public:
    XEditAttribute( const SfxPoolItem& rAttr, sal_Int32 nStart, sal_Int32 nEnd );
    ~XEditAttribute();

    const SfxPoolItem*      GetItem() const             { return pItem; }

    sal_Int32&              GetStart()                  { return nStart; }
    sal_Int32&              GetEnd()                    { return nEnd; }

    sal_Int32               GetStart() const            { return nStart; }
    sal_Int32               GetEnd() const              { return nEnd; }

    sal_Int32               GetLen() const              { return nEnd-nStart; }

    bool IsFeature() const;
    void SetItem(const SfxPoolItem& rNew);

    inline bool operator==( const XEditAttribute& rCompare ) const;
};

inline bool XEditAttribute::operator==( const XEditAttribute& rCompare ) const
{
    return  (nStart == rCompare.nStart) &&
            (nEnd == rCompare.nEnd) &&
            ((pItem == rCompare.pItem) ||
             ((pItem->Which() == rCompare.pItem->Which()) &&
              (*pItem == *rCompare.pItem)));
}

struct XParaPortion
{
    long                nHeight;
    sal_uInt16          nFirstLineOffset;

    EditLineList        aLines;
    TextPortionList     aTextPortions;
};

class XParaPortionList
{
    typedef std::vector<std::unique_ptr<XParaPortion> > ListType;
    ListType maList;

    VclPtr<OutputDevice> nRefDevPtr;
    OutDevType  eRefDevType;
    MapMode     aRefMapMode;
    sal_uInt16  nStretchX;
    sal_uInt16  nStretchY;
    sal_uLong   nPaperWidth;

public:
    XParaPortionList(OutputDevice* pRefDev, sal_uLong nPW, sal_uInt16 _nStretchX, sal_uInt16 _nStretchY);

    void push_back(XParaPortion* p);
    const XParaPortion& operator[](size_t i) const;

    OutputDevice*       GetRefDevPtr() const        { return nRefDevPtr; }
    sal_uLong           GetPaperWidth() const       { return nPaperWidth; }
    bool RefDevIsVirtual() const
        { return (eRefDevType == OUTDEV_VIRDEV) || (eRefDevType == OUTDEV_PDF); }
    const MapMode&  GetRefMapMode() const       { return aRefMapMode; }
    sal_uInt16  GetStretchX() const         { return nStretchX; }
    sal_uInt16  GetStretchY() const         { return nStretchY; }
};

class ContentInfo
{
    friend class EditTextObjectImpl;
public:
    typedef std::vector<std::unique_ptr<XEditAttribute> > XEditAttributesType;

private:
    svl::SharedString   maText;
    OUString            aStyle;

    XEditAttributesType maCharAttribs;
    SfxStyleFamily      eFamily;
    SfxItemSet          aParaAttribs;
    std::unique_ptr<WrongList>
                        mpWrongs;

                        ContentInfo( SfxItemPool& rPool );
                        ContentInfo( const ContentInfo& rCopyFrom, SfxItemPool& rPoolToUse  );

public:
                        ~ContentInfo();
                        ContentInfo(const ContentInfo&) = delete;
    ContentInfo&        operator=(const ContentInfo&) = delete;

    void NormalizeString( svl::SharedStringPool& rPool );
    const svl::SharedString& GetSharedString() const { return maText;}
    OUString GetText() const;
    void SetText( const OUString& rStr );

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    const XEditAttributesType& GetCharAttribs() const { return maCharAttribs; }
    XEditAttributesType& GetCharAttribs() { return maCharAttribs; }

    const OUString&     GetStyle()          const   { return aStyle; }
    SfxStyleFamily      GetFamily()         const   { return eFamily; }

    void                SetStyle(const OUString& rStyle) { aStyle = rStyle; }
    void                SetFamily(const SfxStyleFamily& rFamily) { eFamily  = rFamily; }

    const SfxItemSet&   GetParaAttribs()    const   { return aParaAttribs; }
    SfxItemSet&         GetParaAttribs()    { return aParaAttribs; }

    const WrongList* GetWrongList() const;
    void SetWrongList( WrongList* p );
    bool Equals( const ContentInfo& rCompare, bool bComparePool ) const;

    // #i102062#
    bool isWrongListEqual(const ContentInfo& rCompare) const;

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
};

class EditTextObjectImpl
{
friend class EditTextObject;
public:
    typedef std::vector<std::unique_ptr<ContentInfo> > ContentInfosType;

private:
    EditTextObject* mpFront;

    ContentInfosType        aContents;
    SfxItemPool*            pPool;
    std::unique_ptr<XParaPortionList> pPortionInfo;

    sal_uInt16              nMetric;
    OutlinerMode            nUserType;
    SvtScriptType           nScriptType;

    bool                    bOwnerOfPool:1;
    bool                    bVertical:1;
    bool                    bIsTopToBottomVert : 1;

    bool ImpChangeStyleSheets( const OUString& rOldName, SfxStyleFamily eOldFamily,
                               const OUString& rNewName, SfxStyleFamily eNewFamily );

public:
    EditTextObjectImpl( EditTextObject* pFront, SfxItemPool* pPool );
    EditTextObjectImpl( EditTextObject* pFront, const EditTextObjectImpl& r );
    ~EditTextObjectImpl();

    EditTextObjectImpl(const EditTextObjectImpl&) = delete;
    EditTextObjectImpl& operator=(const EditTextObjectImpl&) = delete;

    OutlinerMode GetUserType() const { return nUserType;}
    void SetUserType( OutlinerMode n );

    void NormalizeString( svl::SharedStringPool& rPool );
    std::vector<svl::SharedString> GetSharedStrings() const;

    bool                    IsVertical() const;
    bool                    IsTopToBottom() const;
    void                    SetVertical( bool bVert, bool bTopToBottom);

    SvtScriptType           GetScriptType() const { return nScriptType;}
    void                    SetScriptType( SvtScriptType nType );

    ContentInfo*            CreateAndInsertContent();
    std::unique_ptr<XEditAttribute> CreateAttrib( const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd );
    void                    DestroyAttrib( std::unique_ptr<XEditAttribute> pAttr );

    ContentInfosType&       GetContents() { return aContents;}
    const ContentInfosType& GetContents() const { return aContents;}
    SfxItemPool*            GetPool() const         { return pPool; }
    XParaPortionList*       GetPortionInfo() const  { return pPortionInfo.get(); }
    void                    SetPortionInfo( std::unique_ptr<XParaPortionList> pP )
                                { pPortionInfo = std::move(pP); }

    sal_Int32 GetParagraphCount() const;
    OUString GetText(sal_Int32 nParagraph) const;

    void ClearPortionInfo();

    bool HasOnlineSpellErrors() const;

    void GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const;

    bool RemoveCharAttribs( sal_uInt16 nWhich );

    void GetAllSections( std::vector<editeng::Section>& rAttrs ) const;

    bool IsFieldObject() const;
    const SvxFieldItem* GetField() const;
    const SvxFieldData* GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const;

    bool HasField( sal_Int32 nType ) const;

    const SfxItemSet& GetParaAttribs(sal_Int32 nPara) const;

    void GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& eFamily) const;
    void SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& eFamily);
    bool ChangeStyleSheets(
        const OUString& rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily);
    void ChangeStyleSheetName(SfxStyleFamily eFamily, const OUString& rOldName, const OUString& rNewName);

    editeng::FieldUpdater GetFieldUpdater() { return editeng::FieldUpdater(*mpFront);}

    bool HasMetric() const { return nMetric != 0xFFFF; }
    sal_uInt16                  GetMetric() const           { return nMetric; }
    void                    SetMetric( sal_uInt16 n )       { nMetric = n; }

    bool                    IsOwnerOfPool() const       { return bOwnerOfPool; }

    bool operator==( const EditTextObjectImpl& rCompare ) const;
    bool Equals( const EditTextObjectImpl& rCompare, bool bComparePool ) const;

    // #i102062#
    bool isWrongListEqual(const EditTextObjectImpl& rCompare) const;

    // from SfxItemPoolUser
    void ObjectInDestruction(const SfxItemPool& rSfxItemPool);

#if DEBUG_EDIT_ENGINE
    void Dump() const;
#endif
};

#endif // INCLUDED_EDITENG_SOURCE_EDITENG_EDITOBJ2_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
