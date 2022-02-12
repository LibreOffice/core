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

#include <memory>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/text/XTextRangeMover.hpp>
#include <com/sun/star/text/XTextCursor.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/XPropertyState.hpp>
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#include <com/sun/star/text/XTextRangeCompare.hpp>
#include <com/sun/star/beans/XMultiPropertyStates.hpp>
#include <com/sun/star/text/XTextAppend.hpp>
#include <com/sun/star/text/XTextCopy.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/style/LineSpacing.hpp>
#include <com/sun/star/style/TabStop.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/weakagg.hxx>
#include <osl/diagnose.hxx>
#include <osl/mutex.hxx>
#include <comphelper/servicehelper.hxx>
#include <svl/itemset.hxx>
#include <svl/solar.hrc>
#include <editeng/editdata.hxx>
#include <editeng/unoedsrc.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/editengdllapi.h>
#include <editeng/memberids.h>

class SvxItemPropertySet;
struct SfxItemPropertyMapEntry;

#define WID_FONTDESC                OWN_ATTR_VALUE_START
#define WID_NUMLEVEL                OWN_ATTR_VALUE_START+1
#define WID_PORTIONTYPE             OWN_ATTR_VALUE_START+2
#define WID_NUMBERINGSTARTVALUE     OWN_ATTR_VALUE_START+3
#define WID_PARAISNUMBERINGRESTART  OWN_ATTR_VALUE_START+4

#define SVX_UNOEDIT_NUMBERING_PROPERTIE \
    {u"" UNO_NAME_NUMBERING_RULES,        EE_PARA_NUMBULLET,  cppu::UnoType<css::container::XIndexReplace>::get(), 0, 0 }, \
    {u"" UNO_NAME_NUMBERING,              EE_PARA_BULLETSTATE,cppu::UnoType<bool>::get(), 0, 0 }

#define SVX_UNOEDIT_OUTLINER_PROPERTIES \
    SVX_UNOEDIT_NUMBERING_PROPERTIE, \
    {u"" UNO_NAME_NUMBERING_LEVEL,        WID_NUMLEVEL,       ::cppu::UnoType<sal_Int16>::get(), 0, 0 }, \
    {u"NumberingStartValue",           WID_NUMBERINGSTARTVALUE, ::cppu::UnoType<sal_Int16>::get(), 0, 0 }, \
    {u"ParaIsNumberingRestart",        WID_PARAISNUMBERINGRESTART, cppu::UnoType<bool>::get(), 0, 0 }

