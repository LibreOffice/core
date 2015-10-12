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

#ifndef INCLUDED_EDITENG_UNOTEXT_HXX
#define INCLUDED_EDITENG_UNOTEXT_HXX

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

#define WID_FONTDESC                OWN_ATTR_VALUE_START
#define WID_NUMLEVEL                OWN_ATTR_VALUE_START+1
#define WID_PORTIONTYPE             OWN_ATTR_VALUE_START+2
#define WID_NUMBERINGSTARTVALUE     OWN_ATTR_VALUE_START+3
#define WID_PARAISNUMBERINGRESTART  OWN_ATTR_VALUE_START+4

#define SVX_UNOEDIT_NUMBERING_PROPERTIE \
    {OUString(UNO_NAME_NUMBERING_RULES),        EE_PARA_NUMBULLET,  cppu::UnoType<css::container::XIndexReplace>::get(), 0, 0 }, \
    {OUString(UNO_NAME_NUMBERING),              EE_PARA_BULLETSTATE,cppu::UnoType<bool>::get(), 0, 0 }

#define SVX_UNOEDIT_OUTLINER_PROPERTIES \
    SVX_UNOEDIT_NUMBERING_PROPERTIE, \
    {OUString(UNO_NAME_NUMBERING_LEVEL),        WID_NUMLEVEL,       ::cppu::UnoType<sal_Int16>::get(), 0, 0 }, \
    {OUString("NumberingStartValue"),           WID_NUMBERINGSTARTVALUE, ::cppu::UnoType<sal_Int16>::get(), 0, 0 }, \
    {OUString("ParaIsNumberingRestart"),        WID_PARAISNUMBERINGRESTART, cppu::UnoType<bool>::get(), 0, 0 }

