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

#ifndef INCLUDED_SD_SOURCE_FILTER_EPPT_TEXT_HXX
#define INCLUDED_SD_SOURCE_FILTER_EPPT_TEXT_HXX

#include "epptbase.hxx"
#include "epptdef.hxx"

#include <rtl/textenc.h>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <editeng/svxenum.hxx>
#include <vector>
#include <memory>

namespace com { namespace sun { namespace star {
namespace awt { struct FontDescriptor; }
namespace beans { class XPropertyState; }
namespace text { class XTextRange; class XTextContent; class XSimpleText; }
namespace style { struct TabStop; }
}}}

struct SOParagraph
{
    bool                bExtendedParameters;
    sal_uInt32              nParaFlags;
    sal_Int16               nBulletFlags;
    OUString                sPrefix;
    OUString                sSuffix;
    OUString                sGraphicUrl;            // String to a graphic
    Size                    aBuGraSize;
    sal_uInt32              nNumberingType;         // this is actually a SvxEnum
    sal_uInt32              nHorzAdjust;
    sal_uInt32              nBulletColor;
    sal_Int32               nBulletOfs;
    sal_Int16               nStartWith;             // start of numbering
    sal_Int16               nTextOfs;
    sal_Int16               nBulletRealSize;        // scale in percent
    sal_Int16               nDepth;                 // actual depth
    sal_Unicode             cBulletId;              // if Numbering Type == CharSpecial
    css::awt::FontDescriptor       aFontDesc;

    bool                    bExtendedBulletsUsed;
    sal_uInt16              nBulletId;
    sal_uInt32              nMappedNumType;
    bool                    bNumberingIsNumber;

    SOParagraph()
        : bExtendedParameters(false)
        , nParaFlags(0)
        , nBulletFlags(0)
        , nNumberingType(SVX_NUM_NUMBER_NONE)
        , nHorzAdjust(0)
        , nBulletColor(0)
        , nBulletOfs(0)
        , nStartWith(0)
        , nTextOfs(0)
        , nBulletRealSize(0)
        , nDepth(0)
        , cBulletId(0)
        , bExtendedBulletsUsed(false)
        , nBulletId(0xffff)
        , nMappedNumType(0)
        , bNumberingIsNumber(true)
    {
    }
};

class PropStateValue : public PropValue
{
public:
    PropStateValue()
        : PropValue()
        , ePropState(css::beans::PropertyState_AMBIGUOUS_VALUE)
    {
    }
protected:
    css::beans::PropertyState ePropState;
    css::uno::Reference < css::beans::XPropertyState > mXPropState;

    bool    ImplGetPropertyValue( const OUString& rString, bool bGetPropertyState = true );
};

struct FieldEntry
{
    sal_uInt32  nFieldType;
    sal_uInt32  nFieldStartPos;
    sal_uInt32  nFieldEndPos;
    OUString    aRepresentation;
    OUString    aFieldUrl;

    FieldEntry( sal_uInt32 nType, sal_uInt32 nStart, sal_uInt32 nEnd )
    {
        nFieldType = nType;
        nFieldStartPos = nStart;
        nFieldEndPos = nEnd;
    }
};

class PortionObj : public PropStateValue
{

    friend class ParagraphObj;

    protected:

        void            ImplClear();
        void            ImplConstruct( const PortionObj& rPortionObj );
        static sal_uInt32 ImplGetTextField( css::uno::Reference< css::text::XTextRange > & rXTextRangeRef,
                            const css::uno::Reference< css::beans::XPropertySet > & rXPropSetRef, OUString& rURL );
        sal_uInt32      ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition );
        void            ImplGetPortionValues( FontCollection& rFontCollection, bool bGetPropStateValue = false );

    public:

        css::beans::PropertyState  meCharColor;
        css::beans::PropertyState  meCharHeight;
        css::beans::PropertyState  meFontName;
        css::beans::PropertyState  meAsianOrComplexFont;
        css::beans::PropertyState  meCharEscapement;
        css::lang::Locale          meCharLocale;
        sal_uInt16      mnCharAttrHard;

        sal_uInt32      mnCharColor;
        sal_uInt16      mnCharAttr;
        sal_uInt16      mnCharHeight;
        sal_uInt16      mnFont;
        sal_uInt16      mnAsianOrComplexFont;
        sal_Int16       mnCharEscapement;

        sal_uInt32      mnTextSize;
        bool        mbLastPortion;

        sal_uInt16*     mpText;
        FieldEntry*     mpFieldEntry;

                        PortionObj( css::uno::Reference< css::text::XTextRange > & rXTextRangeRef,
                                        bool bLast, FontCollection& rFontCollection );
                        PortionObj( const css::uno::Reference< css::beans::XPropertySet > & rXPropSetRef,
                                        FontCollection& rFontCollection );
                        PortionObj( const PortionObj& rPortionObj );
                        ~PortionObj();

        void            Write( SvStream* pStrm, bool bLast );
        sal_uInt32      Count() const { return mnTextSize; };

        PortionObj&     operator=( const PortionObj& rPortionObj );
};

