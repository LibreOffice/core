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

#pragma once

#include <editeng/editobj.hxx>
#include <editeng/fieldupdater.hxx>
#include <editeng/outliner.hxx>
#include <editdoc.hxx>

#include <svl/sharedstring.hxx>
#include <svl/languageoptions.hxx>
#include <tools/long.hxx>
#include <tools/mapunit.hxx>

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
    SfxPoolItemHolder   maItemHolder;
    sal_Int32           nStart;
    sal_Int32           nEnd;

public:
    XEditAttribute(SfxItemPool&, const SfxPoolItem&, sal_Int32 nStart, sal_Int32 nEnd );

    const SfxPoolItem*      GetItem() const             { return maItemHolder.getItem(); }

    sal_Int32&              GetStart()                  { return nStart; }
    sal_Int32&              GetEnd()                    { return nEnd; }

    sal_Int32               GetStart() const            { return nStart; }
    sal_Int32               GetEnd() const              { return nEnd; }

    sal_Int32               GetLen() const              { return nEnd-nStart; }

    bool IsFeature() const;
    void SetItem(SfxItemPool&, const SfxPoolItem&);

    inline bool operator==( const XEditAttribute& rCompare ) const;
};

inline bool XEditAttribute::operator==( const XEditAttribute& rCompare ) const
{
    return  (nStart == rCompare.nStart) &&
            (nEnd == rCompare.nEnd) &&
            SfxPoolItem::areSame(GetItem(), rCompare.GetItem());
}

struct XParaPortion
{
    tools::Long                nHeight;
    sal_uInt16          nFirstLineOffset;

    EditLineList        aLines;
    TextPortionList     aTextPortions;
};

class XParaPortionList
{
    typedef std::vector<std::unique_ptr<XParaPortion> > ListType;
    ListType maList;

    VclPtr<OutputDevice> pRefDevPtr;
    double  mfFontScaleX;
    double  mfFontScaleY;
    double  mfSpacingScaleX;
    double  mfSpacingScaleY;
    sal_uInt32  nPaperWidth;

public:
    XParaPortionList(OutputDevice* pRefDev, sal_uInt32 nPW, double fFontScaleX, double fFontScaleY, double fSpacingScaleX, double fSpacingScaleY);

    void push_back(XParaPortion* p);
    const XParaPortion& operator[](size_t i) const;

    OutputDevice*       GetRefDevPtr() const        { return pRefDevPtr; }
    sal_uInt32          GetPaperWidth() const       { return nPaperWidth; }
    bool                RefDevIsVirtual() const {return pRefDevPtr->IsVirtual();}
    const MapMode&  GetRefMapMode() const       { return pRefDevPtr->GetMapMode(); }
    double  getFontScaleX() const { return mfFontScaleX; }
    double  getFontScaleY() const { return mfFontScaleY; }
    double  getSpacingScaleX() const { return mfSpacingScaleX; }
    double  getSpacingScaleY() const { return mfSpacingScaleY; }
};

class ContentInfo
{
    friend class EditTextObjectImpl;

private:
    svl::SharedString   maText;
    OUString            aStyle;

    std::vector<XEditAttribute> maCharAttribs;
    SfxStyleFamily      eFamily;
    SfxItemSetFixed<EE_PARA_START, EE_CHAR_END> aParaAttribs;
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
    sal_Int32 GetTextLen() const;

    void dumpAsXml(xmlTextWriterPtr pWriter) const;

    const std::vector<XEditAttribute>& GetCharAttribs() const { return maCharAttribs; }
    std::vector<XEditAttribute>& GetCharAttribs() { return maCharAttribs; }

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

class EditTextObjectImpl final : public EditTextObject
{
public:
    typedef std::vector<std::unique_ptr<ContentInfo> > ContentInfosType;

private:
    ContentInfosType        maContents;
    rtl::Reference<SfxItemPool>       mpPool;
    std::unique_ptr<XParaPortionList> mpPortionInfo;

    OutlinerMode            meUserType;
    SvtScriptType           meScriptType;
    TextRotation            meRotation;
    MapUnit                 meMetric;

    bool                    mbVertical;

    bool ImpChangeStyleSheets( std::u16string_view rOldName, SfxStyleFamily eOldFamily,
                               const OUString& rNewName, SfxStyleFamily eNewFamily );

public:
    EditTextObjectImpl(SfxItemPool* pPool, MapUnit eDefaultMetric, bool bVertical,
                       TextRotation eRotation, SvtScriptType eScriptType);
    EditTextObjectImpl( const EditTextObjectImpl& r );
    virtual ~EditTextObjectImpl() override;

