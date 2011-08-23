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

#ifndef _SVX_UNOTEXT_HXX
#define _SVX_UNOTEXT_HXX

#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif

#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_ 
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXT_HPP_
#include <com/sun/star/text/XText.hpp>
#endif
#ifndef _SMART_COM_SUN_STAR_CONTAINER_XENUMERATIONACCESS
#include <com/sun/star/container/XEnumerationAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGEMOVER_HPP_
#include <com/sun/star/text/XTextRangeMover.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCURSOR_HPP_
#include <com/sun/star/text/XTextCursor.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XTYPEPROVIDER_HPP_
#include <com/sun/star/lang/XTypeProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSTATE_HPP_
#include <com/sun/star/beans/XPropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XMULTIPROPERTYSET_HPP_ 
#include <com/sun/star/beans/XMultiPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif
#ifndef _CPPUHELPER_WEAKAGG_HXX_
#include <cppuhelper/weakagg.hxx>
#endif

#ifndef _SFX_ITEMPROP_HXX
#include <bf_svtools/itemprop.hxx>
#endif

#ifndef _MyEDITDATA_HXX // ESelection
#include <bf_svx/editdata.hxx>
#endif

#ifndef _SVX_UNOEDSRC_HXX
#include <bf_svx/unoedsrc.hxx>
#endif

#ifndef _SFXITEMSET_HXX //autog
#include <bf_svtools/itemset.hxx>
#endif

#ifndef _SFXITEMPOOL_HXX //auto
#include <bf_svtools/itempool.hxx>
#endif

#ifndef _SVX_SVXENUM_HXX //autogen
#include <bf_svx/svxenum.hxx>
#endif

#ifndef _OUTLINER_HXX //autogen
#include <bf_svx/outliner.hxx>
#endif

#ifndef _SVX_UNOFOROU_HXX //autogen
#include <bf_svx/unoforou.hxx>
#endif

#include <bf_svx/unoprnms.hxx>

#ifndef _EEITEM_HXX
#include <bf_svx/eeitem.hxx>
#endif

#ifndef _COM_SUN_STAR_TEXT_XTEXTFIELD_HPP_
#include <com/sun/star/text/XTextField.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTSLANT_HPP_
#include <com/sun/star/awt/FontSlant.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_LINESPACING_HPP_
#include <com/sun/star/style/LineSpacing.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_TABSTOP_HPP_
#include <com/sun/star/style/TabStop.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTRANGE_HPP_
#include <com/sun/star/text/XTextRange.hpp>
#endif

#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif

#include <comphelper/servicehelper.hxx>

#ifndef SEQTYPE
 #if defined(__SUNPRO_CC) && (__SUNPRO_CC == 0x500)
  #define SEQTYPE(x) (new ::com::sun::star::uno::Type( x ))
 #else
  #define SEQTYPE(x) &(x)
 #endif
#endif

#include <bf_svx/unoprov.hxx>
#include <bf_svx/unomid.hxx>
namespace binfilter {

#define WID_FONTDESC		3900
#define WID_NUMLEVEL		3901
#define WID_PORTIONTYPE		3903

#define SVX_UNOEDIT_NUMBERING_PROPERTIE \
    {MAP_CHAR_LEN(UNO_NAME_NUMBERING_RULES),		EE_PARA_NUMBULLET,	&::getCppuType((const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace>*)0), 0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_NUMBERING),				EE_PARA_BULLETSTATE,&::getBooleanCppuType(), 0, 0 }

#define SVX_UNOEDIT_OUTLINER_PROPERTIES \
    SVX_UNOEDIT_NUMBERING_PROPERTIE, \
    {MAP_CHAR_LEN(UNO_NAME_NUMBERING_LEVEL),		WID_NUMLEVEL,		&::getCppuType((const sal_Int16*)0), 0, 0 }