#define SVX_UNOEDIT_CHAR_PROPERTIES \
    { u"" UNO_NAME_EDIT_CHAR_HEIGHT,      EE_CHAR_FONTHEIGHT, cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { u"CharScaleWidth",               EE_CHAR_FONTWIDTH,  ::cppu::UnoType<sal_Int16>::get(),    0, 0 }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTNAME,    EE_CHAR_FONTINFO,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_FAMILY_NAME },\
    { u"" UNO_NAME_EDIT_CHAR_FONTSTYLENAME,EE_CHAR_FONTINFO,  ::cppu::UnoType<OUString>::get(),  0, MID_FONT_STYLE_NAME }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTFAMILY,  EE_CHAR_FONTINFO,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTCHARSET, EE_CHAR_FONTINFO,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTPITCH,   EE_CHAR_FONTINFO,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH }, \
    { u"" UNO_NAME_EDIT_CHAR_POSTURE,     EE_CHAR_ITALIC,     ::cppu::UnoType<css::awt::FontSlant>::get(),0, MID_POSTURE }, \
    { u"" UNO_NAME_EDIT_CHAR_WEIGHT,      EE_CHAR_WEIGHT,     cppu::UnoType<float>::get(),            0, MID_WEIGHT }, \
    { u"" UNO_NAME_EDIT_CHAR_LOCALE,      EE_CHAR_LANGUAGE,   ::cppu::UnoType<css::lang::Locale>::get(),0, MID_LANG_LOCALE }, \
    { u"" UNO_NAME_EDIT_CHAR_COLOR,       EE_CHAR_COLOR,      ::cppu::UnoType<sal_Int32>::get(),        0, 0 }, \
    { u"" UNO_NAME_EDIT_CHAR_TRANSPARENCE,EE_CHAR_COLOR,      ::cppu::UnoType<sal_Int16>::get(),        0, MID_COLOR_ALPHA }, \
    { u"CharBackColor",                EE_CHAR_BKGCOLOR,   ::cppu::UnoType<sal_Int32>::get(),        0, 0 }, \
    { u"CharBackTransparent",          EE_CHAR_BKGCOLOR,   ::cppu::UnoType<bool>::get(),             0, MID_GRAPHIC_TRANSPARENT }, \
    { u"" UNO_NAME_EDIT_CHAR_ESCAPEMENT,  EE_CHAR_ESCAPEMENT, ::cppu::UnoType<sal_Int16>::get(),        0, MID_ESC }, \
    { u"" UNO_NAME_EDIT_CHAR_UNDERLINE,   EE_CHAR_UNDERLINE,  ::cppu::UnoType<sal_Int16>::get(),        0, MID_TL_STYLE }, \
    { u"CharUnderlineColor",           EE_CHAR_UNDERLINE,  ::cppu::UnoType<sal_Int32>::get(),        0, MID_TL_COLOR }, \
    { u"CharUnderlineHasColor",        EE_CHAR_UNDERLINE,  cppu::UnoType<bool>::get(),                    0, MID_TL_HASCOLOR } , \
    { u"" UNO_NAME_EDIT_CHAR_OVERLINE,    EE_CHAR_OVERLINE,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_TL_STYLE }, \
    { u"CharOverlineColor",            EE_CHAR_OVERLINE,   ::cppu::UnoType<sal_Int32>::get(),        0, MID_TL_COLOR }, \
    { u"CharOverlineHasColor",         EE_CHAR_OVERLINE,   cppu::UnoType<bool>::get(),                    0, MID_TL_HASCOLOR } , \
    { u"" UNO_NAME_EDIT_CHAR_CROSSEDOUT,  EE_CHAR_STRIKEOUT,  cppu::UnoType<bool>::get(),                    0, MID_CROSSED_OUT }, \
    { u"" UNO_NAME_EDIT_CHAR_STRIKEOUT,   EE_CHAR_STRIKEOUT,  ::cppu::UnoType<sal_Int16>::get(),        0, MID_CROSS_OUT}, \
    { u"" UNO_NAME_EDIT_CHAR_CASEMAP,     EE_CHAR_CASEMAP,    ::cppu::UnoType<sal_Int16>::get(),        0, 0 }, \
    { u"" UNO_NAME_EDIT_CHAR_SHADOWED,    EE_CHAR_SHADOW,     cppu::UnoType<bool>::get(),                    0, 0 }, \
    { u"CharContoured",                EE_CHAR_OUTLINE,    cppu::UnoType<bool>::get(),                    0, 0 }, \
    { u"CharEscapementHeight",         EE_CHAR_ESCAPEMENT, cppu::UnoType<sal_Int8>::get(),         0, MID_ESC_HEIGHT },\
    { u"CharAutoKerning",              EE_CHAR_PAIRKERNING,cppu::UnoType<bool>::get(),                    0, 0 } , \
    { u"CharKerning",                  EE_CHAR_KERNING,    ::cppu::UnoType<sal_Int16>::get()  ,      0, 0 }, \
    { u"CharWordMode",                 EE_CHAR_WLM,        cppu::UnoType<bool>::get(),                    0, 0 }, \
    { u"CharEmphasis",                 EE_CHAR_EMPHASISMARK, ::cppu::UnoType<sal_Int16>::get(),       0, MID_EMPHASIS},\
    { u"" UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN,        EE_CHAR_FONTHEIGHT_CJK, cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN,      EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_FAMILY_NAME },\
    { u"" UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN, EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_STYLE_NAME }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN,    EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN,   EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN,     EE_CHAR_FONTINFO_CJK,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH }, \
    { u"" UNO_NAME_EDIT_CHAR_POSTURE_ASIAN,       EE_CHAR_ITALIC_CJK,     ::cppu::UnoType<css::awt::FontSlant>::get(),0, MID_POSTURE }, \
    { u"" UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN,        EE_CHAR_WEIGHT_CJK,     cppu::UnoType<float>::get(),            0, MID_WEIGHT }, \
    { u"" UNO_NAME_EDIT_CHAR_LOCALE_ASIAN,        EE_CHAR_LANGUAGE_CJK,   ::cppu::UnoType<css::lang::Locale>::get(),0, MID_LANG_LOCALE }, \
    { u"" UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX,      EE_CHAR_FONTHEIGHT_CTL, cppu::UnoType<float>::get(),            0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX,    EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<OUString>::get(),  0, MID_FONT_FAMILY_NAME },\
    { u"" UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX,EE_CHAR_FONTINFO_CTL,  ::cppu::UnoType<OUString>::get(),  0, MID_FONT_STYLE_NAME }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX,  EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_FAMILY }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX, EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_CHAR_SET }, \
    { u"" UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX,   EE_CHAR_FONTINFO_CTL,   ::cppu::UnoType<sal_Int16>::get(),        0, MID_FONT_PITCH }, \
    { u"" UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX,     EE_CHAR_ITALIC_CTL,     ::cppu::UnoType<css::awt::FontSlant>::get(),0, MID_POSTURE }, \
    { u"" UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX,      EE_CHAR_WEIGHT_CTL,     cppu::UnoType<float>::get(),            0, MID_WEIGHT }, \
    { u"" UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX,      EE_CHAR_LANGUAGE_CTL,   ::cppu::UnoType<css::lang::Locale>::get(),0, MID_LANG_LOCALE }, \
    { u"CharRelief",                           EE_CHAR_RELIEF,         ::cppu::UnoType<sal_Int16>::get(),    0, MID_RELIEF }, \
    { u"CharInteropGrabBag",                   EE_CHAR_GRABBAG,        cppu::UnoType<css::uno::Sequence<css::beans::PropertyValue >>::get(), 0, 0}