#define SVX_UNOEDIT_CHAR_PROPERTIES \
    { OUString(UNO_NAME_EDIT_CHAR_HEIGHT),      EE_CHAR_FONTHEIGHT, cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { OUString("CharScaleWidth"),               EE_CHAR_FONTWIDTH,  ::cppu::UnoType<sal_Int16>::get(),    0, 0 }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTNAME),    EE_CHAR_FONTINFO,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_FAMILY_NAME },\
    { OUString(UNO_NAME_EDIT_CHAR_FONTSTYLENAME),EE_CHAR_FONTINFO,  ::cppu::UnoType<OUString>::get(),  0, MID_FONT_STYLE_NAME }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTFAMILY),  EE_CHAR_FONTINFO,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTCHARSET), EE_CHAR_FONTINFO,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTPITCH),   EE_CHAR_FONTINFO,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH }, \
    { OUString(UNO_NAME_EDIT_CHAR_POSTURE),     EE_CHAR_ITALIC,     ::cppu::UnoType<com::sun::star::awt::FontSlant>::get(),0, MID_POSTURE }, \
    { OUString(UNO_NAME_EDIT_CHAR_WEIGHT),      EE_CHAR_WEIGHT,     cppu::UnoType<float>::get(),            0, MID_WEIGHT }, \
    { OUString(UNO_NAME_EDIT_CHAR_LOCALE),      EE_CHAR_LANGUAGE,   ::cppu::UnoType<com::sun::star::lang::Locale>::get(),0, MID_LANG_LOCALE }, \
    { OUString(UNO_NAME_EDIT_CHAR_COLOR),       EE_CHAR_COLOR,      ::cppu::UnoType<sal_Int32>::get(),        0, 0 }, \
    { OUString("CharBackColor"),                EE_CHAR_BKGCOLOR,   ::cppu::UnoType<sal_Int32>::get(),        0, 0 }, \
    { OUString("CharBackTransparent"),          EE_CHAR_BKGCOLOR,   ::cppu::UnoType<bool>::get(),             0, MID_GRAPHIC_TRANSPARENT }, \
    { OUString(UNO_NAME_EDIT_CHAR_ESCAPEMENT),  EE_CHAR_ESCAPEMENT, ::cppu::UnoType<sal_Int16>::get(),        0, MID_ESC }, \
    { OUString(UNO_NAME_EDIT_CHAR_UNDERLINE),   EE_CHAR_UNDERLINE,  ::cppu::UnoType<sal_Int16>::get(),        0, MID_TL_STYLE }, \
    { OUString("CharUnderlineColor"),           EE_CHAR_UNDERLINE,  ::cppu::UnoType<sal_Int32>::get(),        0, MID_TL_COLOR }, \
    { OUString("CharUnderlineHasColor"),        EE_CHAR_UNDERLINE,  cppu::UnoType<bool>::get(),                    0, MID_TL_HASCOLOR } , \
    { OUString(UNO_NAME_EDIT_CHAR_OVERLINE),    EE_CHAR_OVERLINE,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_TL_STYLE }, \
    { OUString("CharOverlineColor"),            EE_CHAR_OVERLINE,   ::cppu::UnoType<sal_Int32>::get(),        0, MID_TL_COLOR }, \
    { OUString("CharOverlineHasColor"),         EE_CHAR_OVERLINE,   cppu::UnoType<bool>::get(),                    0, MID_TL_HASCOLOR } , \
    { OUString(UNO_NAME_EDIT_CHAR_CROSSEDOUT),  EE_CHAR_STRIKEOUT,  cppu::UnoType<bool>::get(),                    0, MID_CROSSED_OUT }, \
    { OUString(UNO_NAME_EDIT_CHAR_STRIKEOUT),   EE_CHAR_STRIKEOUT,  ::cppu::UnoType<sal_Int16>::get(),        0, MID_CROSS_OUT}, \
    { OUString(UNO_NAME_EDIT_CHAR_CASEMAP),     EE_CHAR_CASEMAP,    ::cppu::UnoType<sal_Int16>::get(),        0, 0 }, \
    { OUString(UNO_NAME_EDIT_CHAR_SHADOWED),    EE_CHAR_SHADOW,     cppu::UnoType<bool>::get(),                    0, 0 }, \
    { OUString("CharContoured"),                EE_CHAR_OUTLINE,    cppu::UnoType<bool>::get(),                    0, 0 }, \
    { OUString("CharEscapementHeight"),         EE_CHAR_ESCAPEMENT, cppu::UnoType<sal_Int8>::get(),         0, MID_ESC_HEIGHT },\
    { OUString("CharAutoKerning"),              EE_CHAR_PAIRKERNING,cppu::UnoType<bool>::get(),                    0, 0 } , \
    { OUString("CharKerning"),                  EE_CHAR_KERNING,    ::cppu::UnoType<sal_Int16>::get()  ,      0, 0 }, \
    { OUString("CharWordMode"),                 EE_CHAR_WLM,        cppu::UnoType<bool>::get(),                    0, 0 }, \
    { OUString("CharEmphasis"),                 EE_CHAR_EMPHASISMARK, ::cppu::UnoType<sal_Int16>::get(),       0, MID_EMPHASIS},\
    { OUString(UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN),        EE_CHAR_FONTHEIGHT_CJK, cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN),      EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_FAMILY_NAME },\
    { OUString(UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN), EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_STYLE_NAME }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN),    EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN),   EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN),     EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH }, \
    { OUString(UNO_NAME_EDIT_CHAR_POSTURE_ASIAN),       EE_CHAR_ITALIC_CJK,     ::cppu::UnoType<com::sun::star::awt::FontSlant>::get(),0, MID_POSTURE }, \
    { OUString(UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN),        EE_CHAR_WEIGHT_CJK,     cppu::UnoType<float>::get(),            0, MID_WEIGHT }, \
    { OUString(UNO_NAME_EDIT_CHAR_LOCALE_ASIAN),        EE_CHAR_LANGUAGE_CJK,   ::cppu::UnoType<com::sun::star::lang::Locale>::get(),0, MID_LANG_LOCALE }, \
    { OUString(UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX),      EE_CHAR_FONTHEIGHT_CTL, cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX),    EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_FAMILY_NAME },\
    { OUString(UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX),EE_CHAR_FONTINFO_CTL,  ::cppu::UnoType<OUString>::get(),  0, MID_FONT_STYLE_NAME }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX),  EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX), EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET }, \
    { OUString(UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX),   EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH }, \
    { OUString(UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX),     EE_CHAR_ITALIC_CTL,     ::cppu::UnoType<com::sun::star::awt::FontSlant>::get(),0, MID_POSTURE }, \
    { OUString(UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX),      EE_CHAR_WEIGHT_CTL,     cppu::UnoType<float>::get(),            0, MID_WEIGHT }, \
    { OUString(UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX),      EE_CHAR_LANGUAGE_CTL,   ::cppu::UnoType<com::sun::star::lang::Locale>::get(),0, MID_LANG_LOCALE }, \
    { OUString("CharRelief"),                           EE_CHAR_RELIEF,         ::cppu::UnoType<sal_Int16>::get(),    0, MID_RELIEF }, \
    { OUString("CharInteropGrabBag"),                   EE_CHAR_GRABBAG,        cppu::UnoType<css::uno::Sequence<css::beans::PropertyValue >>::get(), 0, 0}