    EditTextObjectImpl& operator=(const EditTextObjectImpl&) = delete;

    virtual OutlinerMode GetUserType() const override { return meUserType;}
    virtual void SetUserType( OutlinerMode n ) override;

    virtual void NormalizeString( svl::SharedStringPool& rPool ) override;
    virtual std::vector<svl::SharedString> GetSharedStrings() const override;

    virtual bool                    IsEffectivelyVertical() const override;
    virtual bool                    GetVertical() const override;
    virtual bool                    IsTopToBottom() const override;
    virtual void                    SetVertical( bool bVert) override;
    virtual void                    SetRotation(TextRotation nRotation) override;
    virtual TextRotation            GetRotation() const override;

    virtual SvtScriptType           GetScriptType() const override { return meScriptType;}

    virtual std::unique_ptr<EditTextObject> Clone() const override;

    ContentInfo*            CreateAndInsertContent();
    XEditAttribute CreateAttrib( const SfxPoolItem& rItem, sal_Int32 nStart, sal_Int32 nEnd );

    ContentInfosType&       GetContents() { return maContents;}
    const ContentInfosType& GetContents() const { return maContents;}
    SfxItemPool*            GetPool()         { return mpPool.get(); }
    virtual const SfxItemPool* GetPool() const override { return mpPool.get(); }
    XParaPortionList*       GetPortionInfo() const  { return mpPortionInfo.get(); }
    void                    SetPortionInfo( std::unique_ptr<XParaPortionList> pP )
                                { mpPortionInfo = std::move(pP); }

    virtual sal_Int32 GetParagraphCount() const override;
    virtual OUString GetText(sal_Int32 nParagraph) const override;
    virtual sal_Int32 GetTextLen(sal_Int32 nParagraph) const override;

    virtual void ClearPortionInfo() override;

    virtual bool HasOnlineSpellErrors() const override;

    virtual void GetCharAttribs( sal_Int32 nPara, std::vector<EECharAttrib>& rLst ) const override;

    virtual bool RemoveCharAttribs( sal_uInt16 nWhich ) override;

    virtual void GetAllSections( std::vector<editeng::Section>& rAttrs ) const override;

    virtual bool IsFieldObject() const override;
    virtual const SvxFieldItem* GetField() const override;
    virtual const SvxFieldData* GetFieldData(sal_Int32 nPara, size_t nPos, sal_Int32 nType) const override;

    virtual bool HasField( sal_Int32 nType = css::text::textfield::Type::UNSPECIFIED ) const override;

    virtual const SfxItemSet& GetParaAttribs(sal_Int32 nPara) const override;

    virtual void GetStyleSheet(sal_Int32 nPara, OUString& rName, SfxStyleFamily& eFamily) const override;
    virtual void SetStyleSheet(sal_Int32 nPara, const OUString& rName, const SfxStyleFamily& eFamily) override;
    virtual bool ChangeStyleSheets(
        std::u16string_view rOldName, SfxStyleFamily eOldFamily, const OUString& rNewName, SfxStyleFamily eNewFamily) override;
    virtual void ChangeStyleSheetName(SfxStyleFamily eFamily, std::u16string_view rOldName, const OUString& rNewName) override;

    virtual editeng::FieldUpdater GetFieldUpdater() override { return editeng::FieldUpdater(*this); }

    bool HasMetric() const { return meMetric != MapUnit::LASTENUMDUMMY; }
    MapUnit                  GetMetric() const           { return meMetric; }

    virtual bool operator==( const EditTextObject& rCompare ) const override;
    bool Equals( const EditTextObjectImpl& rCompare, bool bComparePool ) const;

    // #i102062#
    virtual bool isWrongListEqual(const EditTextObject& rCompare) const override;

#if DEBUG_EDIT_ENGINE
    virtual void Dump() const override;
#endif
    virtual void dumpAsXml(xmlTextWriterPtr pWriter) const override;
};

inline EditTextObjectImpl& toImpl(EditTextObject& rObj)
{
    assert(dynamic_cast<EditTextObjectImpl*>(&rObj));
    return static_cast<EditTextObjectImpl&>(rObj);
}

inline const EditTextObjectImpl& toImpl(const EditTextObject& rObj)
{
    assert(dynamic_cast<const EditTextObjectImpl*>(&rObj));
    return static_cast<const EditTextObjectImpl&>(rObj);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
