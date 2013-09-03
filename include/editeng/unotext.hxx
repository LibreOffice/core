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

#ifndef _SVX_UNOTEXT_HXX
#define _SVX_UNOTEXT_HXX

#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextRangeMover.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include <com/sun/star/text/XTextField.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/weak.hxx>
#include <cppuhelper/weakagg.hxx>
#include <osl/diagnose.hxx>
#include <osl/mutex.hxx>
#include <comphelper/servicehelper.hxx>
#include <tools/link.hxx>
#include <svl/itemprop.hxx>
#include <svl/itemset.hxx>
#include <svl/itempool.hxx>
#include <svl/solar.hrc>
#include <editeng/editdata.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/memberids.hrc>

class SvxItemPropertySet;

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#define WID_FONTDESC                OWN_ATTR_VALUE_START
#define WID_NUMLEVEL                OWN_ATTR_VALUE_START+1
#define WID_PORTIONTYPE             OWN_ATTR_VALUE_START+2
#define WID_NUMBERINGSTARTVALUE     OWN_ATTR_VALUE_START+3
#define WID_PARAISNUMBERINGRESTART  OWN_ATTR_VALUE_START+4

#define SVX_UNOEDIT_NUMBERING_PROPERTIE \
    {MAP_CHAR_LEN(UNO_NAME_NUMBERING_RULES),        EE_PARA_NUMBULLET,  &::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace>*)0), 0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_NUMBERING),              EE_PARA_BULLETSTATE,&::getBooleanCppuType(), 0, 0 }

#define SVX_UNOEDIT_OUTLINER_PROPERTIES \
    SVX_UNOEDIT_NUMBERING_PROPERTIE, \
    {MAP_CHAR_LEN(UNO_NAME_NUMBERING_LEVEL),        WID_NUMLEVEL,       &::getCppuType((const sal_Int16*)0), 0, 0 }, \
    {MAP_CHAR_LEN("NumberingStartValue"),           WID_NUMBERINGSTARTVALUE, &::getCppuType((const sal_Int16*)0), 0, 0 }, \
    {MAP_CHAR_LEN("ParaIsNumberingRestart"),        WID_PARAISNUMBERINGRESTART, &::getBooleanCppuType(), 0, 0 }