#define SVX_UNOEDIT_CHAR_PROPERTIES \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT),		EE_CHAR_FONTHEIGHT,	&::getCppuType((const float*)0),			0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { MAP_CHAR_LEN("CharScaleWidth"),				EE_CHAR_FONTWIDTH, 		&::getCppuType((const sal_Int16*)0),    0, 0 }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME),	EE_CHAR_FONTINFO,	&::getCppuType((const ::rtl::OUString*)0),	0, MID_FONT_FAMILY_NAME },\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME),EE_CHAR_FONTINFO,	&::getCppuType((const ::rtl::OUString*)0),	0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY),	EE_CHAR_FONTINFO,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_FAMILY }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET),	EE_CHAR_FONTINFO,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_CHAR_SET }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH),	EE_CHAR_FONTINFO,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_PITCH }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE),		EE_CHAR_ITALIC,		&::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT),		EE_CHAR_WEIGHT,		&::getCppuType((const float*)0),			0, MID_WEIGHT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_LOCALE),		EE_CHAR_LANGUAGE,	&::getCppuType((const ::com::sun::star::lang::Locale*)0),0, MID_LANG_LOCALE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_COLOR),		EE_CHAR_COLOR,		&::getCppuType((const sal_Int32*)0),		0, 0 }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_ESCAPEMENT),	EE_CHAR_ESCAPEMENT,	&::getCppuType((const sal_Int16*)0),		0, MID_ESC }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_UNDERLINE),	EE_CHAR_UNDERLINE,	&::getCppuType((const sal_Int16*)0),		0, MID_UNDERLINE }, \
    { MAP_CHAR_LEN("CharUnderlineColor"),			EE_CHAR_UNDERLINE,  &::getCppuType((const sal_Int32*)0),		0, MID_UL_COLOR }, \
    { MAP_CHAR_LEN("CharUnderlineHasColor"),		EE_CHAR_UNDERLINE,	&::getBooleanCppuType(),					0, MID_UL_HASCOLOR } , \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_CROSSEDOUT),	EE_CHAR_STRIKEOUT,	&::getBooleanCppuType(),					0, MID_CROSSED_OUT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_STRIKEOUT),	EE_CHAR_STRIKEOUT,  &::getCppuType((const sal_Int16*)0),		0, MID_CROSS_OUT}, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_SHADOWED),	EE_CHAR_SHADOW,		&::getBooleanCppuType(),					0, 0 }, \
    { MAP_CHAR_LEN("CharContoured"),				EE_CHAR_OUTLINE,	&::getBooleanCppuType(),					0, 0 }, \
    { MAP_CHAR_LEN("CharEscapementHeight"),			EE_CHAR_ESCAPEMENT, &::getCppuType((const sal_Int8*)0),			0, MID_ESC_HEIGHT },\
    { MAP_CHAR_LEN("CharAutoKerning"),				EE_CHAR_PAIRKERNING,&::getBooleanCppuType(),					0, 0 } , \
    { MAP_CHAR_LEN("CharKerning"),					EE_CHAR_KERNING,	&::getCppuType((const sal_Int16*)0)  ,  	0, 0 }, \
    { MAP_CHAR_LEN("CharWordMode"),					EE_CHAR_WLM,		&::getBooleanCppuType(),					0, 0 }, \
    { MAP_CHAR_LEN("CharEmphasis"),					EE_CHAR_EMPHASISMARK,&::getCppuType((const sal_Int16*)0),		0, MID_EMPHASIS},\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT_ASIAN),		EE_CHAR_FONTHEIGHT_CJK,	&::getCppuType((const float*)0),			0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME_ASIAN),		EE_CHAR_FONTINFO_CJK,	&::getCppuType((const ::rtl::OUString*)0),	0, MID_FONT_FAMILY_NAME },\
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME_ASIAN),	EE_CHAR_FONTINFO_CJK,	&::getCppuType((const ::rtl::OUString*)0),	0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY_ASIAN),	EE_CHAR_FONTINFO_CJK,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_FAMILY }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET_ASIAN),	EE_CHAR_FONTINFO_CJK,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_CHAR_SET }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH_ASIAN),		EE_CHAR_FONTINFO_CJK,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_PITCH }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE_ASIAN),		EE_CHAR_ITALIC_CJK,		&::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT_ASIAN),		EE_CHAR_WEIGHT_CJK,		&::getCppuType((const float*)0),			0, MID_WEIGHT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_LOCALE_ASIAN),		EE_CHAR_LANGUAGE_CJK,	&::getCppuType((const ::com::sun::star::lang::Locale*)0),0, MID_LANG_LOCALE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_HEIGHT_COMPLEX),		EE_CHAR_FONTHEIGHT_CTL,	&::getCppuType((const float*)0),			0, MID_FONTHEIGHT|CONVERT_TWIPS }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTNAME_COMPLEX),	EE_CHAR_FONTINFO_CTL,	&::getCppuType((const ::rtl::OUString*)0),	0, MID_FONT_FAMILY_NAME },\
    {MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTSTYLENAME_COMPLEX),EE_CHAR_FONTINFO_CTL,	&::getCppuType((const ::rtl::OUString*)0),	0, MID_FONT_STYLE_NAME }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTFAMILY_COMPLEX),	EE_CHAR_FONTINFO_CTL,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_FAMILY }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTCHARSET_COMPLEX),	EE_CHAR_FONTINFO_CTL,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_CHAR_SET }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_FONTPITCH_COMPLEX),	EE_CHAR_FONTINFO_CTL,	&::getCppuType((const sal_Int16*)0),		0, MID_FONT_PITCH }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_POSTURE_COMPLEX),		EE_CHAR_ITALIC_CTL,		&::getCppuType((const ::com::sun::star::awt::FontSlant*)0),0, MID_POSTURE }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_WEIGHT_COMPLEX),		EE_CHAR_WEIGHT_CTL,		&::getCppuType((const float*)0),			0, MID_WEIGHT }, \
    { MAP_CHAR_LEN(UNO_NAME_EDIT_CHAR_LOCALE_COMPLEX),		EE_CHAR_LANGUAGE_CTL,	&::getCppuType((const ::com::sun::star::lang::Locale*)0),0, MID_LANG_LOCALE }, \
    { MAP_CHAR_LEN("CharRelief"            ),				EE_CHAR_RELIEF,		    &::getCppuType((const sal_Int16*)0),    0, MID_RELIEF }