#define SVX_UNOEDIT_FONT_PROPERTIES \
    {u"" UNO_NAME_EDIT_FONT_DESCRIPTOR,   WID_FONTDESC,       cppu::UnoType<css::awt::FontDescriptor>::get(),    0, MID_FONT_FAMILY_NAME }

#define SVX_UNOEDIT_PARA_PROPERTIES \
    {u"" UNO_NAME_EDIT_PARA_ADJUST,       EE_PARA_JUST,               ::cppu::UnoType<sal_Int16>::get(),            0, MID_PARA_ADJUST }, \
    {u"" UNO_NAME_EDIT_PARA_BMARGIN,      EE_PARA_ULSPACE,            ::cppu::UnoType<sal_Int32>::get(),            0, MID_LO_MARGIN, PropertyMoreFlags::METRIC_ITEM }, \
    {u"" UNO_NAME_EDIT_PARA_IS_HYPHEN,    EE_PARA_HYPHENATE,  ::cppu::UnoType<bool>::get(),                0, 0 }, \
    {u"ParaHyphenationNoCaps",         EE_PARA_HYPHENATE_NO_CAPS, ::cppu::UnoType<bool>::get(),       0, 0 }, \
    {u"" UNO_NAME_EDIT_PARA_LASTLINEADJ,  EE_PARA_JUST,               ::cppu::UnoType<sal_Int16>::get(),            0, MID_LAST_LINE_ADJUST }, \
    {u"" UNO_NAME_EDIT_PARA_LMARGIN,      EE_PARA_LRSPACE,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_TXT_LMARGIN, PropertyMoreFlags::METRIC_ITEM }, \
    {u"" UNO_NAME_EDIT_PARA_LINESPACING,  EE_PARA_SBL,                cppu::UnoType<css::style::LineSpacing>::get(),     0, CONVERT_TWIPS}, \
    {u"" UNO_NAME_EDIT_PARA_RMARGIN,      EE_PARA_LRSPACE,    ::cppu::UnoType<sal_Int32>::get(),            0, MID_R_MARGIN, PropertyMoreFlags::METRIC_ITEM }, \
    {u"" UNO_NAME_EDIT_PARA_TAPSTOPS,     EE_PARA_TABS,               cppu::UnoType<css::uno::Sequence< css::style::TabStop >>::get(), 0, 0 }, \
    {u"" UNO_NAME_EDIT_PARA_TMARGIN,      EE_PARA_ULSPACE,            ::cppu::UnoType<sal_Int32>::get(),            0, MID_UP_MARGIN, PropertyMoreFlags::METRIC_ITEM },\
    {u"" UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT,     EE_PARA_LRSPACE,   ::cppu::UnoType<sal_Int32>::get(),            0, MID_FIRST_LINE_INDENT, PropertyMoreFlags::METRIC_ITEM}, \
    {u"" UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION,EE_PARA_HANGINGPUNCTUATION,  cppu::UnoType<bool>::get(),                0 ,0 }, \
    {u"" UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE, EE_PARA_ASIANCJKSPACING,   cppu::UnoType<bool>::get(),                0 ,0 }, \
    {u"" UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES,    EE_PARA_FORBIDDENRULES,     cppu::UnoType<bool>::get(),                0 ,0 },\
    {u"WritingMode",                   EE_PARA_WRITINGDIR, ::cppu::UnoType<sal_Int16>::get(),            0, 0 }