#define SVX_UNOEDIT_CHAR_PROPERTIES \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT),      EE_CHAR_FONTHEIGHT, &::getCppuType((const float*)0),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { MAP_CHAR_LEN("CharScaleWidth"),               EE_CHAR_FONTWIDTH,      &::getCppuType((const sal_Int16*)0),    0, 0 }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME),    EE_CHAR_FONTINFO,   &::getCppuType((const OUString*)0),  0, MID_FONT_FAMILY_NAME },\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME),EE_CHAR_FONTINFO,  &::getCppuType((const OUString*)0),  0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY),  EE_CHAR_FONTINFO,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_FAMILY }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET), EE_CHAR_FONTINFO,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_CHAR_SET }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH),   EE_CHAR_FONTINFO,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_PITCH }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE),     EE_CHAR_ITALIC,     &::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT),      EE_CHAR_WEIGHT,     &::getCppuType((const float*)0),            0, MID_WEIGHT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_LOCALE),      EE_CHAR_LANGUAGE,   &::getCppuType((const ::com::sun::star::lang::Locale*)0),0, MID_LANG_LOCALE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_COLOR),       EE_CHAR_COLOR,      &::getCppuType((const sal_Int32*)0),        0, 0 }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_ESCAPEMENT),  EE_CHAR_ESCAPEMENT, &::getCppuType((const sal_Int16*)0),        0, MID_ESC }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_UNDERLINE),   EE_CHAR_UNDERLINE,  &::getCppuType((const sal_Int16*)0),        0, MID_TL_STYLE }, \
    { MAP_CHAR_LEN("CharUnderlineColor"),           EE_CHAR_UNDERLINE,  &::getCppuType((const sal_Int32*)0),        0, MID_TL_COLOR }, \
    { MAP_CHAR_LEN("CharUnderlineHasColor"),        EE_CHAR_UNDERLINE,  &::getBooleanCppuType(),                    0, MID_TL_HASCOLOR } , \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_OVERLINE),    EE_CHAR_OVERLINE,   &::getCppuType((const sal_Int16*)0),        0, MID_TL_STYLE }, \
    { MAP_CHAR_LEN("CharOverlineColor"),            EE_CHAR_OVERLINE,   &::getCppuType((const sal_Int32*)0),        0, MID_TL_COLOR }, \
    { MAP_CHAR_LEN("CharOverlineHasColor"),         EE_CHAR_OVERLINE,   &::getBooleanCppuType(),                    0, MID_TL_HASCOLOR } , \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_CROSSEDOUT),  EE_CHAR_STRIKEOUT,  &::getBooleanCppuType(),                    0, MID_CROSSED_OUT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_STRIKEOUT),   EE_CHAR_STRIKEOUT,  &::getCppuType((const sal_Int16*)0),        0, MID_CROSS_OUT}, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_SHADOWED),    EE_CHAR_SHADOW,     &::getBooleanCppuType(),                    0, 0 }, \
    { MAP_CHAR_LEN("CharContoured"),                EE_CHAR_OUTLINE,    &::getBooleanCppuType(),                    0, 0 }, \
    { MAP_CHAR_LEN("CharEscapementHeight"),         EE_CHAR_ESCAPEMENT, &::getCppuType((const sal_Int8*)0),         0, MID_ESC_HEIGHT },\
    { MAP_CHAR_LEN("CharAutoKerning"),              EE_CHAR_PAIRKERNING,&::getBooleanCppuType(),                    0, 0 } , \
    { MAP_CHAR_LEN("CharKerning"),                  EE_CHAR_KERNING,    &::getCppuType((const sal_Int16*)0)  ,      0, 0 }, \
    { MAP_CHAR_LEN("CharWordMode"),                 EE_CHAR_WLM,        &::getBooleanCppuType(),                    0, 0 }, \
    { MAP_CHAR_LEN("CharEmphasis"),                 EE_CHAR_EMPHASISMARK,&::getCppuType((const sal_Int16*)0),       0, MID_EMPHASIS},\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN),        EE_CHAR_FONTHEIGHT_CJK, &::getCppuType((const float*)0),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN),      EE_CHAR_FONTINFO_CJK,   &::getCppuType((const OUString*)0),  0, MID_FONT_FAMILY_NAME },\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN), EE_CHAR_FONTINFO_CJK,   &::getCppuType((const OUString*)0),  0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN),    EE_CHAR_FONTINFO_CJK,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_FAMILY }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN),   EE_CHAR_FONTINFO_CJK,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_CHAR_SET }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN),     EE_CHAR_FONTINFO_CJK,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_PITCH }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE_ASIAN),       EE_CHAR_ITALIC_CJK,     &::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN),        EE_CHAR_WEIGHT_CJK,     &::getCppuType((const float*)0),            0, MID_WEIGHT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_LOCALE_ASIAN),        EE_CHAR_LANGUAGE_CJK,   &::getCppuType((const ::com::sun::star::lang::Locale*)0),0, MID_LANG_LOCALE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX),      EE_CHAR_FONTHEIGHT_CTL, &::getCppuType((const float*)0),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX),    EE_CHAR_FONTINFO_CTL,   &::getCppuType((const OUString*)0),  0, MID_FONT_FAMILY_NAME },\
    {MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX),EE_CHAR_FONTINFO_CTL,   &::getCppuType((const OUString*)0),  0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX),  EE_CHAR_FONTINFO_CTL,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_FAMILY }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX), EE_CHAR_FONTINFO_CTL,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_CHAR_SET }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX),   EE_CHAR_FONTINFO_CTL,   &::getCppuType((const sal_Int16*)0),        0, MID_FONT_PITCH }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX),     EE_CHAR_ITALIC_CTL,     &::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX),      EE_CHAR_WEIGHT_CTL,     &::getCppuType((const float*)0),            0, MID_WEIGHT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX),      EE_CHAR_LANGUAGE_CTL,   &::getCppuType((const ::com::sun::star::lang::Locale*)0),0, MID_LANG_LOCALE }, \
    { MAP_CHAR_LEN("CharRelief"            ),               EE_CHAR_RELIEF,         &::getCppuType((const sal_Int16*)0),    0, MID_RELIEF }