#define SVX_UNOEDIT_FONT_PROPERTIES \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_FONT_DESCRIPTOR),	WID_FONTDESC,		&::getCppuType((const ::com::sun::star::awt::FontDescriptor*)0),	0, MID_FONT_FAMILY_NAME }

#define SVX_UNOEDIT_PARA_PROPERTIES \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_ADJUST),		EE_PARA_JUST,				&::getCppuType((const sal_Int16*)0),			0, MID_PARA_ADJUST }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_BMARGIN),		EE_PARA_ULSPACE,			&::getCppuType((const sal_Int32*)0),			0, MID_LO_MARGIN|SFX_METRIC_ITEM }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_HYPHEN),	EE_PARA_HYPHENATE,			&::getBooleanCppuType(),				0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_LASTLINEADJ),	EE_PARA_JUST,				&::getCppuType((const sal_Int16*)0),			0, MID_LAST_LINE_ADJUST }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_LMARGIN),		EE_PARA_LRSPACE,			&::getCppuType((const sal_Int32*)0),			0, MID_TXT_LMARGIN|SFX_METRIC_ITEM }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_LINESPACING),	EE_PARA_SBL,				&::getCppuType((const ::com::sun::star::style::LineSpacing*)0),		0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_RMARGIN),		EE_PARA_LRSPACE,			&::getCppuType((const sal_Int32*)0),			0, MID_R_MARGIN|SFX_METRIC_ITEM }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_TAPSTOPS),		EE_PARA_TABS,				SEQTYPE(::getCppuType((const ::com::sun::star::uno::Sequence< ::com::sun::star::style::TabStop >*)0)), 0, 0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_TMARGIN),		EE_PARA_ULSPACE,			&::getCppuType((const sal_Int32*)0),			0, MID_UP_MARGIN|SFX_METRIC_ITEM },\
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_FIRST_LINE_INDENT),	 EE_PARA_LRSPACE,		&::getCppuType((const sal_Int32*)0),			0, MID_FIRST_LINE_INDENT|SFX_METRIC_ITEM}, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_HANGING_PUNCTUATION),EE_PARA_HANGINGPUNCTUATION,	&::getBooleanCppuType(),				0 ,0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_CHARACTER_DISTANCE), EE_PARA_ASIANCJKSPACING,	&::getBooleanCppuType(),				0 ,0 }, \
    {MAP_CHAR_LEN(UNO_NAME_EDIT_PARA_IS_FORBIDDEN_RULES),	 EE_PARA_FORBIDDENRULES,		&::getBooleanCppuType(),				0 ,0 },\
    {MAP_CHAR_LEN("WritingMode"),					EE_PARA_WRITINGDIR,			&::getCppuType((const sal_Int16*)0),			0, 0 }

SvxAdjust ConvertUnoAdjust( unsigned short nAdjust ) throw();
unsigned short ConvertUnoAdjust( SvxAdjust eAdjust ) throw();

class SvxEditSource;
class SvxTextForwarder;
class SvxFieldItem;
class SvxTextEditSource;
class SvxFieldData;
struct ESelection;

void GetSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw();
void CheckSelection( struct ESelection& rSel, SvxTextForwarder* pForwarder ) throw();