class SvxFieldItem;
class SvxFieldData;

EDITENG_DLLPUBLIC void GetSelection( struct ESelection& rSel, SvxTextForwarder const * pForwarder ) noexcept;
EDITENG_DLLPUBLIC void CheckSelection( struct ESelection& rSel, SvxTextForwarder const * pForwarder ) noexcept;


// This class implements a SvxEditSource and SvxTextForwarder and does
// nothing otherwise


class SvxDummyTextSource final : public SvxEditSource, public SvxTextForwarder
{
public:

    // SvxEditSource
    virtual ~SvxDummyTextSource() override;
    virtual std::unique_ptr<SvxEditSource> Clone() const override;
    virtual SvxTextForwarder*       GetTextForwarder() override;
    virtual void                    UpdateData() override;

    // SvxTextForwarder
    virtual sal_Int32       GetParagraphCount() const override;
    virtual sal_Int32       GetTextLen( sal_Int32 nParagraph ) const override;
    virtual OUString        GetText( const ESelection& rSel ) const override;
    virtual SfxItemSet      GetAttribs( const ESelection& rSel, EditEngineAttribs nOnlyHardAttrib = EditEngineAttribs::All ) const override;
    virtual SfxItemSet      GetParaAttribs( sal_Int32 nPara ) const override;
    virtual void            SetParaAttribs( sal_Int32 nPara, const SfxItemSet& rSet ) override;
    virtual void            RemoveAttribs( const ESelection& rSelection ) override;
    virtual void            GetPortions( sal_Int32 nPara, std::vector<sal_Int32>& rList ) const override;

    SfxItemState            GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const override;
    SfxItemState            GetItemState( sal_Int32 nPara, sal_uInt16 nWhich ) const override;

    virtual SfxItemPool*    GetPool() const override;

    virtual void            QuickInsertText( const OUString& rText, const ESelection& rSel ) override;
    virtual void            QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel ) override;
    virtual void            QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel ) override;
    virtual void            QuickInsertLineBreak( const ESelection& rSel ) override;

    virtual OUString        CalcFieldValue( const SvxFieldItem& rField, sal_Int32 nPara, sal_Int32 nPos, std::optional<Color>& rpTxtColor, std::optional<Color>& rpFldColor ) override;
    virtual void            FieldClicked( const SvxFieldItem& rField ) override;

    virtual bool            IsValid() const override;

    virtual LanguageType    GetLanguage( sal_Int32, sal_Int32 ) const override;
    virtual sal_Int32       GetFieldCount( sal_Int32 nPara ) const override;
    virtual EFieldInfo      GetFieldInfo( sal_Int32 nPara, sal_uInt16 nField ) const override;
    virtual EBulletInfo     GetBulletInfo( sal_Int32 nPara ) const override;
    virtual tools::Rectangle       GetCharBounds( sal_Int32 nPara, sal_Int32 nIndex ) const override;
    virtual tools::Rectangle       GetParaBounds( sal_Int32 nPara ) const override;
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


class EDITENG_DLLPUBLIC SvxUnoTextRangeBase : public css::text::XTextRange,
                            public css::beans::XPropertySet,
                            public css::beans::XMultiPropertySet,
                            public css::beans::XMultiPropertyStates,
                            public css::beans::XPropertyState,
                            public css::lang::XServiceInfo,
                            public css::text::XTextRangeCompare,
                            public css::lang::XUnoTunnel,
                            private osl::DebugBase<SvxUnoTextRangeBase>