#define SVX_UNOEDIT_FONT_PROPERTIES \
    {OUString(UNO_NAME_EDIT_FONT_DESCRIPTOR),   WID_FONTDESC,       cppu::UnoType<css::awt::FontDescriptor>::get(),    0, MID_FONT_FAMILY_NAME }

#define SVX_UNOEDIT_PARA_PROPERTIES \
    {OUString(UNO_NAME_EDIT_PARA_ADJUST),       EE_PARA_JUST,               ::cppu::UnoType<sal_Int16>::get(),            0, MID_PARA_ADJUST }, \
    {OUString(UNO_NAME_EDIT_PARA_BMARGIN),      EE_PARA_ULSPACE,            ::cppu::UnoType<sal_Int32>::get(),            0, MID_LO_MARGIN|SFX_METRIC_ITEM }, \
    {OUString(UNO_NAME_EDIT_PARA_IS_HYPHEN),    EE_PARA_HYPHENATE,          cppu::UnoType<bool>::get(),                0, 0 }, \
    {OUString(UNO_NAME_EDIT_PARA_LASTLINEADJ),  EE_PARA_JUST,               ::cppu::UnoType<sal_Int16>::get(),            0, MID_LAST_LINE_ADJUST }, \
    {OUString(UNO_NAME_EDIT_PARA_LMARGIN),      EE_PARA_LRSPACE,            ::cppu::UnoType<sal_Int32>::get(),            0, MID_TXT_LMARGIN|SFX_METRIC_ITEM }, \
    {OUString(UNO_NAME_EDIT_PARA_LINESPACING),  EE_PARA_SBL,                cppu::UnoType<css::style::LineSpacing>::get(),     0, CONVERT_TWIPS}, \
    {OUString(UNO_NAME_EDIT_PARA_RMARGIN),      EE_PARA_LRSPACE,            ::cppu::UnoType<sal_Int32>::get(),            0, MID_R_MARGIN|SFX_METRIC_ITEM }, \
    {OUString(UNO_NAME_EDIT_PARA_TAPSTOPS),     EE_PARA_TABS,               cppu::UnoType<css::uno::Sequence< ::com::sun::star::style::TabStop >>::get(), 0, 0 }, \
    {OUString(UNO_NAME_EDIT_PARA_TMARGIN),      EE_PARA_ULSPACE,            ::cppu::UnoType<sal_Int32>::get(),            0, MID_UP_MARGIN|SFX_METRIC_ITEM },\
    {OUString(UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT),     EE_PARA_LRSPACE,       ::cppu::UnoType<sal_Int32>::get(),            0, MID_FIRST_LINE_INDENT|SFX_METRIC_ITEM}, \
    {OUString(UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION),EE_PARA_HANGINGPUNCTUATION,    cppu::UnoType<bool>::get(),                0 ,0 }, \
    {OUString(UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE), EE_PARA_ASIANCJKSPACING,   cppu::UnoType<bool>::get(),                0 ,0 }, \
    {OUString(UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES),    EE_PARA_FORBIDDENRULES,        cppu::UnoType<bool>::get(),                0 ,0 },\
    {OUString("WritingMode"),                   EE_PARA_WRITINGDIR,         ::cppu::UnoType<sal_Int16>::get(),            0, 0 }

