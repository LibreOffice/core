/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <svl/itemset.hxx>
#include <edtspell.hxx>
#include <EditLineList.hxx>
#include <TextPortionList.hxx>

#include <svl/sharedstring.hxx>
#include <svl/languageoptions.hxx>
#include <tools/long.hxx>
#include <tools/mapunit.hxx>

#include <memory>
#include <vector>

namespace svl {

class SharedStringPool;

}

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
    friend class EditTextObject;

private:
    svl::SharedString   maText;
    OUString            aStyle;

    std::vector<XEditAttribute> maCharAttribs;
    SfxStyleFamily      eFamily;
    SfxItemSet aParaAttribs;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