{
    friend class SvxUnoTextRangeEnumeration;
    friend class ::accessibility::AccessibleEditableTextPara;
    const SvxItemPropertySet* mpPropSet;

protected:
    std::unique_ptr<SvxEditSource> mpEditSource;
    ESelection              maSelection;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void _setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue, sal_Int32 nPara = -1 );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    css::uno::Any _getPropertyValue( const OUString& PropertyName, sal_Int32 nPara = -1 );

    /// @throws css::beans::PropertyVetoException
    /// @throws css::lang::IllegalArgumentException
    /// @throws css::lang::WrappedTargetException
    /// @throws css::uno::RuntimeException
    void _setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues, sal_Int32 nPara = -1 );
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::uno::Any > _getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, sal_Int32 nPara = -1 );

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    css::beans::PropertyState _getPropertyState( const SfxItemPropertyMapEntry* pMap, sal_Int32 nPara = -1 );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    css::beans::PropertyState _getPropertyState( std::u16string_view PropertyName, sal_Int32 nPara = -1 );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    css::uno::Sequence< css::beans::PropertyState > _getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName, sal_Int32 nPara = -1  );
    // returns true if property found or false if unknown property
    static bool _getOnePropertyStates(const SfxItemSet* pSet, const SfxItemPropertyMapEntry* pMap, css::beans::PropertyState& rState);

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    void _setPropertyToDefault( const OUString& PropertyName, sal_Int32 nPara = -1 );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    void _setPropertyToDefault( SvxTextForwarder* pForwarder, const SfxItemPropertyMapEntry* pMap, sal_Int32 nPara );
    void SetEditSource( SvxEditSource* _pEditSource ) noexcept;

    /// @throws css::beans::UnknownPropertyException
    /// @throws css::uno::RuntimeException
    void getPropertyValue( const SfxItemPropertyMapEntry* pMap, css::uno::Any& rAny, const SfxItemSet& rSet );
    /// @throws css::beans::UnknownPropertyException
    /// @throws css::lang::IllegalArgumentException
    void setPropertyValue( const SfxItemPropertyMapEntry* pMap, const css::uno::Any& rValue, const ESelection& rSelection, const SfxItemSet& rOldSet, SfxItemSet& rNewSet );

    SvxUnoTextRangeBase(const SvxItemPropertySet* _pSet);
    SvxUnoTextRangeBase(const SvxEditSource* pSource, const SvxItemPropertySet* _pSet);
    SvxUnoTextRangeBase(const SvxUnoTextRangeBase& rRange);
    virtual ~SvxUnoTextRangeBase() noexcept;