#define SVX_UNOEDIT_FONT_PROPERTIES \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_FONT_DESCRIPTOR),   WID_FONTDESC,       &::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0),    0, MID_FONT_FAMILY_NAME }

#define SVX_UNOEDIT_PARA_PROPERTIES \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_ADJUST),       EE_PARA_JUST,               &::getCppuType((const sal_Int16*)0),            0, MID_PARA_ADJUST }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_BMARGIN),      EE_PARA_ULSPACE,            &::getCppuType((const sal_Int32*)0),            0, MID_LO_MARGIN|SFX_METRIC_ITEM }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_HYPHEN),    EE_PARA_HYPHENATE,          &::getBooleanCppuType(),                0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_LASTLINEADJ),  EE_PARA_JUST,               &::getCppuType((const sal_Int16*)0),            0, MID_LAST_LINE_ADJUST }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_LMARGIN),      EE_PARA_LRSPACE,            &::getCppuType((const sal_Int32*)0),            0, MID_TXT_LMARGIN|SFX_METRIC_ITEM }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_LINESPACING),  EE_PARA_SBL,                &::getCppuType((const ::com::sun::star::style::LineSpacing*)0),     0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_RMARGIN),      EE_PARA_LRSPACE,            &::getCppuType((const sal_Int32*)0),            0, MID_R_MARGIN|SFX_METRIC_ITEM }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_TAPSTOPS),     EE_PARA_TABS,               SEQTYPE(::getCppuType((const ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >*)0)), 0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_TMARGIN),      EE_PARA_ULSPACE,            &::getCppuType((const sal_Int32*)0),            0, MID_UP_MARGIN|SFX_METRIC_ITEM },\
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT),     EE_PARA_LRSPACE,       &::getCppuType((const sal_Int32*)0),            0, MID_FIRST_LINE_INDENT|SFX_METRIC_ITEM}, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION),EE_PARA_HANGINGPUNCTUATION,    &::getBooleanCppuType(),                0 ,0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE), EE_PARA_ASIANCJKSPACING,   &::getBooleanCppuType(),                0 ,0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES),    EE_PARA_FORBIDDENRULES,        &::getBooleanCppuType(),                0 ,0 },\
    {MAP_CHAR_LEN("WritingMode"),                   EE_PARA_WRITINGDIR,         &::getCppuType((const sal_Int16*)0),            0, 0 }

class SvxEditSource;
class SvxTextForwarder;
class SvxFieldItem;
class SvxTextEditSource;
class SvxFieldData;
struct ESelection;

EDITENG_DLLPUBLIC void GetSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw();
EDITENG_DLLPUBLIC void CheckSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw();

// ====================================================================
// This class implements a SvxEditSource and SvxTextForwarder and does
// nothing otherwise
// ====================================================================

class SvxDummyTextSource : public SvxEditSource, public SvxTextForwarder
{
public:

    // SvxEditSource
    virtual ~SvxDummyTextSource();
    virtual SvxEditSource*          Clone() const;
    virtual SvxTextForwarder*       GetTextForwarder();
    virtual void                    UpdateData();