// ====================================================================
// Diese Klasse implementiert eine SvxEditSource und einen SvxTextForwarder
// und macht ansonsten rein garnichts
// ====================================================================

class SvxDummyTextSource : public SvxEditSource, public SvxTextForwarder
{
public:

    // SvxEditSource
    virtual ~SvxDummyTextSource();
    virtual SvxEditSource*			Clone() const;
    virtual SvxTextForwarder*		GetTextForwarder();
    virtual void					UpdateData();

    // SvxTextForwarder
    virtual sal_uInt16		GetParagraphCount() const;
    virtual sal_uInt16		GetTextLen( sal_uInt16 nParagraph ) const;
    virtual String			GetText( const ESelection& rSel ) const;
    virtual SfxItemSet		GetAttribs( const ESelection& rSel, BOOL bOnlyHardAttrib = 0 ) const;
    virtual	SfxItemSet		GetParaAttribs( sal_uInt16 nPara ) const;
    virtual void			SetParaAttribs( sal_uInt16 nPara, const SfxItemSet& rSet );
    virtual void			GetPortions( sal_uInt16 nPara, SvUShorts& rList ) const;

    sal_uInt16				GetItemState( const ESelection& rSel, sal_uInt16 nWhich ) const;
    sal_uInt16				GetItemState( sal_uInt16 nPara, sal_uInt16 nWhich ) const;

    virtual SfxItemPool* 	GetPool() const;

    virtual void			QuickInsertText( const String& rText, const ESelection& rSel );
    virtual void			QuickInsertField( const SvxFieldItem& rFld, const ESelection& rSel );
    virtual void			QuickSetAttribs( const SfxItemSet& rSet, const ESelection& rSel );
    virtual void			QuickInsertLineBreak( const ESelection& rSel );

    virtual XubString		CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos, Color*& rpTxtColor, Color*& rpFldColor );
    virtual sal_Bool		IsValid() const;

    virtual void 			SetNotifyHdl( const Link& );
    virtual LanguageType 	GetLanguage( USHORT, USHORT ) const;
    virtual USHORT			GetFieldCount( USHORT nPara ) const;
    virtual EFieldInfo		GetFieldInfo( USHORT nPara, USHORT nField ) const;
    virtual EBulletInfo     GetBulletInfo( USHORT nPara ) const;
    virtual Rectangle		GetCharBounds( USHORT nPara, USHORT nIndex ) const;
    virtual Rectangle		GetParaBounds( USHORT nPara ) const;
    virtual MapMode		 	GetMapMode() const;
    virtual OutputDevice*	GetRefDevice() const;
    virtual sal_Bool		GetIndexAtPoint( const Point&, USHORT& nPara, USHORT& nIndex ) const;
    virtual sal_Bool		GetWordIndices( USHORT nPara, USHORT nIndex, USHORT& nStart, USHORT& nEnd ) const;
    virtual sal_Bool 		GetAttributeRun( USHORT& nStartIndex, USHORT& nEndIndex, USHORT nPara, USHORT nIndex ) const;
    virtual USHORT			GetLineCount( USHORT nPara ) const;
    virtual USHORT			GetLineLen( USHORT nPara, USHORT nLine ) const;
    virtual sal_Bool		Delete( const ESelection& );
    virtual sal_Bool		InsertText( const String&, const ESelection& );
    virtual sal_Bool		QuickFormatDoc( BOOL bFull=FALSE );
    virtual USHORT			GetDepth( USHORT nPara ) const;
    virtual sal_Bool		SetDepth( USHORT nPara, USHORT nNewDepth );

};

// ====================================================================

class SvxUnoTextRangeBase : public ::com::sun::star::text::XTextRange,
                            public ::com::sun::star::beans::XPropertySet,
                            public ::com::sun::star::beans::XMultiPropertySet,
                            public ::com::sun::star::beans::XPropertyState,
                            public ::com::sun::star::lang::XServiceInfo,
                            public ::com::sun::star::lang::XUnoTunnel

