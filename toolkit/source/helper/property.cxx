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

#include <toolkit/helper/property.hxx>
#include <toolkit/helper/macros.hxx>
#include <osl/mutex.hxx>

#include <stdlib.h> // qsort/bsearch
#include <tools/debug.hxx>
#include <com/sun/star/awt/FontWeight.hpp>
#include <com/sun/star/awt/FontSlant.hpp>
#include <com/sun/star/awt/CharSet.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/FontType.hpp>
#include <com/sun/star/awt/FontUnderline.hpp>
#include <com/sun/star/awt/FontStrikeout.hpp>
#include <com/sun/star/awt/FontPitch.hpp>
#include <com/sun/star/awt/XDevice.hpp>
#include <com/sun/star/awt/tree/XTreeDataModel.hpp>
#include <com/sun/star/awt/grid/XGridDataModel.hpp>
#include <com/sun/star/awt/grid/XGridColumnModel.hpp>
#include <com/sun/star/view/SelectionType.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/util/XNumberFormatsSupplier.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/resource/XStringResourceResolver.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <comphelper/types.hxx>
#include <functional>
#include <algorithm>
#include <toolkit/helper/property.hxx>

using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::awt::XDevice;
using ::com::sun::star::awt::FontDescriptor;
using ::com::sun::star::style::VerticalAlignment;
using ::com::sun::star::graphic::XGraphic;

struct ImplPropertyInfo
{
    ::rtl::OUString                 aName;
    sal_uInt16                      nPropId;
    ::com::sun::star::uno::Type     aType;
    sal_Int16                       nAttribs;
    sal_Bool                        bDependsOnOthers;   // eg. VALUE depends on MIN/MAX and must be set after MIN/MAX.

    ImplPropertyInfo()
     {
         nPropId = 0;
        nAttribs = 0;
           bDependsOnOthers = sal_False;
     }

    ImplPropertyInfo( rtl::OUString const & theName, sal_uInt16 nId, const ::com::sun::star::uno::Type& rType,
                        sal_Int16 nAttrs, sal_Bool bDepends = sal_False )
     : aName( theName )
     {
         nPropId = nId;
        aType = rType;
        nAttribs = nAttrs;
           bDependsOnOthers = bDepends;
     }

};

#define DECL_PROP_1( asciiname, id, type, attrib1 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1 )
#define DECL_PROP_2( asciiname, id, type, attrib1, attrib2 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2 )
#define DECL_PROP_3( asciiname, id, type, attrib1, attrib2, attrib3 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2 | ::com::sun::star::beans::PropertyAttribute::attrib3 )
#define DECL_PROP_4( asciiname, id, type, attrib1, attrib2, attrib3, attrib4 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2 | ::com::sun::star::beans::PropertyAttribute::attrib3 | ::com::sun::star::beans::PropertyAttribute::attrib4 )