class SvxEditSource;
class SvxTextForwarder;
class SvxFieldItem;
class SvxTextEditSource;
class SvxFieldData;
struct ESelection;

EDITENG_DLLPUBLIC void GetSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw();
EDITENG_DLLPUBLIC void CheckSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw();


// This class implements a SvxEditSource and SvxTextForwarder and does
// nothing otherwise


class SvxDummyTextSource : public SvxEditSource, public SvxTextForwarder
{
public:

    // SvxEditSource
    virtual ~SvxDummyTextSource();
    virtual SvxEditSource*          Clone() const override;
    virtual SvxTextForwarder*       GetTextForwarder() override;
    virtual void                    UpdateData() override;

    // SvxTextForwarder
    virtual sal_Int32       GetParagraphCount() const override;
    virtual sal_Int32       GetTextLen( sal_Int32 nParagraph ) const override;
    virtual OUString        GetText( const ESelection& rSel ) const override;
    virtual SfxItemSet      GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs_All ) const override;
    virtual SfxItemSet      GetParaAttribs( sal_Int32 nPara ) const override;
    virtual void            SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) override;
    virtual void            RemoveAttribs( const ESelection& rSelection, bool bRemoveParaAttribs, sal_uInt16 nWhich ) override;
    virtual void            GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const override;

    SfxItemState            GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const override;
    SfxItemState            GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const override;

    virtual SfxItemPool*    GetPool() const override;

    virtual void            QuickInsertText( const OUString& rText, const ESelection& rSel ) override;
    virtual void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) override;
    virtual void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) override;
    virtual void            QuickInsertLineBreak( const ESelection& rSel ) override;

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, Color*& rpTxtColor, Color*& rpFldColor ) override;
    virtual void            FieldClicked( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos ) override;

    virtual bool            IsValid() const override;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const override;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const override;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const override;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const override;
    virtual Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const override;
    virtual Rectangle       GetParaBounds( sal_Int32 nPara ) const override;
    virtual MapMode         GetMapMode() const override;
    virtual OutputDevice*   GetRefDevice() const override;
    virtual bool            GetIndexAtPoint( const Point&, sal_Int32& nPara, sal_Int32& nIndex ) const override;
    virtual bool            GetWordIndices( sal_Int32 nPara, sal_Int32 nIndex, sal_Int32& nStart, sal_Int32& nEnd ) const override;
    virtual bool            GetAttributeRun( sal_Int32& nStartIndex, sal_Int32& nEndIndex, sal_Int32 nPara, sal_Int32 nIndex, bool bInCell = false ) const override;
    virtual sal_Int32       GetLineCount( sal_Int32 nPara ) const override;
    virtual sal_Int32       GetLineLen( sal_Int32 nPara, sal_Int32 nLine ) const override;
    virtual void            GetLineBoundaries( /*out*/sal_Int32 &rStart, /*out*/sal_Int32 &rEnd, sal_Int32 nParagraph, sal_Int32 nLine ) const override;
    virtual sal_Int32       GetLineNumberAtIndex( sal_Int32 nPara, sal_Int32 nIndex ) const override;
    virtual bool            Delete( const ESelection& ) override;
    virtual bool            InsertText( const OUString&, const ESelection& ) override;
    virtual bool            QuickFormatDoc( bool bFull = false ) override;
    virtual sal_Int16       GetDepth( sal_Int32 nPara ) const override;
    virtual bool            SetDepth( sal_Int32 nPara, sal_Int16 nNewDepth ) override;

    virtual const SfxItemSet*   GetEmptyItemSetPtr() override;

    // implementation functions for XParagraphAppend and XTextPortionAppend
    virtual void        AppendParagraph() override;
    virtual sal_Int32   AppendTextPortion( sal_Int32 nPara, const OUString &rText, const SfxItemSet &rSet ) override;
    //XTextCopy
    virtual void        CopyText(const SvxTextForwarder& rSource) override;
};