{
private:
    SvxEditSource*			pEditSource;
    ESelection				aSelection;
    SvxItemPropertySet		aPropSet;

protected:
    virtual void SAL_CALL _setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL _getPropertyValue( const ::rtl::OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL _setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues, sal_Int32 nPara = -1 ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL _getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, sal_Int32 nPara = -1 ) throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::beans::PropertyState SAL_CALL _getPropertyState( const ::rtl::OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL _getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName, sal_Int32 nPara = -1  ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL _setPropertyToDefault( const ::rtl::OUString& PropertyName, sal_Int32 nPara = -1 ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    void SetEditSource( SvxEditSource* _pEditSource ) throw();

    virtual void getPropertyValue( const SfxItemPropertyMap* pMap, ::com::sun::star::uno::Any& rAny, const SfxItemSet& rSet ) throw(::com::sun::star::beans::UnknownPropertyException );
    virtual void setPropertyValue( const SfxItemPropertyMap* pMap, const ::com::sun::star::uno::Any& rValue, const ESelection& rSelection, const SfxItemSet& rOldSet, SfxItemSet& rNewSet ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::IllegalArgumentException );

public:
    SvxUnoTextRangeBase( const SfxItemPropertyMap* _pMap ) throw();
    SvxUnoTextRangeBase( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap ) throw();
    SvxUnoTextRangeBase( const SvxUnoTextRangeBase& rRange ) throw();
    virtual ~SvxUnoTextRangeBase() throw();

    // Internal
    const ESelection&		GetSelection() const throw() { CheckSelection( ((SvxUnoTextRangeBase*)this)->aSelection, pEditSource->GetTextForwarder() ); return aSelection; };
    void					SetSelection( const ESelection& rSelection ) throw();

    virtual void			CollapseToStart(void) throw();
    virtual void			CollapseToEnd(void) throw();
    virtual sal_Bool		IsCollapsed(void) throw();
    virtual sal_Bool		GoLeft(sal_Int16 nCount, sal_Bool Expand) throw();
    virtual sal_Bool		GoRight(sal_Int16 nCount, sal_Bool Expand) throw();
    virtual void			GotoStart(sal_Bool Expand) throw();
    virtual void			GotoEnd(sal_Bool Expand) throw();

    const SfxItemPropertyMap*	getPropertyMap() const throw() { return aPropSet.getPropertyMap(); }
    SvxEditSource*				GetEditSource() const throw() { return pEditSource; }

    static sal_Bool SetPropertyValueHelper( const SfxItemSet& rOldSet, const SfxItemPropertyMap* pMap, const ::com::sun::star::uno::Any& aValue, SfxItemSet& rNewSet, const ESelection* pSelection = NULL, SvxTextEditSource* pEditSource = NULL ) throw( ::com::sun::star::uno::RuntimeException );
    static sal_Bool GetPropertyValueHelper(  SfxItemSet& rSet, const SfxItemPropertyMap* pMap, ::com::sun::star::uno::Any& aAny, const ESelection* pSelection = NULL,  SvxTextEditSource* pEditSource = NULL  ) throw( ::com::sun::star::uno::RuntimeException );

    void attachField( const SvxFieldData* pData ) throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextRangeBase )

    // ::com::sun::star::text::XTextRange
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getStart() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > SAL_CALL getEnd() throw(::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertySet
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL addPropertiesChangeListener( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removePropertiesChangeListener( const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL firePropertiesChangeEvent( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertiesChangeListener >& xListener ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyDefault( const ::rtl::OUString& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);
};

// ====================================================================

class SvxUnoTextBase;
class SvxUnoTextRange : public SvxUnoTextRangeBase,
                        public ::com::sun::star::lang::XTypeProvider,
                        public ::cppu::OWeakAggObject
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > 	xParentText;
    sal_Bool mbPortion;

protected:
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

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
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

class SvxUnoTextBase  : public SvxUnoTextRangeBase,
                        public ::com::sun::star::text::XText,
                        public ::com::sun::star::container::XEnumerationAccess,
                        public ::com::sun::star::text::XTextRangeMover,
                        public ::com::sun::star::lang::XTypeProvider
{
protected:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > 	xParentText;
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

public:
    SvxUnoTextBase( const SfxItemPropertyMap* _pMap ) throw();
    SvxUnoTextBase( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap, ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > xParent ) throw();
    SvxUnoTextBase( const SvxUnoTextBase& rText ) throw();
    virtual ~SvxUnoTextBase() throw();

    UNO3_GETIMPLEMENTATION_DECL( SvxUnoTextBase )

    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getStaticTypes() throw();

    sal_Bool queryAggregation( const ::com::sun::star::uno::Type & rType, ::com::sun::star::uno::Any& rAny );

    // ::com::sun::star::uno::XInterface
    virtual ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type & rType ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XSimpleText
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursor(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextCursor > SAL_CALL createTextCursorByRange( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& aTextPosition ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertString( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::rtl::OUString& aString, sal_Bool bAbsorb ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL insertControlCharacter( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, sal_Int16 nControlCharacter, sal_Bool bAbsorb ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::text::XText
    virtual void SAL_CALL insertTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange >& xRange, const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent, sal_Bool bAbsorb ) throw(::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeTextContent( const ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextContent >& xContent ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::uno::RuntimeException);
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException);

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

    // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);
};

// ====================================================================
class SvxUnoText  : public SvxUnoTextBase,
                    public ::cppu::OWeakAggObject
{
public:
    SvxUnoText( const SvxEditSource* pSource, const SfxItemPropertyMap* _pMap, ::com::sun::star::uno::Reference < ::com::sun::star::text::XText > xParent ) throw();
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

#include <cppuhelper/implbase1.hxx>

class SvxUnoTextContentEnumeration : public ::cppu::WeakAggImplHelper1< ::com::sun::star::container::XEnumeration >
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > xParentText;
    SvxEditSource*			pEditSource;
    sal_uInt16				nNextParagraph;
    const SvxUnoTextBase&		rText;

public:
    SvxUnoTextContentEnumeration( const SvxUnoTextBase& _rText ) throw();
    virtual ~SvxUnoTextContentEnumeration() throw();

    // ::com::sun::star::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

// ====================================================================
}//end of namespace binfilter
#ifndef _COM_SUN_STAR_TEXT_XTEXTCONTENT_HPP_
#include <com/sun/star/text/XTextContent.hpp>
#endif
namespace binfilter {
class SvUShorts;
class SvxUnoTextContent : public SvxUnoTextRangeBase,
                          public ::com::sun::star::text::XTextContent,
                          public ::com::sun::star::container::XEnumerationAccess,
                          public ::com::sun::star::lang::XTypeProvider,
                          public ::cppu::OWeakAggObject
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > xParentText;
    sal_uInt16				nParagraph;
    const SvxUnoTextBase&	rParentText;

    // for xComponent
    ::osl::Mutex aDisposeContainerMutex;
    ::cppu::OInterfaceContainerHelper aDisposeListeners;
    BOOL bDisposing;

protected:
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

public:
    SvxUnoTextContent( const SvxUnoTextBase& rText, sal_uInt16 nPara ) throw();
    SvxUnoTextContent( const SvxUnoTextContent& rContent ) throw();
    virtual	~SvxUnoTextContent() throw();

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
    virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // XMultiPropertySet
    virtual void SAL_CALL setPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames, const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aValues ) throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any > SAL_CALL getPropertyValues( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyNames ) throw (::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::beans::XPropertyState
    virtual ::com::sun::star::beans::PropertyState SAL_CALL getPropertyState( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyState > SAL_CALL getPropertyStates( const ::com::sun::star::uno::Sequence< ::rtl::OUString >& aPropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setPropertyToDefault( const ::rtl::OUString& PropertyName ) throw(::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XServiceInfo
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

};

// ====================================================================

class SvxUnoTextRangeEnumeration : public ::cppu::WeakAggImplHelper1< ::com::sun::star::container::XEnumeration >
{
private:
    SvxEditSource*		pEditSource;
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > xParentText;
    const SvxUnoTextBase&	rParentText;
    sal_uInt16			nParagraph;
    SvUShorts*			pPortions;
    sal_uInt16			nNextPortion;

public:
    SvxUnoTextRangeEnumeration( const SvxUnoTextBase& rText, sal_uInt16 nPara ) throw();
    virtual ~SvxUnoTextRangeEnumeration() throw();

    // ::com::sun::star::container::XEnumeration
    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL nextElement(  ) throw(::com::sun::star::container::NoSuchElementException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
};

// ====================================================================

class SvxUnoTextCursor : public SvxUnoTextRangeBase,
                         public ::com::sun::star::text::XTextCursor,
                         public ::com::sun::star::lang::XTypeProvider,
                         public ::cppu::OWeakAggObject
{
private:
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XText > xParentText;

protected:
    static ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > maTypeSequence;

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
    virtual ::rtl::OUString SAL_CALL getString() throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setString( const ::rtl::OUString& aString ) throw(::com::sun::star::uno::RuntimeException);
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
    virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    // ::com::sun::star::lang::XTypeProvider
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes(  ) throw(::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getImplementationId(  ) throw(::com::sun::star::uno::RuntimeException);

};

const SfxItemPropertyMap* ImplGetSvxUnoOutlinerTextCursorPropertyMap();
const SfxItemPropertyMap* ImplGetSvxTextPortionPropertyMap();

}//end of namespace binfilter
#endif