struct ParaFlags
{
    bool    bFirstParagraph : 1;
    bool    bLastParagraph  : 1;

                    ParaFlags() { bFirstParagraph = true; bLastParagraph = false; };
};

class ParagraphObj : public PropStateValue, public SOParagraph
{
    friend class TextObj;
    friend struct PPTExParaSheet;

    std::vector<std::unique_ptr<PortionObj> > mvPortions;

    protected:

        void            ImplConstruct( const ParagraphObj& rParagraphObj );
        void            ImplClear();
        sal_uInt32      ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition );
        void            ImplGetParagraphValues( PPTExBulletProvider* pBuProv, bool bGetPropStateValue = false );
        void            ImplGetNumberingLevel( PPTExBulletProvider* pBuProv, sal_Int16 nDepth, bool bIsBullet, bool bGetPropStateValue = false );

    public:

        css::uno::Sequence< css::style::TabStop > maTabStop;

        sal_uInt32          mnTextSize;

        bool                mbIsBullet;
        bool                mbFirstParagraph;
        bool                mbLastParagraph;

        css::beans::PropertyState  meBullet;
        css::beans::PropertyState  meTextAdjust;
        css::beans::PropertyState  meLineSpacing;
        css::beans::PropertyState  meLineSpacingTop;
        css::beans::PropertyState  meLineSpacingBottom;
        css::beans::PropertyState  meForbiddenRules;
        css::beans::PropertyState  meParagraphPunctation;
        css::beans::PropertyState  meBiDi;

        sal_uInt16                              mnTextAdjust;
        sal_Int16                               mnLineSpacing;
        bool                                mbFixedLineSpacing;
        sal_Int16                               mnLineSpacingTop;
        sal_Int16                               mnLineSpacingBottom;
        bool                                mbForbiddenRules;
        bool                                mbParagraphPunctation;
        sal_uInt16                              mnBiDi;

                        ParagraphObj( css::uno::Reference< css::text::XTextContent > & rXTextContentRef,
                            ParaFlags, FontCollection& rFontCollection,
                                PPTExBulletProvider& rBuProv );
                        ParagraphObj( const ParagraphObj& rParargraphObj );
                        ParagraphObj( const css::uno::Reference< css::beans::XPropertySet > & rXPropSetRef,
                                      PPTExBulletProvider* pBuProv );

    bool empty() const { return mvPortions.empty(); }

    const PortionObj& front() const { return *mvPortions.front().get(); }

    std::vector<std::unique_ptr<PortionObj> >::const_iterator begin() const { return mvPortions.begin(); }
    std::vector<std::unique_ptr<PortionObj> >::const_iterator end() const { return mvPortions.end(); }

        void            CalculateGraphicBulletSize( sal_uInt16 nFontHeight );
                        ~ParagraphObj();

        void            Write( SvStream* pStrm );
        sal_uInt32      CharacterCount() const { return mnTextSize; };

        ParagraphObj&   operator=( const ParagraphObj& rParagraphObj );
};

struct ImplTextObj;

class TextObj
{
    std::shared_ptr<ImplTextObj>    mpImplTextObj;
    void            ImplCalculateTextPositions();

public:
    TextObj( css::uno::Reference< css::text::XSimpleText > &
            rXText, int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rBuProv );

    ParagraphObj*   GetParagraph(int idx);
    sal_uInt32      ParagraphCount() const;
    sal_uInt32      Count() const;
    int             GetInstance() const;
    bool            HasExtendedBullets();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
