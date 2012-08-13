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

#ifndef EPPT_TEXT_HXX
#define EPPT_TEXT_HXX

#include "epptbase.hxx"
#include "epptdef.hxx"

#include <rtl/textenc.h>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/lang/Locale.hpp>

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
    String                  sPrefix;
    String                  sSuffix;
    String                  sGraphicUrl;            // String auf eine Graphic
    Size                    aBuGraSize;
    sal_uInt32              nNumberingType;         // in wirlichkeit ist dies ein SvxEnum
    sal_uInt32              nHorzAdjust;
    sal_uInt32              nBulletColor;
    sal_Int32               nBulletOfs;
    sal_Int16               nStartWith;             // Start der nummerierung
    sal_Int16               nTextOfs;
    sal_Int16               nBulletRealSize;        // GroessenVerhaeltnis in Proz
    sal_Int16               nDepth;                 // aktuelle tiefe
    sal_Unicode             cBulletId;              // wenn Numbering Type == CharSpecial
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

        sal_Bool    ImplGetPropertyValue( const String& rString, sal_Bool bGetPropertyState = sal_True );

};

struct FieldEntry
{
    sal_uInt32  nFieldType;
    sal_uInt32  nFieldStartPos;
    sal_uInt32  nFieldEndPos;
    String      aRepresentation;
    String      aFieldUrl;

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
                            const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & rXPropSetRef, String& rURL );
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
        sal_uInt32          Count() const { return mnTextSize; };

        ParagraphObj&   operator=( const ParagraphObj& rParagraphObj );
};

struct ImplTextObj
{
    sal_uInt32      mnRefCount;
    sal_uInt32      mnTextSize;
    int             mnInstance;
    std::vector<ParagraphObj*> maList;
    sal_Bool        mbHasExtendedBullets;
    sal_Bool        mbFixedCellHeightUsed;

                    ImplTextObj( int nInstance );
                    ~ImplTextObj();
};

class TextObj
{
        ImplTextObj*    mpImplTextObj;
        void            ImplCalculateTextPositions();

    public :
                        TextObj( ::com::sun::star::uno::Reference< ::com::sun::star::text::XSimpleText > &
                                    rXText, int nInstance, FontCollection& rFontCollection, PPTExBulletProvider& rBuProv );
                        TextObj( const TextObj& rTextObj );
                        ~TextObj();

        ParagraphObj*   GetParagraph(int idx) { return mpImplTextObj->maList[idx]; };
        sal_uInt32      ParagraphCount() const { return mpImplTextObj->maList.size(); };
        sal_uInt32      Count() const { return mpImplTextObj->mnTextSize; };
        int             GetInstance() const { return mpImplTextObj->mnInstance; };
        sal_Bool        HasExtendedBullets(){ return mpImplTextObj->mbHasExtendedBullets; };
        void            WriteTextSpecInfo( SvStream* pStrm );

        TextObj&        operator=( TextObj& rTextObj );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