namespace accessibility
{
    class AccessibleEditableTextPara;
}


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

    void SAL_CALL _setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Any SAL_CALL _getPropertyValue( const OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    void SAL_CALL _setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues, sal_Int32 nPara = -1 ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL _getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, sal_Int32 nPara = -1 ) throw (::com::sun::star::uno::RuntimeException);

    ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const SfxItemPropertySimpleEntry* pMap, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL _getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName, sal_Int32 nPara = -1  ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    // returns true if property found or false if unknown property
    static bool _getOnePropertyStates(const SfxItemSet* pSet, const SfxItemPropertySimpleEntry* pMap, ::com::sun::star::beans::PropertyState& rState);

    void SAL_CALL _setPropertyToDefault( const OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    void _setPropertyToDefault( SvxTextForwarder* pForwarder, const SfxItemPropertySimpleEntry* pMap, sal_Int32 nPara ) throw( ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException );
    void SetEditSource( SvxEditSource* _pEditSource ) throw();

    void getPropertyValue( const SfxItemPropertySimpleEntry* pMap, com::sun::star::uno::Any& rAny, const SfxItemSet& rSet )
        throw(css::beans::UnknownPropertyException, css::uno::RuntimeException);
    void setPropertyValue( const SfxItemPropertySimpleEntry* pMap, const com::sun::star::uno::Any& rValue, const ESelection& rSelection, const SfxItemSet& rOldSet, SfxItemSet& rNewSet ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException );

    SvxUnoTextRangeBase( const SvxItemPropertySet* _pSet ) throw();
    SvxUnoTextRangeBase( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet ) throw();
    SvxUnoTextRangeBase( const SvxUnoTextRangeBase& rRange ) throw();
    virtual ~SvxUnoTextRangeBase() throw();

public:
    // Internal
    const ESelection&       GetSelection() const throw() { CheckSelection( const_cast<SvxUnoTextRangeBase*>(this)->maSelection, mpEditSource->GetTextForwarder() ); return maSelection; };
    void                    SetSelection( const ESelection& rSelection ) throw();

    void            CollapseToStart() throw();
    void            CollapseToEnd() throw();
    bool            IsCollapsed() throw();
    bool            GoLeft(sal_Int16 nCount, bool Expand) throw();
    bool            GoRight(sal_Int16 nCount, bool Expand) throw();
    void            GotoStart(bool Expand) throw();
    void            GotoEnd(bool Expand) throw();

    //const SfxItemPropertyMapEntry*   getPropertyMapEntries() const throw() { return maPropSet.getPropertyMapEntries(); }
    const SvxItemPropertySet*   getPropertySet() const throw() { return mpPropSet; }
    SvxEditSource*              GetEditSource() const throw() { return mpEditSource; }

    static bool SetPropertyValueHelper( const SfxItemSet& rOldSet, const SfxItemPropertySimpleEntry* pMap, const ::com::sun::star::uno::Any& aValue, SfxItemSet& rNewSet, const ESelection* pSelection = NULL, SvxEditSource* pEditSource = NULL );
    static bool GetPropertyValueHelper(  SfxItemSet& rSet, const SfxItemPropertySimpleEntry* pMap, ::com::sun::star::uno::Any& aAny, const ESelection* pSelection = NULL,  SvxEditSource* pEditSource = NULL  ) throw( ::com::sun::star::uno::RuntimeException );

    void attachField( const SvxFieldData* pData ) throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextRangeBase )

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XMultiPropertyStates
    //virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setAllPropertiesToDefault()
        throw (::com::sun::star::uno::RuntimeException,
               std::exception) override;
    virtual void SAL_CALL setPropertiesToDefault( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyDefaults( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XTextRangeCompare
    virtual ::sal_Int16 SAL_CALL compareRegionStarts( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR2 ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::sal_Int16 SAL_CALL compareRegionEnds( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR1, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xR2 ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static( );
};



class SvxUnoTextBase;
class EDITENG_DLLPUBLIC SvxUnoTextRange : public SvxUnoTextRangeBase,
                        public ::com::sun::star::lang::XTypeProvider,
                        public ::cppu::OWeakAggObject
{
    friend class SvxUnoTextRangeEnumeration;
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText >   xParentText;
    bool mbPortion;

public:
    SvxUnoTextRange( const SvxUnoTextBase& rParent, bool bPortion = false ) throw();
    virtual ~SvxUnoTextRange() throw();

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
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

    SvxUnoTextBase( const SvxItemPropertySet* _pSet ) throw();
    SvxUnoTextBase( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet, ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > xParent ) throw();
    SvxUnoTextBase( const SvxUnoTextBase& rText ) throw();
    virtual ~SvxUnoTextBase() throw();

public:
    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextBase )

    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getStaticTypes() throw();

    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > createTextCursorBySelection( const ESelection& rSel );

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception);

    // ::com::sun::star::text::XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& aTextPosition ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertString( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL insertControlCharacter( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::text::XText
    virtual void SAL_CALL insertTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent, sal_Bool bAbsorb ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::text::XTextRangeMover
    virtual void SAL_CALL moveTextRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Int16 nParagraphs ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // com::sun::star::text::XParagraphAppend (new import API)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL finishParagraph( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL finishParagraphInsert( const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xInsertPosition ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // com::sun::star::text::XTextPortionAppend (new import API)
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL appendTextPortion( const OUString& Text, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL insertTextPortion( const OUString& Text, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& CharacterAndParagraphProperties, const com::sun::star::uno::Reference< com::sun::star::text::XTextRange>& rTextRange ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // com::sun::star::text::XTextCopy
    virtual void SAL_CALL copyText( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCopy >& xSource ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    static  ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static( );

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};


class EDITENG_DLLPUBLIC SvxUnoText : public SvxUnoTextBase,
                    public ::cppu::OWeakAggObject
{
public:
    SvxUnoText( const SvxItemPropertySet* _pSet ) throw();
    SvxUnoText( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet, ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > xParent ) throw();
    SvxUnoText( const SvxUnoText& rText ) throw();
    virtual ~SvxUnoText() throw();

    // Internal
    static const ::com::sun::star::uno::Sequence< sal_Int8 > & getUnoTunnelId() throw();
    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};



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
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};


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
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::text::XTextContent -> ::com::sun::star::lang::XComponent
    virtual void SAL_CALL attach( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xTextRange ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getAnchor(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XComponent
    virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL addEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& xListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL removeEventListener( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >& aListener ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XEnumerationAccess
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XEnumeration > SAL_CALL createEnumeration(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::container::XElementAccess
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL hasElements() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
};



class SvxUnoTextRangeEnumeration : public ::cppu::WeakAggImplHelper1< ::com::sun::star::container::XEnumeration >
{
private:
    SvxEditSource*      mpEditSource;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > mxParentText;
    const SvxUnoTextBase&    mrParentText;
    sal_Int32                mnParagraph;
    std::vector<sal_Int32>*  mpPortions;
    sal_uInt16               mnNextPortion;

public:
    SvxUnoTextRangeEnumeration( const SvxUnoTextBase& rText, sal_Int32 nPara ) throw();
    virtual ~SvxUnoTextRangeEnumeration() throw();

    // ::com::sun::star::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException, std::exception) override;
};



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
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL acquire() throw() override;
    virtual void SAL_CALL release() throw() override;

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > SAL_CALL getText() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL setString( const OUString& aString ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::text::XTextCursor -> ::com::sun::star::text::XTextRange
    virtual void SAL_CALL collapseToStart(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL collapseToEnd(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isCollapsed(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL gotoRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Bool bExpand ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception) override;

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException, std::exception) override;

};

EDITENG_DLLPUBLIC const SvxItemPropertySet* ImplGetSvxUnoOutlinerTextCursorSvxPropertySet();
EDITENG_DLLPUBLIC const SfxItemPropertyMapEntry* ImplGetSvxUnoOutlinerTextCursorPropertyMap();
EDITENG_DLLPUBLIC const SvxItemPropertySet* ImplGetSvxTextPortionSvxPropertySet();
EDITENG_DLLPUBLIC const SfxItemPropertyMapEntry* ImplGetSvxTextPortionPropertyMap();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
