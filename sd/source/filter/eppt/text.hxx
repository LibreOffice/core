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

#ifndef EPPT_TEXT_HXX
#define EPPT_TEXT_HXX

#include "epptbase.hxx"
#include "epptdef.hxx"

#include <rtl/textenc.h>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <boost/shared_ptr.hpp>

namespace com { namespace sun { namespace star {
namespace awt { struct FontDescriptor; }
namespace beans { class XPropertyState; }
namespace text { class XTextRange; class XTextContent; class XSimpleText; }
namespace style { struct TabStop; }
}}}

struct SOParagraph
{
    sal_Bool                bExtendedParameters;
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
    ::com::sun::star::awt::FontDescriptor       aFontDesc;

    sal_Bool                bExtendedBulletsUsed;
    sal_uInt16              nBulletId;
    sal_uInt32              nMappedNumType;
    sal_Bool                bNumberingIsNumber;

    SOParagraph()
    {
        nDepth = 0;
        bExtendedParameters = sal_False;
        nParaFlags = 0;
        nBulletFlags = 0;
        nBulletOfs = 0;
        nTextOfs = 0;
        bExtendedBulletsUsed = sal_False;
        nBulletId = 0xffff;
        bNumberingIsNumber = sal_True;
    };
};

class PropStateValue : public PropValue
{
    protected :

        ::com::sun::star::beans::PropertyState                  ePropState;
        ::com::sun::star::uno::Reference
            < ::com::sun::star::beans::XPropertyState >         mXPropState;

        sal_Bool    ImplGetPropertyValue( const OUString& rString, sal_Bool bGetPropertyState = sal_True );

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

    protected :

        void            ImplClear();
        void            ImplConstruct( const PortionObj& rPortionObj );
        sal_uInt32      ImplGetTextField( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & rXTextRangeRef,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSetRef, OUString& rURL );
        sal_uInt32      ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition );
        void            ImplGetPortionValues( FontCollection& rFontCollection, sal_Bool bGetPropStateValue = sal_False );

    public :

        ::com::sun::star::beans::PropertyState  meCharColor;
        ::com::sun::star::beans::PropertyState  meCharHeight;
        ::com::sun::star::beans::PropertyState  meFontName;
        ::com::sun::star::beans::PropertyState  meAsianOrComplexFont;
        ::com::sun::star::beans::PropertyState  meCharEscapement;
        ::com::sun::star::lang::Locale          meCharLocale;
        sal_uInt16      mnCharAttrHard;

        sal_uInt32      mnCharColor;
        sal_uInt16      mnCharAttr;
        sal_uInt16      mnCharHeight;
        sal_uInt16      mnFont;
        sal_uInt16      mnAsianOrComplexFont;
        sal_Int16       mnCharEscapement;

        sal_uInt32      mnTextSize;
        sal_Bool        mbLastPortion;

        sal_uInt16*     mpText;
        FieldEntry*     mpFieldEntry;

                        PortionObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > & rXTextRangeRef,
                                        sal_Bool bLast, FontCollection& rFontCollection );
                        PortionObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSetRef,
                                        FontCollection& rFontCollection );
                        PortionObj( const PortionObj& rPortionObj );
                        ~PortionObj();

        void            Write( SvStream* pStrm, sal_Bool bLast );
        sal_uInt32      Count() const { return mnTextSize; };

        PortionObj&     operator=( const PortionObj& rPortionObj );
};

struct ParaFlags
{
    sal_Bool    bFirstParagraph : 1;
    sal_Bool    bLastParagraph  : 1;

                    ParaFlags() { bFirstParagraph = sal_True; bLastParagraph = sal_False; };
};

class ParagraphObj : public std::vector<PortionObj*>, public PropStateValue, public SOParagraph
{
    friend class TextObj;
    friend struct PPTExParaSheet;

        MapMode         maMapModeSrc;
        MapMode         maMapModeDest;

    protected :

        void            ImplConstruct( const ParagraphObj& rParagraphObj );
        void            ImplClear();
        sal_uInt32      ImplCalculateTextPositions( sal_uInt32 nCurrentTextPosition );
        void            ImplGetParagraphValues( PPTExBulletProvider& rBuProv, sal_Bool bGetPropStateValue = sal_False );
        void            ImplGetNumberingLevel( PPTExBulletProvider& rBuProv, sal_Int16 nDepth, sal_Bool bIsBullet, sal_Bool bGetPropStateValue = sal_False );

    public :

        ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop > maTabStop;

        sal_uInt32          mnTextSize;

        sal_Bool            mbIsBullet;
        sal_Bool            mbFirstParagraph;
        sal_Bool            mbLastParagraph;

        ::com::sun::star::beans::PropertyState  meBullet;
        ::com::sun::star::beans::PropertyState  meTextAdjust;
        ::com::sun::star::beans::PropertyState  meLineSpacing;
        ::com::sun::star::beans::PropertyState  meLineSpacingTop;
        ::com::sun::star::beans::PropertyState  meLineSpacingBottom;
        ::com::sun::star::beans::PropertyState  meForbiddenRules;
        ::com::sun::star::beans::PropertyState  meParagraphPunctation;
        ::com::sun::star::beans::PropertyState  meBiDi;

        sal_uInt16                              mnTextAdjust;
        sal_Int16                               mnLineSpacing;
        sal_Bool                                mbFixedLineSpacing;
        sal_Int16                               mnLineSpacingTop;
        sal_Int16                               mnLineSpacingBottom;
        sal_Bool                                mbForbiddenRules;
        sal_Bool                                mbParagraphPunctation;
        sal_uInt16                              mnBiDi;

                        ParagraphObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent > & rXTextContentRef,
                            ParaFlags, FontCollection& rFontCollection,
                                PPTExBulletProvider& rBuProv );
                        ParagraphObj( const ParagraphObj& rParargraphObj );
                        ParagraphObj( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSetRef,
                                        PPTExBulletProvider& rBuProv );

        void            CalculateGraphicBulletSize( sal_uInt16 nFontHeight );
                        ~ParagraphObj();

        void            Write( SvStream* pStrm );
        sal_uInt32      CharacterCount() const { return mnTextSize; };

        ParagraphObj&   operator=( const ParagraphObj& rParagraphObj );
};

struct ImplTextObj;

class TextObj
{
    boost::shared_ptr<ImplTextObj>    mpImplTextObj;
    void            ImplCalculateTextPositions();

public :
    TextObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > &
            rXText, int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rBuProv );

    ParagraphObj*   GetParagraph(int idx);
    sal_uInt32      ParagraphCount() const;
    sal_uInt32      Count() const;
    int             GetInstance() const;
    sal_Bool        HasExtendedBullets();
    void            WriteTextSpecInfo( SvStream* pStrm );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