#define DECL_DEP_PROP_1( asciiname, id, type, attrib1 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1, sal_True )
#define DECL_DEP_PROP_2( asciiname, id, type, attrib1, attrib2 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2, sal_True )
#define DECL_DEP_PROP_3( asciiname, id, type, attrib1, attrib2, attrib3 ) \
    ImplPropertyInfo( ::rtl::OUString::createFromAscii( asciiname ), BASEPROPERTY_##id, ::getCppuType( static_cast< const type* >( NULL ) ), ::com::sun::star::beans::PropertyAttribute::attrib1 | ::com::sun::star::beans::PropertyAttribute::attrib2 | ::com::sun::star::beans::PropertyAttribute::attrib3, sal_True )

ImplPropertyInfo* ImplGetPropertyInfos( sal_uInt16& rElementCount )
{
    static ImplPropertyInfo* pPropertyInfos = NULL;
    static sal_uInt16 nElements = 0;
    if( !pPropertyInfos )
    {
        ::osl::Guard< ::osl::Mutex > aGuard( ::osl::Mutex::getGlobalMutex() );
        if( !pPropertyInfos )
        {
            static ImplPropertyInfo aImplPropertyInfos [] =
            {
            DECL_PROP_2     ( "AccessibleName",         ACCESSIBLENAME,     ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "Align",                  ALIGN,              sal_Int16,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "Autocomplete",           AUTOCOMPLETE,       bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "AutoHScroll",            AUTOHSCROLL,        bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_1     ( "AutoMnemonics",          AUTOMNEMONICS,      bool,               BOUND ),
            DECL_PROP_2     ( "AutoToggle",             AUTOTOGGLE,         bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "AutoVScroll",            AUTOVSCROLL,        bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "BackgroundColor",        BACKGROUNDCOLOR,    sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_DEP_PROP_2 ( "BlockIncrement",         BLOCKINCREMENT,     sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "Border",                 BORDER,             sal_Int16,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_DEP_PROP_3 ( "BorderColor",            BORDERCOLOR,        sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "Closeable",              CLOSEABLE,          bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "CurrencySymbol",         CURRENCYSYMBOL,     ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "CustomUnitText",         CUSTOMUNITTEXT,     ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_3 ( "Date",                   DATE,               sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "DateFormat",             EXTDATEFORMAT,      sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DateMax",                DATEMAX,            sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DateMin",                DATEMIN,            sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "DateShowCentury",        DATESHOWCENTURY,    bool,               BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "DecimalAccuracy",        DECIMALACCURACY,    sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DefaultButton",          DEFAULTBUTTON,      bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DefaultControl",         DEFAULTCONTROL,     ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DesktopAsParent",        DESKTOP_AS_PARENT,  bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DisplayBackgroundColor", DISPLAYBACKGROUNDCOLOR, sal_Int32, BOUND, MAYBEVOID ),
            DECL_PROP_2     ( "Dropdown",               DROPDOWN,           bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "EchoChar",               ECHOCHAR,           sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "EditMask",               EDITMASK,           ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "EffectiveDefault",       EFFECTIVE_DEFAULT, Any,                BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "EffectiveMax",           EFFECTIVE_MAX,      double,             BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "EffectiveMin",           EFFECTIVE_MIN,      double,             BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_DEP_PROP_3 ( "EffectiveValue",         EFFECTIVE_VALUE,    Any,                BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "Enabled",                ENABLED,            bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "EnforceFormat",          ENFORCE_FORMAT,     bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "FillColor",              FILLCOLOR,          sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "FocusOnClick",           FOCUSONCLICK,       bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontRelief",             FONTRELIEF,         sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontEmphasisMark",       FONTEMPHASISMARK,   sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontDescriptor",         FONTDESCRIPTOR,     FontDescriptor,     BOUND, MAYBEDEFAULT ),

            // Teile des ::com::sun::star::awt::FontDescriptor
            DECL_PROP_2     ( "FontName",               FONTDESCRIPTORPART_NAME,         ::rtl::OUString,BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontStyleName",          FONTDESCRIPTORPART_STYLENAME,    ::rtl::OUString,BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontFamily",             FONTDESCRIPTORPART_FAMILY,       sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontCharset",            FONTDESCRIPTORPART_CHARSET,      sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontHeight",             FONTDESCRIPTORPART_HEIGHT,       float,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontWidth",              FONTDESCRIPTORPART_WIDTH,        sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontPitch",              FONTDESCRIPTORPART_PITCH,        sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontWeight",             FONTDESCRIPTORPART_WEIGHT,       float,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontCharWidth",          FONTDESCRIPTORPART_CHARWIDTH,    float,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontOrientation",        FONTDESCRIPTORPART_ORIENTATION,  float,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontSlant",              FONTDESCRIPTORPART_SLANT,        sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontUnderline",          FONTDESCRIPTORPART_UNDERLINE,    sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontStrikeout",          FONTDESCRIPTORPART_STRIKEOUT,    sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontKerning",            FONTDESCRIPTORPART_KERNING,      bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontWordLineMode",       FONTDESCRIPTORPART_WORDLINEMODE, bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "FontType",               FONTDESCRIPTORPART_TYPE,         sal_Int16,      BOUND, MAYBEDEFAULT ),

            DECL_PROP_3     ( "FormatKey",              FORMATKEY,          sal_Int32,      BOUND, MAYBEVOID, TRANSIENT ),
            DECL_PROP_3     ( "FormatsSupplier",        FORMATSSUPPLIER,    Reference< ::com::sun::star::util::XNumberFormatsSupplier >, BOUND, MAYBEVOID, TRANSIENT ),

            DECL_PROP_2     ( "Graphic",                GRAPHIC,            Reference< XGraphic >, BOUND, TRANSIENT ),
            DECL_PROP_2     ( "GroupName",              GROUPNAME,          ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "HelpText",               HELPTEXT,           ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "HelpURL",                HELPURL,            ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "HideInactiveSelection",  HIDEINACTIVESELECTION, bool,            BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "HighContrastMode",       HIGHCONTRASTMODE,   bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "HScroll",                HSCROLL,            bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "HardLineBreaks",         HARDLINEBREAKS,     bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ImageAlign",             IMAGEALIGN,         sal_Int16,          BOUND, MAYBEDEFAULT),
            DECL_PROP_2     ( "ImagePosition",          IMAGEPOSITION,      sal_Int16,          BOUND, MAYBEDEFAULT),
            DECL_PROP_2     ( "ImageURL",               IMAGEURL,           ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "ItemSeparatorPos",       ITEM_SEPARATOR_POS, sal_Int16,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "Label",                  LABEL,              ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "LineColor",              LINECOLOR,          sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "LineCount",              LINECOUNT,          sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "LineEndFormat",          LINE_END_FORMAT,    sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_2 ( "LineIncrement",          LINEINCREMENT,      sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "LiteralMask",            LITERALMASK,        ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "LiveScroll",             LIVE_SCROLL,        bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "MaxTextLen",             MAXTEXTLEN,         sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Moveable",               MOVEABLE,           bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_1     ( "MouseTransparent",       MOUSETRANSPARENT,   bool,               BOUND ),
            DECL_PROP_2     ( "MultiLine",              MULTILINE,          bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "MultiSelection",         MULTISELECTION,     bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "MultiSelectionSimpleMode",   MULTISELECTION_SIMPLEMODE,    bool, BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "NativeWidgetLook",       NATIVE_WIDGET_LOOK, bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "NoLabel",                NOLABEL,            bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Orientation",            ORIENTATION,        sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "PaintTransparent",       PAINTTRANSPARENT,   bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "PluginParent",           PLUGINPARENT,       sal_Int64,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "PrependCurrencySymbol",  CURSYM_POSITION,    bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Printable",              PRINTABLE,          bool,               BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_3 ( "ProgressValue",          PROGRESSVALUE,      sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "ProgressValueMax",       PROGRESSVALUE_MAX,  sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ProgressValueMin",       PROGRESSVALUE_MIN,  sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "PushButtonType",         PUSHBUTTONTYPE,     sal_Int16,          BOUND, MAYBEDEFAULT),
            DECL_PROP_2     ( "ReadOnly",               READONLY,           bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Repeat",                 REPEAT,             bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "AutoRepeat",             AUTO_REPEAT,        sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "RepeatDelay",            REPEAT_DELAY,       sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScaleImage",             SCALEIMAGE,         bool,               BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScaleMode",              IMAGE_SCALE_MODE,   sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_3 ( "ScrollValue",            SCROLLVALUE,        sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "ScrollValueMax",         SCROLLVALUE_MAX,    sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScrollValueMin",         SCROLLVALUE_MIN,    sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScrollWidth",            SCROLLWIDTH,        sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScrollHeight",           SCROLLHEIGHT,       sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScrollTop",              SCROLLTOP,          sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ScrollLeft",             SCROLLLEFT,         sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_2 ( "SelectedItems",          SELECTEDITEMS,      Sequence<sal_Int16>, BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ShowThousandsSeparator", NUMSHOWTHOUSANDSEP,     bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Sizeable",               SIZEABLE,               bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Spin",                   SPIN,                   bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "SpinIncrement",          SPININCREMENT,          sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_2 ( "SpinValue",              SPINVALUE,              sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "SpinValueMax",           SPINVALUE_MAX,          sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "SpinValueMin",           SPINVALUE_MIN,          sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_2 ( "State",                  STATE,                  sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "StrictFormat",           STRICTFORMAT,           bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "StringItemList",         STRINGITEMLIST,         Sequence< ::rtl::OUString >, BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "VisualEffect",           VISUALEFFECT,           sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "SymbolColor",            SYMBOL_COLOR,           sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "Tabstop",                TABSTOP,                bool,           BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "Text",                   TEXT,                   ::rtl::OUString, BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "TextColor",              TEXTCOLOR,              sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "TextLineColor",          TEXTLINECOLOR,          sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_DEP_PROP_3 ( "Time",                   TIME,                   sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "TimeFormat",             EXTTIMEFORMAT,          sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "TimeMax",                TIMEMAX,                sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "TimeMin",                TIMEMIN,                sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Title",                  TITLE,                  ::rtl::OUString, BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Toggle",                 TOGGLE,                 bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "TreatAsNumber",          TREATASNUMBER,          bool,           BOUND, MAYBEDEFAULT,TRANSIENT ),
            DECL_PROP_2     ( "TriState",               TRISTATE,               bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Unit",                   UNIT,                   sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "VScroll",                VSCROLL,                bool,           BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_3 ( "Value",                  VALUE_DOUBLE,           double,         BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "ValueMax",               VALUEMAX_DOUBLE,        double,         BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ValueMin",               VALUEMIN_DOUBLE,        double,         BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ValueStep",              VALUESTEP_DOUBLE,       double,         BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "VerticalAlign",          VERTICALALIGN,          VerticalAlignment, BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_DEP_PROP_3 ( "VisibleSize",            VISIBLESIZE,            sal_Int32,      BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "Activated",              ACTIVATED,              sal_Bool,       BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Complete",               COMPLETE,               sal_Bool,       BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "CurrentItemID",          CURRENTITEMID,          sal_Int16,      BOUND, MAYBEDEFAULT ),

            DECL_PROP_2     ( "MouseWheelBehavior",     MOUSE_WHEEL_BEHAVIOUR,  sal_Int16,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "StepTime",               STEP_TIME,              sal_Int32,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Decoration",             DECORATION,             sal_Bool,       BOUND, MAYBEDEFAULT ),

            DECL_PROP_2     ( "SelectionType",          TREE_SELECTIONTYPE,     ::com::sun::star::view::SelectionType,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "Editable",               TREE_EDITABLE,          sal_Bool,       BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "DataModel",              TREE_DATAMODEL,         Reference< ::com::sun::star::awt::tree::XTreeDataModel >,       BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "RootDisplayed",          TREE_ROOTDISPLAYED,     sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ShowsHandles",           TREE_SHOWSHANDLES,      sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ShowsRootHandles",       TREE_SHOWSROOTHANDLES,  sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "RowHeight",              ROW_HEIGHT,             sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "InvokesStopNodeEditing", TREE_INVOKESSTOPNODEEDITING, sal_Bool,      BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "DialogSourceURL",        DIALOGSOURCEURL,        ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "URL",                    URL,                    ::rtl::OUString,    BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "WritingMode",            WRITING_MODE,           sal_Int16,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "ContextWritingMode",     CONTEXT_WRITING_MODE,   sal_Int16,          BOUND, MAYBEDEFAULT, TRANSIENT ),
            DECL_PROP_2     ( "ShowRowHeader",          GRID_SHOWROWHEADER,     sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "RowHeaderWidth",         ROW_HEADER_WIDTH,       sal_Int32,          BOUND, MAYBEDEFAULT ),
            DECL_PROP_2     ( "ShowColumnHeader",       GRID_SHOWCOLUMNHEADER,  sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "ColumnHeaderHeight",     COLUMN_HEADER_HEIGHT,   sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_1     ( "GridDataModel",          GRID_DATAMODEL,         Reference< ::com::sun::star::awt::grid::XGridDataModel >,          BOUND ),
            DECL_PROP_1     ( "ColumnModel",            GRID_COLUMNMODEL,       Reference< ::com::sun::star::awt::grid::XGridColumnModel >,          BOUND ),
            DECL_PROP_3     ( "SelectionModel",         GRID_SELECTIONMODE,     ::com::sun::star::view::SelectionType,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "EnableVisible",          ENABLEVISIBLE,          sal_Bool,           BOUND, MAYBEDEFAULT ),
            DECL_PROP_3     ( "ReferenceDevice",        REFERENCE_DEVICE,       Reference< XDevice >,BOUND, MAYBEDEFAULT, TRANSIENT ),
            DECL_PROP_3     ( "HeaderBackgroundColor",  GRID_HEADER_BACKGROUND,     sal_Int32,              BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "HeaderTextColor",        GRID_HEADER_TEXT_COLOR,     sal_Int32,              BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "GridLineColor",          GRID_LINE_COLOR,            sal_Int32,              BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "RowBackgroundColors",    GRID_ROW_BACKGROUND_COLORS, Sequence< sal_Int32 >,  BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_2     ( "UseGridLines",           USE_GRID_LINES,             sal_Bool,               BOUND, MAYBEDEFAULT ),
            DECL_DEP_PROP_3 ( "MultiPageValue",          MULTIPAGEVALUE,      sal_Int32,          BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "AllDialogChildren",                USERFORMCONTAINEES,                Reference< ::com::sun::star::container::XNameContainer >,           BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "ActiveSelectionBackgroundColor",     ACTIVE_SEL_BACKGROUND_COLOR,    sal_Int32,  BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "InactiveSelectionBackgroundColor",   INACTIVE_SEL_BACKGROUND_COLOR,  sal_Int32,  BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "ActiveSelectionTextColor",           ACTIVE_SEL_TEXT_COLOR,          sal_Int32,  BOUND, MAYBEDEFAULT, MAYBEVOID ),
            DECL_PROP_3     ( "InactiveSelectionTextColor",         INACTIVE_SEL_TEXT_COLOR,        sal_Int32,  BOUND, MAYBEDEFAULT, MAYBEVOID ),
    };
            pPropertyInfos = aImplPropertyInfos;
            nElements = sizeof( aImplPropertyInfos ) / sizeof( ImplPropertyInfo );
        }
    }
    rElementCount = nElements;
    return pPropertyInfos;
}


struct ImplPropertyInfoCompareFunctor : ::std::binary_function<ImplPropertyInfo,::rtl::OUString,bool>
{
    inline bool operator()(const ImplPropertyInfo& lhs,const ImplPropertyInfo& rhs) const
    {
        return lhs.aName.compareTo(rhs.aName) < 0;
    }
    inline bool operator()(const ImplPropertyInfo& lhs,const ::rtl::OUString& rhs)  const
    {
        return lhs.aName.compareTo(rhs) < 0;
    }
    inline bool operator()(const ::rtl::OUString& lhs,const ImplPropertyInfo& rhs)  const
    {
        return lhs.compareTo(rhs.aName) < 0;
    }
};

void ImplAssertValidPropertyArray()
{
    static sal_Bool bSorted = sal_False;
    if( !bSorted )
    {
        sal_uInt16 nElements;
        ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
        ::std::sort(pInfos, pInfos+nElements,ImplPropertyInfoCompareFunctor());
        bSorted = sal_True;
    }
}

sal_uInt16 GetPropertyId( const ::rtl::OUString& rPropertyName )
{
    ImplAssertValidPropertyArray();

    sal_uInt16 nElements;
    ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
    ImplPropertyInfo* pInf = ::std::lower_bound(pInfos,pInfos+nElements,rPropertyName,ImplPropertyInfoCompareFunctor());
/*
        (ImplPropertyInfo*)
                                bsearch( &aSearch, pInfos, nElements, sizeof( ImplPropertyInfo ), ImplPropertyInfoCompare );
*/

    return ( pInf && pInf != (pInfos+nElements) && pInf->aName == rPropertyName) ? pInf->nPropId: 0;
}

const ImplPropertyInfo* ImplGetImplPropertyInfo( sal_uInt16 nPropertyId )
{
    ImplAssertValidPropertyArray();

    sal_uInt16 nElements;
    ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
    sal_uInt16 n;
    for ( n = 0; n < nElements && pInfos[n].nPropId != nPropertyId; ++n)
        ;

    return (n < nElements) ? &pInfos[n] : NULL;
}

sal_uInt16 GetPropertyOrderNr( sal_uInt16 nPropertyId )
{
    ImplAssertValidPropertyArray();

    sal_uInt16 nElements;
    ImplPropertyInfo* pInfos = ImplGetPropertyInfos( nElements );
    for ( sal_uInt16 n = nElements; n; )
    {
        if ( pInfos[--n].nPropId == nPropertyId )
            return n;
    }
    return 0xFFFF;
}

const ::rtl::OUString& GetPropertyName( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo->aName;
}

const ::com::sun::star::uno::Type* GetPropertyType( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? &pImplPropertyInfo->aType : NULL;
}

sal_Int16 GetPropertyAttribs( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? pImplPropertyInfo->nAttribs : 0;
}

sal_Bool DoesDependOnOthers( sal_uInt16 nPropertyId )
{
    const ImplPropertyInfo* pImplPropertyInfo = ImplGetImplPropertyInfo( nPropertyId );
    DBG_ASSERT( pImplPropertyInfo, "Invalid PropertyId!" );
    return pImplPropertyInfo ? pImplPropertyInfo->bDependsOnOthers : sal_False;
}

sal_Bool CompareProperties( const ::com::sun::star::uno::Any& r1, const ::com::sun::star::uno::Any& r2 )
{
    return ::comphelper::compare( r1, r2 );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
