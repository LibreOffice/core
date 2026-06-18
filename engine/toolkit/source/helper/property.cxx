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

#include <helper/property.hxx>

#include <tools/debug.hxx>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/tree/XTreeDataModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <algorithm>
#include <string_view>
#include <utility>
#include <unordered_map>

using ::cpo::uno::Any;
using ::cpo::uno::Sequence;
using ::cpo::uno::Reference;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::FontDescriptor;
using ::com::sun::star::style::VerticalAlignment;
using ::com::sun::star::graphic::XGraphic;

using namespace com::sun::star;

namespace {

struct ImplPropertyInfo
{
    cpo::uno::Type           aType;
    sal_uInt16               nPropId;
    sal_Int16                nAttribs;
    bool                     bDependsOnOthers;   // eg. VALUE depends on MIN/MAX and must be set after MIN/MAX.

    ImplPropertyInfo( sal_uInt16 nId, const cpo::uno::Type& rType,
                        sal_Int16 nAttrs, bool bDepends = false )
         : aType(rType)
         , nPropId(nId)
         , nAttribs(nAttrs)
         , bDependsOnOthers(bDepends)
     {
     }

};

}

#define DECL_PROP_1( asciiname, id, type, attrib1 ) \
    { asciiname, ImplPropertyInfo( BASEPROPERTY_##id, cppu::UnoType<type>::get(), css::beans::PropertyAttribute::attrib1 ) }
#define DECL_PROP_2( asciiname, id, type, attrib1, attrib2 ) \
    { asciiname, ImplPropertyInfo( BASEPROPERTY_##id, cppu::UnoType<type>::get(), css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 ) }
#define DECL_PROP_3( asciiname, id, type, attrib1, attrib2, attrib3 ) \
    { asciiname, ImplPropertyInfo( BASEPROPERTY_##id, cppu::UnoType<type>::get(), css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3 ) }

#define DECL_DEP_PROP_2( asciiname, id, type, attrib1, attrib2 ) \
    { asciiname, ImplPropertyInfo( BASEPROPERTY_##id, cppu::UnoType<type>::get(), css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2, true ) }
#define DECL_DEP_PROP_3( asciiname, id, type, attrib1, attrib2, attrib3 ) \
    { asciiname, ImplPropertyInfo( BASEPROPERTY_##id, cppu::UnoType<type>::get(), css::beans::PropertyAttribute::attrib1 | css::beans::PropertyAttribute::attrib2 | css::beans::PropertyAttribute::attrib3, true ) }

typedef std::unordered_map<OUString, ImplPropertyInfo> ImpPropertyInfoMap;
static const ImpPropertyInfoMap & ImplGetPropertyInfos()
{
    static const ImpPropertyInfoMap aImplPropertyInfos {
        DECL_PROP_2     ( u"AccessibleName"_ustr,         ACCESSIBLENAME,         OUString,       BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"Align"_ustr,                  ALIGN,                  sal_Int16,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"Autocomplete"_ustr,           AUTOCOMPLETE,           bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"AutoHScroll"_ustr,            AUTOHSCROLL,            bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_1     ( u"AutoMnemonics"_ustr,          AUTOMNEMONICS,          bool,           BOUND ),
        DECL_PROP_2     ( u"AutoToggle"_ustr,             AUTOTOGGLE,             bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"AutoVScroll"_ustr,            AUTOVSCROLL,            bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"BackgroundColor"_ustr,        BACKGROUNDCOLOR,        sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_DEP_PROP_2 ( u"BlockIncrement"_ustr,         BLOCKINCREMENT,         sal_Int32,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"Border"_ustr,                 BORDER,                 sal_Int16,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_DEP_PROP_3 ( u"BorderColor"_ustr,            BORDERCOLOR,            sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"Closeable"_ustr,              CLOSEABLE,              bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"CurrencySymbol"_ustr,         CURRENCYSYMBOL,         OUString,       BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"CustomUnitText"_ustr,         CUSTOMUNITTEXT,         OUString,       BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_3 ( u"Date"_ustr,                   DATE,                   util::Date,     BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"DateFormat"_ustr,             EXTDATEFORMAT,          sal_Int16,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DateMax"_ustr,                DATEMAX,                util::Date,     BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DateMin"_ustr,                DATEMIN,                util::Date,     BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"DateShowCentury"_ustr,        DATESHOWCENTURY,        bool,           BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"DecimalAccuracy"_ustr,        DECIMALACCURACY,        sal_Int16,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DefaultButton"_ustr,          DEFAULTBUTTON,          bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DefaultControl"_ustr,         DEFAULTCONTROL,         OUString,       BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DesktopAsParent"_ustr,        DESKTOP_AS_PARENT,      bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DisplayBackgroundColor"_ustr, DISPLAYBACKGROUNDCOLOR, sal_Int32,      BOUND, MAYBEVOID ),
        DECL_PROP_2     ( u"Dropdown"_ustr,               DROPDOWN,               bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"EchoChar"_ustr,               ECHOCHAR,               sal_Int16,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"EditMask"_ustr,               EDITMASK,               OUString,       BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"EffectiveDefault"_ustr,       EFFECTIVE_DEFAULT,      Any,            BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"EffectiveMax"_ustr,           EFFECTIVE_MAX,          double,         BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"EffectiveMin"_ustr,           EFFECTIVE_MIN,          double,         BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_DEP_PROP_3 ( u"EffectiveValue"_ustr,         EFFECTIVE_VALUE,        Any,            BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"Enabled"_ustr,                ENABLED,                bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"EnforceFormat"_ustr,          ENFORCE_FORMAT,         bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"FillColor"_ustr,              FILLCOLOR,              sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"FocusOnClick"_ustr,           FOCUSONCLICK,           bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontRelief"_ustr,             FONTRELIEF,             sal_Int16,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontEmphasisMark"_ustr,       FONTEMPHASISMARK,       sal_Int16,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontDescriptor"_ustr,         FONTDESCRIPTOR,         FontDescriptor, BOUND, MAYBEDEFAULT ),

        // parts of css::awt::FontDescriptor
        DECL_PROP_2     ( u"FontName"_ustr,               FONTDESCRIPTORPART_NAME,         OUString,  BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontStyleName"_ustr,          FONTDESCRIPTORPART_STYLENAME,    OUString,  BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontFamily"_ustr,             FONTDESCRIPTORPART_FAMILY,       sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontCharset"_ustr,            FONTDESCRIPTORPART_CHARSET,      sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontHeight"_ustr,             FONTDESCRIPTORPART_HEIGHT,       float,     BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontWidth"_ustr,              FONTDESCRIPTORPART_WIDTH,        sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontPitch"_ustr,              FONTDESCRIPTORPART_PITCH,        sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontWeight"_ustr,             FONTDESCRIPTORPART_WEIGHT,       float,     BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontCharWidth"_ustr,          FONTDESCRIPTORPART_CHARWIDTH,    float,     BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontOrientation"_ustr,        FONTDESCRIPTORPART_ORIENTATION,  float,     BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontSlant"_ustr,              FONTDESCRIPTORPART_SLANT,        sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontUnderline"_ustr,          FONTDESCRIPTORPART_UNDERLINE,    sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontStrikeout"_ustr,          FONTDESCRIPTORPART_STRIKEOUT,    sal_Int16, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontKerning"_ustr,            FONTDESCRIPTORPART_KERNING,      bool,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontWordLineMode"_ustr,       FONTDESCRIPTORPART_WORDLINEMODE, bool,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"FontType"_ustr,               FONTDESCRIPTORPART_TYPE,         sal_Int16, BOUND, MAYBEDEFAULT ),

        DECL_PROP_3     ( u"FormatKey"_ustr,              FORMATKEY,          sal_Int32,      BOUND, MAYBEVOID, TRANSIENT ),
        DECL_PROP_3     ( u"FormatsSupplier"_ustr,        FORMATSSUPPLIER,    Reference< css::util::XNumberFormatsSupplier >, BOUND, MAYBEVOID, TRANSIENT ),

        DECL_PROP_2     ( u"Graphic"_ustr,                GRAPHIC,            Reference< XGraphic >, BOUND, TRANSIENT ),
        DECL_PROP_2     ( u"GroupName"_ustr,              GROUPNAME,          OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"HelpText"_ustr,               HELPTEXT,           OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"HelpURL"_ustr,                HELPURL,            OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"HideInactiveSelection"_ustr,  HIDEINACTIVESELECTION, bool,            BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"HighContrastMode"_ustr,       HIGHCONTRASTMODE,   bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"HScroll"_ustr,                HSCROLL,            bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"HardLineBreaks"_ustr,         HARDLINEBREAKS,     bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"HighlightColor"_ustr,         HIGHLIGHT_COLOR,    sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID),
        DECL_PROP_3     ( u"HighlightTextColor"_ustr,     HIGHLIGHT_TEXT_COLOR, sal_Int32,        BOUND, MAYBEDEFAULT, MAYBEVOID),
        DECL_PROP_2     ( u"ImageAlign"_ustr,             IMAGEALIGN,         sal_Int16,          BOUND, MAYBEDEFAULT),
        DECL_PROP_2     ( u"ImagePosition"_ustr,          IMAGEPOSITION,      sal_Int16,          BOUND, MAYBEDEFAULT),
        DECL_PROP_2     ( u"ImageURL"_ustr,               IMAGEURL,           cpo::uno::Any,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"ItemSeparatorPos"_ustr,       ITEM_SEPARATOR_POS, sal_Int16,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"Label"_ustr,                  LABEL,              OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"LineColor"_ustr,              LINECOLOR,          sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"LineCount"_ustr,              LINECOUNT,          sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"LineEndFormat"_ustr,          LINE_END_FORMAT,    sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_2 ( u"LineIncrement"_ustr,          LINEINCREMENT,      sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"LiteralMask"_ustr,            LITERALMASK,        OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"LiveScroll"_ustr,             LIVE_SCROLL,        bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"MaxTextLen"_ustr,             MAXTEXTLEN,         sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Moveable"_ustr,               MOVEABLE,           bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_1     ( u"MouseTransparent"_ustr,       MOUSETRANSPARENT,   bool,               BOUND ),
        DECL_PROP_2     ( u"MultiLine"_ustr,              MULTILINE,          bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"MultiSelection"_ustr,         MULTISELECTION,     bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"MultiSelectionSimpleMode"_ustr,   MULTISELECTION_SIMPLEMODE,    bool, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"NativeWidgetLook"_ustr,       NATIVE_WIDGET_LOOK, bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"NoLabel"_ustr,                NOLABEL,            bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Orientation"_ustr,            ORIENTATION,        sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"PaintTransparent"_ustr,       PAINTTRANSPARENT,   bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"PluginParent"_ustr,           PLUGINPARENT,       sal_Int64,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"PrependCurrencySymbol"_ustr,  CURSYM_POSITION,    bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Printable"_ustr,              PRINTABLE,          bool,               BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_3 ( u"ProgressValue"_ustr,          PROGRESSVALUE,      sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"ProgressValueMax"_ustr,       PROGRESSVALUE_MAX,  sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ProgressValueMin"_ustr,       PROGRESSVALUE_MIN,  sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"PushButtonType"_ustr,         PUSHBUTTONTYPE,     sal_Int16,          BOUND, MAYBEDEFAULT),
        DECL_PROP_2     ( u"ReadOnly"_ustr,               READONLY,           bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Repeat"_ustr,                 REPEAT,             bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"AutoRepeat"_ustr,             AUTO_REPEAT,        bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"RepeatDelay"_ustr,            REPEAT_DELAY,       sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ScaleImage"_ustr,             SCALEIMAGE,         bool,               BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_2 ( u"ScaleMode"_ustr,              IMAGE_SCALE_MODE,   sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_3 ( u"ScrollValue"_ustr,            SCROLLVALUE,        sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"ScrollValueMax"_ustr,         SCROLLVALUE_MAX,    sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ScrollValueMin"_ustr,         SCROLLVALUE_MIN,    sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ScrollWidth"_ustr,            SCROLLWIDTH,        sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ScrollHeight"_ustr,           SCROLLHEIGHT,       sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ScrollTop"_ustr,              SCROLLTOP,          sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ScrollLeft"_ustr,             SCROLLLEFT,         sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_2 ( u"SelectedItems"_ustr,          SELECTEDITEMS,      Sequence<sal_Int16>, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ShowThousandsSeparator"_ustr, NUMSHOWTHOUSANDSEP, bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Sizeable"_ustr,               SIZEABLE,           bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Spin"_ustr,                   SPIN,               bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"SpinIncrement"_ustr,          SPININCREMENT,      sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_2 ( u"SpinValue"_ustr,              SPINVALUE,          sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"SpinValueMax"_ustr,           SPINVALUE_MAX,      sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"SpinValueMin"_ustr,           SPINVALUE_MIN,      sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_2 ( u"State"_ustr,                  STATE,              sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"StrictFormat"_ustr,           STRICTFORMAT,       bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"StringItemList"_ustr,         STRINGITEMLIST,     Sequence< OUString >, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"TypedItemList"_ustr,          TYPEDITEMLIST,      Sequence< Any >,    BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"VisualEffect"_ustr,           VISUALEFFECT,       sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"SymbolColor"_ustr,            SYMBOL_COLOR,       sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"Tabstop"_ustr,                TABSTOP,            bool,               BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"Text"_ustr,                   TEXT,               OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"TextColor"_ustr,              TEXTCOLOR,          sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"TextLineColor"_ustr,          TEXTLINECOLOR,      sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_DEP_PROP_3 ( u"Time"_ustr,                   TIME,               util::Time,         BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"TimeFormat"_ustr,             EXTTIMEFORMAT,      sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"TimeMax"_ustr,                TIMEMAX,            util::Time,         BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"TimeMin"_ustr,                TIMEMIN,            util::Time,         BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Title"_ustr,                  TITLE,              OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Toggle"_ustr,                 TOGGLE,             bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"TreatAsNumber"_ustr,          TREATASNUMBER,      bool,               BOUND, MAYBEDEFAULT,TRANSIENT ),
        DECL_PROP_2     ( u"TriState"_ustr,               TRISTATE,           bool,               BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Unit"_ustr,                   UNIT,               sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"VScroll"_ustr,                VSCROLL,            bool,               BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_3 ( u"Value"_ustr,                  VALUE_DOUBLE,       double,             BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"ValueMax"_ustr,               VALUEMAX_DOUBLE,    double,             BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ValueMin"_ustr,               VALUEMIN_DOUBLE,    double,             BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ValueStep"_ustr,              VALUESTEP_DOUBLE,   double,             BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"VerticalAlign"_ustr,          VERTICALALIGN,      VerticalAlignment,  BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_DEP_PROP_3 ( u"VisibleSize"_ustr,            VISIBLESIZE,        sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"Activated"_ustr,              ACTIVATED,          bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Complete"_ustr,               COMPLETE,           bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"CurrentItemID"_ustr,          CURRENTITEMID,      sal_Int16,          BOUND, MAYBEDEFAULT ),

        DECL_PROP_2     ( u"MouseWheelBehavior"_ustr,     MOUSE_WHEEL_BEHAVIOUR,  sal_Int16,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"StepTime"_ustr,               STEP_TIME,              sal_Int32,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Decoration"_ustr,             DECORATION,             bool,       BOUND, MAYBEDEFAULT ),

        DECL_PROP_2     ( u"SelectionType"_ustr,          TREE_SELECTIONTYPE,     css::view::SelectionType, BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"Editable"_ustr,               TREE_EDITABLE,          bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"DataModel"_ustr,              TREE_DATAMODEL,         Reference< css::awt::tree::XTreeDataModel >,BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"RootDisplayed"_ustr,          TREE_ROOTDISPLAYED,     bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ShowsHandles"_ustr,           TREE_SHOWSHANDLES,      bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ShowsRootHandles"_ustr,       TREE_SHOWSROOTHANDLES,  bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"RowHeight"_ustr,              ROW_HEIGHT,             sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"InvokesStopNodeEditing"_ustr, TREE_INVOKESSTOPNODEEDITING, bool,      BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"DialogSourceURL"_ustr,        DIALOGSOURCEURL,        OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"URL"_ustr,                    URL,                    OUString,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"WritingMode"_ustr,            WRITING_MODE,           sal_Int16,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"ContextWritingMode"_ustr,     CONTEXT_WRITING_MODE,   sal_Int16,          BOUND, MAYBEDEFAULT, TRANSIENT ),
        DECL_PROP_2     ( u"ShowRowHeader"_ustr,          GRID_SHOWROWHEADER,     bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"RowHeaderWidth"_ustr,         ROW_HEADER_WIDTH,       sal_Int32,          BOUND, MAYBEDEFAULT ),
        DECL_PROP_2     ( u"ShowColumnHeader"_ustr,       GRID_SHOWCOLUMNHEADER,  bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"ColumnHeaderHeight"_ustr,     COLUMN_HEADER_HEIGHT,   sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_1     ( u"GridDataModel"_ustr,          GRID_DATAMODEL,         Reference< css::awt::grid::XGridDataModel >, BOUND ),
        DECL_PROP_1     ( u"ColumnModel"_ustr,            GRID_COLUMNMODEL,       Reference< css::awt::grid::XGridColumnModel >, BOUND ),
        DECL_PROP_3     ( u"SelectionModel"_ustr,         GRID_SELECTIONMODE,     css::view::SelectionType, BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"EnableVisible"_ustr,          ENABLEVISIBLE,          bool,           BOUND, MAYBEDEFAULT ),
        DECL_PROP_3     ( u"ReferenceDevice"_ustr,        REFERENCE_DEVICE,       Reference< XDevice >,BOUND, MAYBEDEFAULT, TRANSIENT ),
        DECL_PROP_3     ( u"HeaderBackgroundColor"_ustr,  GRID_HEADER_BACKGROUND, sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"HeaderTextColor"_ustr,        GRID_HEADER_TEXT_COLOR, sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"GridLineColor"_ustr,          GRID_LINE_COLOR,        sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"RowBackgroundColors"_ustr,    GRID_ROW_BACKGROUND_COLORS, Sequence< sal_Int32 >, BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_2     ( u"UseGridLines"_ustr,           USE_GRID_LINES,         bool,           BOUND, MAYBEDEFAULT ),
        DECL_DEP_PROP_3 ( u"MultiPageValue"_ustr,         MULTIPAGEVALUE,         sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"AllDialogChildren"_ustr,      USERFORMCONTAINEES,     Reference< css::container::XNameContainer >, BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"ActiveSelectionBackgroundColor"_ustr,   ACTIVE_SEL_BACKGROUND_COLOR,   sal_Int32, BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"InactiveSelectionBackgroundColor"_ustr, INACTIVE_SEL_BACKGROUND_COLOR, sal_Int32, BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"ActiveSelectionTextColor"_ustr,         ACTIVE_SEL_TEXT_COLOR,         sal_Int32, BOUND, MAYBEDEFAULT, MAYBEVOID ),
        DECL_PROP_3     ( u"InactiveSelectionTextColor"_ustr,       INACTIVE_SEL_TEXT_COLOR,       sal_Int32, BOUND, MAYBEDEFAULT, MAYBEVOID ),

        DECL_PROP_2(u"Referer"_ustr, REFERER, OUString, BOUND, MAYBEVOID),
    };
    return aImplPropertyInfos;
}

sal_uInt16 GetPropertyId( const OUString& rPropertyName )
{
    const ImpPropertyInfoMap & rMap = ImplGetPropertyInfos();
    auto it = rMap.find(rPropertyName);
    return it != rMap.end() ? it->second.nPropId : 0;
}

static const ImplPropertyInfo* ImplGetImplPropertyInfo( sal_uInt16 nPropertyId )
{
    const ImpPropertyInfoMap & rMap = ImplGetPropertyInfos();

    for (auto const & rPair : rMap)
        if (rPair.second.nPropId == nPropertyId)
            return &rPair.second;
    return nullptr;
}

const OUString& GetPropertyName( sal_uInt16 nPropertyId )
{
    const ImpPropertyInfoMap & rMap = ImplGetPropertyInfos();

    for (auto const & rPair : rMap)
        if (rPair.second.nPropId == nPropertyId)
            return rPair.first;

    assert(false && "Invalid PropertyId!");
    static const OUString EMPTY;
    return EMPTY;
}

const cpo::uno::Type* GetPropertyType( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? &pImplPropertyInfo->aType : nullptr;
}

sal_Int16 GetPropertyAttribs( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? pImplPropertyInfo->nAttribs : 0;
}

bool DoesDependOnOthers( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo && pImplPropertyInfo->bDependsOnOthers;
}

bool CompareProperties( const cpo::uno::Any& r1, const cpo::uno::Any& r2 )
{
    return r1 == r2;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