    // SvxTextForwarder
    virtual sal_Int32       GetParagraphCount() const;
    virtual sal_uInt16      GetTextLen( sal_Int32 nParagraph ) const;
    virtual OUString        GetText( const ESelection& rSel ) const;
    virtual SfxItemSet      GetAttribs( const ESelection& rSel, sal_Bool bOnlyHardAttrib = 0 ) const;
    virtual SfxItemSet      GetParaAttribs( sal_Int32 nPara ) const;
    virtual void            SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet );
    virtual void            RemoveAttribs( const ESelection& rSelection, sal_Bool bRemoveParaAttribs, sal_uInt16 nWhich );
    virtual void            GetPortions( sal_Int32 nPara, std::vector<sal_uInt16>& rList ) const;

    sal_uInt16              GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const;
    sal_uInt16              GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const;

    virtual SfxItemPool*    GetPool() const;

    virtual void            QuickInsertText( const OUString& rText, const ESelection& rSel );
    virtual void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void            QuickInsertLineBreak( const ESelection& rSel );

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual void            FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, xub_StrLen nPos );

    virtual sal_Bool        IsValid() const;

    virtual void            SetNotifyHdl( const Link& );
    virtual LanguageType    GetLanguage( sal_Int32, sal_uInt16 ) const;
    virtual sal_uInt16      GetFieldCount( sal_Int32 nPara ) const;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_uInt16 nIndex ) const;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const;
    virtual MapMode         GetMapMode() const;
    virtual OutputDevice*   GetRefDevice() const;
    virtual sal_Bool        GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_uInt16& nIndex ) const;
    virtual sal_Bool        GetWordIndices( sal_Int32 nPara, sal_uInt16 nIndex, sal_uInt16& nStart, sal_uInt16& nEnd ) const;
    virtual sal_Bool        GetAttributeRun( sal_uInt16& nStartIndex, sal_uInt16& nEndIndex, sal_Int32 nPara, sal_uInt16 nIndex ) const;
    virtual sal_uInt16      GetLineCount( sal_Int32 nPara ) const;
    virtual sal_uInt16      GetLineLen( sal_Int32 nPara, sal_uInt16 nLine ) const;
    virtual void            GetLineBoundaries( /*out*/sal_uInt16 &rStart, /*out*/sal_uInt16 &rEnd, sal_Int32 nParagraph, sal_uInt16 nLine ) const;
    virtual sal_uInt16      GetLineNumberAtIndex( sal_Int32 nPara, sal_uInt16 nIndex ) const;
    virtual sal_Bool        Delete( const ESelection& );
    virtual sal_Bool        InsertText( const OUString&, const ESelection& );
    virtual sal_Bool        QuickFormatDoc( sal_Bool bFull=sal_False );
    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const;
    virtual sal_Bool        SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth );

    virtual const SfxItemSet*   GetEmptyItemSetPtr();

    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph();
    virtual sal_uInt16  AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet );
    //XTextCopy
    virtual void        CopyText(const SvxTextForwarder& rSource);
};

namespace accessibility
{
    class AccessibleEditableTextPara;
}

// ====================================================================
class EDITENG_DLLPUBLIC SvxUnoTextRangeBase : public ::com::sun::star::text::XTextRange,
                            public ::com::sun::star::beans::XPropertySet,
                            public ::com::sun::star::beans::XMultiPropertySet,
                            public ::com::sun::star::beans::XMultiPropertyStates,
                            public ::com::sun::star::beans::XPropertyState,
                            public ::com::sun::star::lang::XServiceInfo,
                            public ::com::sun::star::text::XTextRangeCompare,
                            public ::com::sun::star::lang::XUnoTunnel,
                            private osl::DebugBase<SvxUnoTextRangeBase>

{
    friend class SvxUnoTextRangeEnumeration;
    friend class ::accessibility::AccessibleEditableTextPara;

protected:
    SvxEditSource*          mpEditSource;
    ESelection              maSelection;
    const SvxItemPropertySet* mpPropSet;

    virtual void SAL_CALL _setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL _getPropertyValue( const OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues, sal_Int32 nPara = -1 ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL _getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, sal_Int32 nPara = -1 ) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const SfxItemPropertySimpleEntry* pMap, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL _getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName, sal_Int32 nPara = -1  ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    // returns true if property found or false if unknown property
    virtual sal_Bool _getOnePropertyStates(const SfxItemSet* pSet, const SfxItemPropertySimpleEntry* pMap, ::com::sun::star::beans::PropertyState& rState);

    virtual void SAL_CALL _setPropertyToDefault( const OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void _setPropertyToDefault( SvxTextForwarder* pForwarder, const SfxItemPropertySimpleEntry* pMap, sal_Int32 nPara ) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );
    void SetEditSource( SvxEditSource* _pEditSource ) throw();

    virtual void getPropertyValue( const SfxItemPropertySimpleEntry* pMap, com::sun::star::uno::Any& rAny, const SfxItemSet& rSet ) throw(::com::sun::star::beans::UnknownPropertyException );
    virtual void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const com::sun::star::uno::Any& rValue, const ESelection& rSelection, const SfxItemSet& rOldSet, SfxItemSet& rNewSet ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException );

public:
    SvxUnoTextRangeBase( const SvxItemPropertySet* _pSet ) throw();
    SvxUnoTextRangeBase( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet ) throw();
    SvxUnoTextRangeBase( const SvxUnoTextRangeBase& rRange ) throw();
    virtual ~SvxUnoTextRangeBase() throw();

    // Internal
    const ESelection&       GetSelection() const throw() { CheckSelection( ((SvxUnoTextRangeBase*)this)->maSelection, mpEditSource->GetTextForwarder() ); return maSelection; };
    void                    SetSelection( const ESelection& rSelection ) throw();

    virtual void            CollapseToStart(void) throw();
    virtual void            CollapseToEnd(void) throw();
    virtual sal_Bool        IsCollapsed(void) throw();
    virtual sal_Bool        GoLeft(sal_Int16 nCount, sal_Bool Expand) throw();
    virtual sal_Bool        GoRight(sal_Int16 nCount, sal_Bool Expand) throw();
    virtual void            GotoStart(sal_Bool Expand) throw();
    virtual void            GotoEnd(sal_Bool Expand) throw();

    //const SfxItemPropertyMapEntry*   getPropertyMapEntries() const throw() { return maPropSet.getPropertyMapEntries(); }
    const SvxItemPropertySet*   getPropertySet() const throw() { return mpPropSet; }
    SvxEditSource*              GetEditSource() const throw() { return mpEditSource; }

    static sal_Bool SetPropertyValueHelper( const SfxItemSet& rOldSet, const SfxItemPropertySimpleEntry* pMap, const ::com::sun::star::uno::Any& aValue, SfxItemSet& rNewSet, const ESelection* pSelection = NULL, SvxEditSource* pEditSource = NULL );
    static sal_Bool GetPropertyValueHelper(  SfxItemSet& rSet, const SfxItemPropertySimpleEntry* pMap, ::com::sun::star::uno::Any& aAny, const ESelection* pSelection = NULL,  SvxEditSource* pEditSource = NULL  ) throw( ::com::sun::star::uno::RuntimeException );

    void attachField( const SvxFieldData* pData ) throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextRangeBase )

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XMultiPropertyStates
    //virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAllPropertiesToDefault(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XTextRangeCompare
    virtual ::sal_Int16 SAL_CALL compareRegionStarts( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR2 ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::sal_Int16 SAL_CALL compareRegionEnds( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR2 ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static( ) SAL_THROW(());
};

// ====================================================================

class SvxUnoTextBase;
class EDITENG_DLLPUBLIC SvxUnoTextRange : public SvxUnoTextRangeBase,
                        public ::com::sun::star::lang::XTypeProvider,
                        public ::cppu::OWeakAggObject
{
    friend class SvxUnoTextRangeEnumeration;
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >   xParentText;
    sal_Bool mbPortion;

public:
    SvxUnoTextRange( const SvxUnoTextBase& rParent, sal_Bool bPortion = sal_False ) throw();
    virtual ~SvxUnoTextRange() throw();

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

class EDITENG_DLLPUBLIC SvxUnoTextBase  : public SvxUnoTextRangeBase,
                        public ::com::sun::star::text::XTextAppend,
                        public ::com::sun::star::text::XTextCopy,
                        public ::com::sun::star::container::XEnumerationAccess,
                        public ::com::sun::star::text::XTextRangeMover,
                        public ::com::sun::star::lang::XTypeProvider
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >   xParentText;

public:
    SvxUnoTextBase( ) throw();
    SvxUnoTextBase( const SvxItemPropertySet* _pSet ) throw();
    SvxUnoTextBase( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet, ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > xParent ) throw();
    SvxUnoTextBase( const SvxUnoTextBase& rText ) throw();
    virtual ~SvxUnoTextBase() throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextBase )

    ESelection InsertField( const SvxFieldItem& rField ) throw();
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getStaticTypes() throw();

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > createTextCursorBySelection( const ESelection& rSel );

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursor(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& aTextPosition ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertString( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertControlCharacter( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XText
    virtual void SAL_CALL insertTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent, sal_Bool bAbsorb ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XTextRangeMover
    virtual void SAL_CALL moveTextRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Int16 nParagraphs ) throw(::com::sun::star::uno::RuntimeException);

    // com::sun::star::text::XParagraphAppend (new import API)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL finishParagraph( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL finishParagraphInsert( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xInsertPosition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::text::XTextPortionAppend (new import API)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL appendTextPortion( const OUString& Text, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL insertTextPortion( const OUString& Text, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties, const com::sun::star::uno::Reference< com::sun::star::text::XTextRange>& rTextRange ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    // com::sun::star::text::XTextCopy
    virtual void SAL_CALL copyText( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCopy >& xSource ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static( ) SAL_THROW(());

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

// ====================================================================
class EDITENG_DLLPUBLIC SvxUnoText : public SvxUnoTextBase,
                    public ::cppu::OWeakAggObject
{
public:
    SvxUnoText( ) throw();
    SvxUnoText( const SvxItemPropertySet* _pSet ) throw();
    SvxUnoText( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet, ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > xParent ) throw();
    SvxUnoText( const SvxUnoText& rText ) throw();
    virtual ~SvxUnoText() throw();

    // Internal
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

// ====================================================================

class SvxUnoTextContentEnumeration : public ::cppu::WeakAggImplHelper1< ::com::sun::star::container::XEnumeration >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxParentText;
    SvxEditSource*          mpEditSource;
    sal_Int32               mnNextParagraph;
    const SvxUnoTextBase&   mrText;

public:
    SvxUnoTextContentEnumeration( const SvxUnoTextBase& _rText ) throw();
    virtual ~SvxUnoTextContentEnumeration() throw();

    // ::com::sun::star::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

// ====================================================================
#include <com/sun/star/text/XTextContent.hpp>

class SvxUnoTextContent : public SvxUnoTextRangeBase,
                          public ::com::sun::star::text::XTextContent,
                          public ::com::sun::star::container::XEnumerationAccess,
                          public ::com::sun::star::lang::XTypeProvider,
                          public ::cppu::OWeakAggObject
{
    friend class SvxUnoTextContentEnumeration;
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxParentText;
    sal_Int32               mnParagraph;
    const SvxUnoTextBase&   mrParentText;

    // for xComponent
    ::osl::Mutex maDisposeContainerMutex;
    ::cppu::OInterfaceContainerHelper maDisposeListeners;
    bool mbDisposing;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxUnoTextContent( const SvxUnoTextBase& rText, sal_Int32 nPara ) throw();
    SvxUnoTextContent( const SvxUnoTextContent& rContent ) throw();
    virtual ~SvxUnoTextContent() throw();

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XTextContent -> ::com::sun::star::lang::XComponent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

// ====================================================================

class SvxUnoTextRangeEnumeration : public ::cppu::WeakAggImplHelper1< ::com::sun::star::container::XEnumeration >
{
private:
    SvxEditSource*      mpEditSource;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxParentText;
    const SvxUnoTextBase&    mrParentText;
    sal_Int32                mnParagraph;
    std::vector<sal_uInt16>* mpPortions;
    sal_uInt16               mnNextPortion;

public:
    SvxUnoTextRangeEnumeration( const SvxUnoTextBase& rText, sal_Int32 nPara ) throw();
    virtual ~SvxUnoTextRangeEnumeration() throw();

    // ::com::sun::star::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

// ====================================================================

class EDITENG_DLLPUBLIC SvxUnoTextCursor : public SvxUnoTextRangeBase,
                         public ::com::sun::star::text::XTextCursor,
                         public ::com::sun::star::lang::XTypeProvider,
                         public ::cppu::OWeakAggObject
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxParentText;

public:
    SvxUnoTextCursor( const SvxUnoTextBase& rText ) throw();
    SvxUnoTextCursor( const SvxUnoTextCursor& rCursor ) throw();
    virtual ~SvxUnoTextCursor() throw();

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL acquire() throw();
    virtual void SAL_CALL release() throw();

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException);
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XTextCursor -> ::com::sun::star::text::XTextRange
    virtual void SAL_CALL collapseToStart(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL collapseToEnd(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isCollapsed(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL gotoRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

};

EDITENG_DLLPUBLIC const SvxItemPropertySet* ImplGetSvxUnoOutlinerTextCursorSvxPropertySet();
EDITENG_DLLPUBLIC const SfxItemPropertyMapEntry* ImplGetSvxUnoOutlinerTextCursorPropertyMap();
EDITENG_DLLPUBLIC const SvxItemPropertySet* ImplGetSvxTextPortionSvxPropertySet();
EDITENG_DLLPUBLIC const SfxItemPropertyMapEntry* ImplGetSvxTextPortionPropertyMap();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