public:
    // Internal
    const ESelection& GetSelection() const noexcept
    {
        const SvxTextForwarder* pForwarder = mpEditSource ? mpEditSource->GetTextForwarder() : nullptr;
        CheckSelection(const_cast<SvxUnoTextRangeBase*>(this)->maSelection, pForwarder);
        return maSelection;
    }
    void                    SetSelection( const ESelection& rSelection ) noexcept;

    void            CollapseToStart() noexcept;
    void            CollapseToEnd() noexcept;
    bool            IsCollapsed() noexcept;
    bool            GoLeft(sal_Int32 nCount, bool Expand) noexcept;
    bool            GoRight(sal_Int32 nCount, bool Expand) noexcept;
    void            GotoStart(bool Expand) noexcept;
    void            GotoEnd(bool Expand) noexcept;

    //const SfxItemPropertyMapEntry*   getPropertyMapEntries() const throw() { return maPropSet.getPropertyMapEntries(); }
    const SvxItemPropertySet*   getPropertySet() const noexcept { return mpPropSet; }
    SvxEditSource*              GetEditSource() const noexcept { return mpEditSource.get(); }

    static bool SetPropertyValueHelper( const SfxItemPropertyMapEntry* pMap, const css::uno::Any& aValue, SfxItemSet& rNewSet, const ESelection* pSelection = nullptr, SvxEditSource* pEditSource = nullptr );
    /// @throws css::uno::RuntimeException
    static bool GetPropertyValueHelper(  SfxItemSet const & rSet, const SfxItemPropertyMapEntry* pMap, css::uno::Any& aAny, const ESelection* pSelection = nullptr,  SvxEditSource* pEditSource = nullptr  );

    void attachField( std::unique_ptr<SvxFieldData> pData ) noexcept;

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextRangeBase )

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString( const OUString& aString ) override;

    // css::beans::XPropertySet
    virtual css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() override;
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;
    virtual void SAL_CALL addPropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertyChangeListener( const OUString& aPropertyName, const css::uno::Reference< css::beans::XPropertyChangeListener >& aListener ) override;
    virtual void SAL_CALL addVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;
    virtual void SAL_CALL removeVetoableChangeListener( const OUString& PropertyName, const css::uno::Reference< css::beans::XVetoableChangeListener >& aListener ) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual void SAL_CALL addPropertiesChangeListener( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL removePropertiesChangeListener( const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;
    virtual void SAL_CALL firePropertiesChangeEvent( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Reference< css::beans::XPropertiesChangeListener >& xListener ) override;

    // css::beans::XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;
    virtual css::uno::Any SAL_CALL getPropertyDefault( const OUString& aPropertyName ) override;

    // css::beans::XMultiPropertyStates
    //virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) throw (css::beans::UnknownPropertyException, css::uno::RuntimeException);
    virtual void SAL_CALL setAllPropertiesToDefault() override;
    virtual void SAL_CALL setPropertiesToDefault( const css::uno::Sequence< OUString >& aPropertyNames ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyDefaults( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    // XTextRangeCompare
    virtual ::sal_Int16 SAL_CALL compareRegionStarts( const css::uno::Reference< css::text::XTextRange >& xR1, const css::uno::Reference< css::text::XTextRange >& xR2 ) override;
    virtual ::sal_Int16 SAL_CALL compareRegionEnds( const css::uno::Reference< css::text::XTextRange >& xR1, const css::uno::Reference< css::text::XTextRange >& xR2 ) override;

    // css::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    static  css::uno::Sequence< OUString > getSupportedServiceNames_Static( );
};


class SvxUnoTextBase;
class EDITENG_DLLPUBLIC SvxUnoTextRange final : public SvxUnoTextRangeBase,
                        public css::lang::XTypeProvider,
                        public ::cppu::OWeakAggObject
{
    friend class SvxUnoTextRangeEnumeration;
private:
    css::uno::Reference< css::text::XText >   xParentText;
    bool mbPortion;

public:
    SvxUnoTextRange(const SvxUnoTextBase& rParent, bool bPortion = false);
    virtual ~SvxUnoTextRange() noexcept override;

    // css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL getText() override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;

    // css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};

class EDITENG_DLLPUBLIC SvxUnoTextBase  : public SvxUnoTextRangeBase,
                        public css::text::XTextAppend,
                        public css::text::XTextCopy,
                        public css::container::XEnumerationAccess,
                        public css::text::XTextRangeMover,
                        public css::lang::XTypeProvider
{
    css::uno::Reference< css::text::XText >   xParentText;

protected:
    SvxUnoTextBase(const SvxItemPropertySet* _pSet);
    SvxUnoTextBase(const SvxEditSource* pSource, const SvxItemPropertySet* _pSet, css::uno::Reference < css::text::XText > const & xParent);
    SvxUnoTextBase(const SvxUnoTextBase& rText);
    virtual ~SvxUnoTextBase() noexcept override;

public:
    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextBase )

    css::uno::Reference< css::text::XTextCursor > createTextCursorBySelection( const ESelection& rSel );

    // css::uno::XInterface
    /// @throws css::uno::RuntimeException
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType );

    // css::text::XSimpleText
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL createTextCursor(  ) override;
    virtual css::uno::Reference< css::text::XTextCursor > SAL_CALL createTextCursorByRange( const css::uno::Reference< css::text::XTextRange >& aTextPosition ) override;
    virtual void SAL_CALL insertString( const css::uno::Reference< css::text::XTextRange >& xRange, const OUString& aString, sal_Bool bAbsorb ) override;
    virtual void SAL_CALL insertControlCharacter( const css::uno::Reference< css::text::XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb ) override;

    // css::text::XText
    virtual void SAL_CALL insertTextContent( const css::uno::Reference< css::text::XTextRange >& xRange, const css::uno::Reference< css::text::XTextContent >& xContent, sal_Bool bAbsorb ) override;
    virtual void SAL_CALL removeTextContent( const css::uno::Reference< css::text::XTextContent >& xContent ) override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString( const OUString& aString ) override;

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL getText(  ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd() override;

    // css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

    // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType(  ) override;
    virtual sal_Bool SAL_CALL hasElements(  ) override;

    // css::text::XTextRangeMover
    virtual void SAL_CALL moveTextRange( const css::uno::Reference< css::text::XTextRange >& xRange, sal_Int16 nParagraphs ) override;

    // css::text::XParagraphAppend (new import API)
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL finishParagraph( const css::uno::Sequence< css::beans::PropertyValue >& CharacterAndParagraphProperties ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL finishParagraphInsert( const css::uno::Sequence< css::beans::PropertyValue >& CharacterAndParagraphProperties, const css::uno::Reference< css::text::XTextRange >& xInsertPosition ) override;

    // css::text::XTextPortionAppend (new import API)
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL appendTextPortion( const OUString& Text, const css::uno::Sequence< css::beans::PropertyValue >& CharacterAndParagraphProperties ) override;

    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL insertTextPortion( const OUString& Text, const css::uno::Sequence< css::beans::PropertyValue >& CharacterAndParagraphProperties, const css::uno::Reference< css::text::XTextRange>& rTextRange ) override;

    // css::text::XTextCopy
    virtual void SAL_CALL copyText( const css::uno::Reference< css::text::XTextCopy >& xSource ) override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;
    static  css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames_Static( );

    // css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};


class EDITENG_DLLPUBLIC SvxUnoText : public SvxUnoTextBase,
                    public ::cppu::OWeakAggObject
{
public:
    SvxUnoText( const SvxItemPropertySet* _pSet ) noexcept;
    SvxUnoText( const SvxEditSource* pSource, const SvxItemPropertySet* _pSet, css::uno::Reference < css::text::XText > const & xParent ) noexcept;
    SvxUnoText( const SvxUnoText& rText ) noexcept;
    virtual ~SvxUnoText() noexcept override;

    // Internal
    static const css::uno::Sequence< sal_Int8 > & getUnoTunnelId() noexcept;
    virtual sal_Int64 SAL_CALL getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier ) override;

    // css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};


class SvxUnoTextContent final : public SvxUnoTextRangeBase,
                          public css::text::XTextContent,
                          public css::container::XEnumerationAccess,
                          public css::lang::XTypeProvider,
                          public ::cppu::OWeakAggObject
{
    friend class SvxUnoTextContentEnumeration;
private:
    css::uno::Reference< css::text::XText > mxParentText;
    sal_Int32               mnParagraph;
    const SvxUnoTextBase&   mrParentText;

    // for xComponent
    ::osl::Mutex maDisposeContainerMutex;
    ::comphelper::OInterfaceContainerHelper2 maDisposeListeners;
    bool mbDisposing;

protected:
    using SvxUnoTextRangeBase::setPropertyValue;
    using SvxUnoTextRangeBase::getPropertyValue;

public:
    SvxUnoTextContent( const SvxUnoTextBase& rText, sal_Int32 nPara ) noexcept;
    SvxUnoTextContent( const SvxUnoTextContent& rContent ) noexcept;
    virtual ~SvxUnoTextContent() noexcept override;

    // css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL getText(  ) override;

    // css::text::XTextContent -> css::lang::XComponent
    virtual void SAL_CALL attach( const css::uno::Reference< css::text::XTextRange >& xTextRange ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getAnchor(  ) override;

    // css::lang::XComponent
    virtual void SAL_CALL dispose() override;
    virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) override;
    virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& aListener ) override;

    // css::container::XEnumerationAccess
    virtual css::uno::Reference< css::container::XEnumeration > SAL_CALL createEnumeration(  ) override;

    // css::container::XElementAccess
    virtual css::uno::Type SAL_CALL getElementType() override;
    virtual sal_Bool SAL_CALL hasElements() override;

    // css::beans::XPropertySet
    virtual void SAL_CALL setPropertyValue( const OUString& aPropertyName, const css::uno::Any& aValue ) override;
    virtual css::uno::Any SAL_CALL getPropertyValue( const OUString& PropertyName ) override;

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames, const css::uno::Sequence< css::uno::Any >& aValues ) override;
    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getPropertyValues( const css::uno::Sequence< OUString >& aPropertyNames ) override;

    // css::beans::XPropertyState
    virtual css::beans::PropertyState SAL_CALL getPropertyState( const OUString& PropertyName ) override;
    virtual css::uno::Sequence< css::beans::PropertyState > SAL_CALL getPropertyStates( const css::uno::Sequence< OUString >& aPropertyName ) override;
    virtual void SAL_CALL setPropertyToDefault( const OUString& PropertyName ) override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames(  ) override;

    // css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;
};


class SvxUnoTextContentEnumeration : public ::cppu::WeakAggImplHelper1< css::container::XEnumeration >
{
private:
    css::uno::Reference< css::text::XText > mxParentText;
    std::unique_ptr<SvxEditSource>          mpEditSource;
    sal_Int32               mnNextParagraph;
    std::vector< rtl::Reference<SvxUnoTextContent> >  maContents;

public:
    SvxUnoTextContentEnumeration( const SvxUnoTextBase& _rText, const ESelection& rSel ) noexcept;
    virtual ~SvxUnoTextContentEnumeration() noexcept override;

    // css::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;
};


class SvxUnoTextRangeEnumeration : public ::cppu::WeakAggImplHelper1< css::container::XEnumeration >
{
private:
    std::unique_ptr<SvxEditSource>      mpEditSource;
    css::uno::Reference< css::text::XText > mxParentText;
    std::vector< rtl::Reference<SvxUnoTextRange> >  maPortions;
    sal_uInt16               mnNextPortion;

public:
    SvxUnoTextRangeEnumeration(const SvxUnoTextBase& rText, sal_Int32 nPara, const ESelection& rSel);
    virtual ~SvxUnoTextRangeEnumeration() noexcept override;

    // css::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) override;
    virtual css::uno::Any SAL_CALL nextElement(  ) override;
};


class EDITENG_DLLPUBLIC SvxUnoTextCursor : public SvxUnoTextRangeBase,
                         public css::text::XTextCursor,
                         public css::lang::XTypeProvider,
                         public ::cppu::OWeakAggObject
{
private:
    css::uno::Reference< css::text::XText > mxParentText;

public:
    SvxUnoTextCursor( const SvxUnoTextBase& rText ) noexcept;
    SvxUnoTextCursor( const SvxUnoTextCursor& rCursor ) noexcept;
    virtual ~SvxUnoTextCursor() noexcept override;

    // css::uno::XInterface
    virtual css::uno::Any SAL_CALL queryAggregation( const css::uno::Type & rType ) override;
    virtual css::uno::Any SAL_CALL queryInterface( const css::uno::Type & rType ) override;
    virtual void SAL_CALL acquire() noexcept override;
    virtual void SAL_CALL release() noexcept override;

    // css::text::XTextRange
    virtual css::uno::Reference< css::text::XText > SAL_CALL getText() override;
    virtual OUString SAL_CALL getString() override;
    virtual void SAL_CALL setString( const OUString& aString ) override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getStart() override;
    virtual css::uno::Reference< css::text::XTextRange > SAL_CALL getEnd() override;

    // css::text::XTextCursor -> css::text::XTextRange
    virtual void SAL_CALL collapseToStart(  ) override;
    virtual void SAL_CALL collapseToEnd(  ) override;
    virtual sal_Bool SAL_CALL isCollapsed(  ) override;
    virtual sal_Bool SAL_CALL goLeft( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual sal_Bool SAL_CALL goRight( sal_Int16 nCount, sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoStart( sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoEnd( sal_Bool bExpand ) override;
    virtual void SAL_CALL gotoRange( const css::uno::Reference< css::text::XTextRange >& xRange, sal_Bool bExpand ) override;

    // css::lang::XServiceInfo
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    // css::lang::XTypeProvider
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes(  ) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) override;

};

EDITENG_DLLPUBLIC const SvxItemPropertySet* ImplGetSvxUnoOutlinerTextCursorSvxPropertySet();
const SfxItemPropertyMapEntry* ImplGetSvxUnoOutlinerTextCursorPropertyMap();
const SvxItemPropertySet* ImplGetSvxTextPortionSvxPropertySet();
const SfxItemPropertyMapEntry* ImplGetSvxTextPortionPropertyMap();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
