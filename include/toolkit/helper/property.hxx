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

#ifndef INCLUDED_TOOLKIT_HELPER_PROPERTY_HXX
#define INCLUDED_TOOLKIT_HELPER_PROPERTY_HXX

#include <toolkit/dllapi.h>

#include <sal/types.h>
#include <rtl/ustring.hxx>

namespace com {
namespace sun {
namespace star {
namespace uno {
    class Type;
    class Any;
} } } }


#define BASEPROPERTY_NOTFOUND                        0

#define BASEPROPERTY_TEXT                            1  // OUString
#define BASEPROPERTY_BACKGROUNDCOLOR                 2  // sal_Int32
#define BASEPROPERTY_FILLCOLOR                       3  // sal_Int32
#define BASEPROPERTY_TEXTCOLOR                       4  // sal_Int32
#define BASEPROPERTY_LINECOLOR                       5  // sal_Int32
#define BASEPROPERTY_BORDER                          6  // sal_Int16
#define BASEPROPERTY_ALIGN                           7  // sal_Int16
#define BASEPROPERTY_FONTDESCRIPTOR                  8  // css::awt::FontDescriptor
#define BASEPROPERTY_DROPDOWN                        9  // sal_Bool
#define BASEPROPERTY_MULTILINE                       10 // sal_Bool
#define BASEPROPERTY_STRINGITEMLIST                  11 // UStringSequence
#define BASEPROPERTY_HSCROLL                         12 // sal_Bool
#define BASEPROPERTY_VSCROLL                         13 // sal_Bool
#define BASEPROPERTY_TABSTOP                         14 // sal_Bool
#define BASEPROPERTY_STATE                           15 // sal_Int16
#define BASEPROPERTY_FONT_TYPE                       16 // OLD: Font_Type
#define BASEPROPERTY_FONT_SIZE                       17 // OLD: Font_Size
#define BASEPROPERTY_FONT_ATTRIBS                    18 // OLD: Font_Attribs
#define BASEPROPERTY_DEFAULTCONTROL                  19 // OUString (ServiceName)
#define BASEPROPERTY_LABEL                           20 // OUString
#define BASEPROPERTY_LINECOUNT                       21 // sal_Int16
#define BASEPROPERTY_EXTDATEFORMAT                   22 // sal_Int16
#define BASEPROPERTY_DATESHOWCENTURY                 23 // sal_Bool
#define BASEPROPERTY_EXTTIMEFORMAT                   24 // sal_Int16
#define BASEPROPERTY_NUMSHOWTHOUSANDSEP              25 // sal_Bool
#define BASEPROPERTY_CURRENCYSYMBOL                  26 // OUString
#define BASEPROPERTY_SPIN                            27 // sal_Bool
#define BASEPROPERTY_STRICTFORMAT                    28 // sal_Bool
#define BASEPROPERTY_DECIMALACCURACY                 29 // sal_Int16
#define BASEPROPERTY_DATE                            30 // css::util::Date
#define BASEPROPERTY_DATEMIN                         31 // css::util::Date
#define BASEPROPERTY_DATEMAX                         32 // css::util::Date
#define BASEPROPERTY_TIME                            33 // css::util::Time
#define BASEPROPERTY_TIMEMIN                         34 // css::util::Time
#define BASEPROPERTY_TIMEMAX                         35 // css::util::Time
#define BASEPROPERTY_VALUE_INT32                     36 // sal_Int32
#define BASEPROPERTY_VALUEMIN_INT32                  37 // sal_Int32
#define BASEPROPERTY_VALUEMAX_INT32                  38 // sal_Int32
#define BASEPROPERTY_VALUESTEP_INT32                 39 // sal_Int32
#define BASEPROPERTY_EDITMASK                        40 // OUString
#define BASEPROPERTY_LITERALMASK                     41 // OUString
#define BASEPROPERTY_IMAGEURL                        42 // OUString
#define BASEPROPERTY_READONLY                        43 // sal_Bool
#define BASEPROPERTY_ENABLED                         44 // sal_Bool
#define BASEPROPERTY_PRINTABLE                       45 // sal_Bool
#define BASEPROPERTY_ECHOCHAR                        46 // sal_Int16
#define BASEPROPERTY_MAXTEXTLEN                      47 // sal_Int16
#define BASEPROPERTY_HARDLINEBREAKS                  48 // sal_Int16
#define BASEPROPERTY_AUTOCOMPLETE                    49 // sal_Bool
#define BASEPROPERTY_MULTISELECTION                  50 // sal_Bool
#define BASEPROPERTY_SELECTEDITEMS                   51 // INT16Sequence
#define BASEPROPERTY_VALUE_DOUBLE                    52 // DOUBLE
#define BASEPROPERTY_VALUEMIN_DOUBLE                 53 // DOUBLE
#define BASEPROPERTY_VALUEMAX_DOUBLE                 54 // DOUBLE
#define BASEPROPERTY_VALUESTEP_DOUBLE                55 // DOUBLE
#define BASEPROPERTY_TRISTATE                        56 // sal_Bool
#define BASEPROPERTY_DEFAULTBUTTON                   57 // sal_Bool
#define BASEPROPERTY_HELPURL                         58 // OUString
#define BASEPROPERTY_AUTOTOGGLE                      59 // sal_Bool
//#define BASEPROPERTY_FOCUSSELECTIONHIDE            60 // sal_Bool
#define BASEPROPERTY_FORMATKEY                       61 // sal_Bool
#define BASEPROPERTY_FORMATSSUPPLIER                 62 // css::util::XNumberFormatsSupplier
#define BASEPROPERTY_EFFECTIVE_VALUE                 63 // Any (double or string)
#define BASEPROPERTY_TREATASNUMBER                   64 // sal_Bool
#define BASEPROPERTY_EFFECTIVE_DEFAULT               65 // Any (double or string)
#define BASEPROPERTY_EFFECTIVE_MIN                   66 // Double
#define BASEPROPERTY_EFFECTIVE_MAX                   67 // Double
#define BASEPROPERTY_CURSYM_POSITION                 68 // sal_Bool
#define BASEPROPERTY_TITLE                           69 // OUString
#define BASEPROPERTY_MOVEABLE                        70 // sal_Bool
#define BASEPROPERTY_CLOSEABLE                       71 // sal_Bool
#define BASEPROPERTY_SIZEABLE                        72 // sal_Bool
#define BASEPROPERTY_HELPTEXT                        73 // OUString
#define BASEPROPERTY_PROGRESSVALUE                   74 // sal_Int32
#define BASEPROPERTY_PROGRESSVALUE_MIN               75 // sal_Int32
#define BASEPROPERTY_PROGRESSVALUE_MAX               76 // sal_Int32
#define BASEPROPERTY_SCROLLVALUE                     77 // sal_Int32
#define BASEPROPERTY_SCROLLVALUE_MAX                 78 // sal_Int32
#define BASEPROPERTY_LINEINCREMENT                   79 // sal_Int32
#define BASEPROPERTY_BLOCKINCREMENT                  80 // sal_Int32
#define BASEPROPERTY_VISIBLESIZE                     81 // sal_Int32
#define BASEPROPERTY_ORIENTATION                     82 // sal_Int32
#define BASEPROPERTY_FONTRELIEF                      83 // sal_Int16
#define BASEPROPERTY_FONTEMPHASISMARK                84 // sal_Int16
#define BASEPROPERTY_TEXTLINECOLOR                   85 // sal_Int32
#define BASEPROPERTY_IMAGEALIGN                      86 // sal_Int16
#define BASEPROPERTY_SCALEIMAGE                      87 // sal_Bool
#define BASEPROPERTY_PUSHBUTTONTYPE                  88 // sal_Int16
#define BASEPROPERTY_DISPLAYBACKGROUNDCOLOR          89 // sal_Int32
#define BASEPROPERTY_AUTOMNEMONICS                   90 // sal_Bool
#define BASEPROPERTY_MOUSETRANSPARENT                91 // sal_Bool
#define BASEPROPERTY_ACCESSIBLENAME                  92  // OUString
#define BASEPROPERTY_PLUGINPARENT                    93  // sal_Int64
#define BASEPROPERTY_SCROLLVALUE_MIN                 94  // sal_Int32
#define BASEPROPERTY_REPEAT_DELAY                    95  // sal_Int32
#define BASEPROPERTY_SYMBOL_COLOR                    96  // sal_Int32
#define BASEPROPERTY_SPINVALUE                       97  // sal_Int32
#define BASEPROPERTY_SPINVALUE_MIN                   98  // sal_Int32
#define BASEPROPERTY_SPINVALUE_MAX                   99  // sal_Int32
#define BASEPROPERTY_SPININCREMENT                  100  // sal_Int32
#define BASEPROPERTY_REPEAT                         101  // sal_Bool
#define BASEPROPERTY_ENFORCE_FORMAT                 102  // sal_Bool
#define BASEPROPERTY_LIVE_SCROLL                    103  // sal_Bool
#define BASEPROPERTY_LINE_END_FORMAT                104  // sal_Int16
#define BASEPROPERTY_ACTIVATED                      105  // sal Bool
#define BASEPROPERTY_COMPLETE                       106  // sal_Bool
#define BASEPROPERTY_CURRENTITEMID                  107  // sal_Int16
#define BASEPROPERTY_TOGGLE                         108  // sal_Bool
#define BASEPROPERTY_FOCUSONCLICK                   109  // sal_Bool
#define BASEPROPERTY_HIDEINACTIVESELECTION          110  // sal_Bool
#define BASEPROPERTY_VISUALEFFECT                   111  // sal_Int16
#define BASEPROPERTY_BORDERCOLOR                    112  // sal_Int32
#define BASEPROPERTY_IMAGEPOSITION                  113  // sal_Int16
#define BASEPROPERTY_NATIVE_WIDGET_LOOK             114  // sal_Bool
#define BASEPROPERTY_VERTICALALIGN                  115  // VerticalAlignment
#define BASEPROPERTY_MOUSE_WHEEL_BEHAVIOUR          116  // sal_Int16
#define BASEPROPERTY_GRAPHIC                        117  // css.graphic.XGraphic
#define BASEPROPERTY_STEP_TIME                      118  // sal_Int32
#define BASEPROPERTY_DECORATION                     119  // sal_Bool
#define BASEPROPERTY_PAINTTRANSPARENT               120  // sal_Bool
#define BASEPROPERTY_AUTOHSCROLL                    121 // sal_Bool
#define BASEPROPERTY_AUTOVSCROLL                    122  // sal_Bool
#define BASEPROPERTY_DESKTOP_AS_PARENT              123  // sal_Bool
#define BASEPROPERTY_TREE_START                     124
#define BASEPROPERTY_TREE_SELECTIONTYPE             124
#define BASEPROPERTY_TREE_EDITABLE                  125
#define BASEPROPERTY_TREE_DATAMODEL                 126
#define BASEPROPERTY_TREE_ROOTDISPLAYED             127
#define BASEPROPERTY_TREE_SHOWSHANDLES              128
#define BASEPROPERTY_TREE_SHOWSROOTHANDLES          129
#define BASEPROPERTY_ROW_HEIGHT                     130
#define BASEPROPERTY_TREE_INVOKESSTOPNODEEDITING    131
#define BASEPROPERTY_TREE_END                       131
#define BASEPROPERTY_DIALOGSOURCEURL                132
#define BASEPROPERTY_NOLABEL                        133  // OUString  added for issue79712
#define BASEPROPERTY_URL                            134  // OUString
#define BASEPROPERTY_UNIT                           135  // ::awt::FieldUnit
#define BASEPROPERTY_CUSTOMUNITTEXT                 136  // OUString
#define BASEPROPERTY_IMAGE_SCALE_MODE               137
#define BASEPROPERTY_WRITING_MODE                   138
#define BASEPROPERTY_CONTEXT_WRITING_MODE           139
#define BASEPROPERTY_GRID_SHOWROWHEADER             140
#define BASEPROPERTY_GRID_SHOWCOLUMNHEADER          141
#define BASEPROPERTY_GRID_DATAMODEL                 142
#define BASEPROPERTY_GRID_COLUMNMODEL               143
#define BASEPROPERTY_GRID_SELECTIONMODE             144
#define BASEPROPERTY_ENABLEVISIBLE                  145  // sal_Bool
#define BASEPROPERTY_REFERENCE_DEVICE               146

#define BASEPROPERTY_HIGHCONTRASTMODE               147
#define BASEPROPERTY_GRID_HEADER_BACKGROUND         148
#define BASEPROPERTY_GRID_HEADER_TEXT_COLOR         149
#define BASEPROPERTY_GRID_ROW_BACKGROUND_COLORS     150
#define BASEPROPERTY_GRID_LINE_COLOR                151
#define BASEPROPERTY_MULTISELECTION_SIMPLEMODE      152
#define BASEPROPERTY_ITEM_SEPARATOR_POS             153
#define BASEPROPERTY_GROUPNAME                      154  // OUString
#define BASEPROPERTY_MULTIPAGEVALUE                 155  // sal_Int32
#define BASEPROPERTY_USERFORMCONTAINEES             156  // css::container::XNameContainer
#define BASEPROPERTY_AUTO_REPEAT                    157
#define BASEPROPERTY_ROW_HEADER_WIDTH               158
#define BASEPROPERTY_COLUMN_HEADER_HEIGHT           159
#define BASEPROPERTY_USE_GRID_LINES                 160
#define BASEPROPERTY_SCROLLWIDTH                    161
#define BASEPROPERTY_SCROLLHEIGHT                   162
#define BASEPROPERTY_SCROLLTOP                      163
#define BASEPROPERTY_SCROLLLEFT                     164
#define BASEPROPERTY_ACTIVE_SEL_BACKGROUND_COLOR    165
#define BASEPROPERTY_INACTIVE_SEL_BACKGROUND_COLOR  166
#define BASEPROPERTY_ACTIVE_SEL_TEXT_COLOR          167
#define BASEPROPERTY_INACTIVE_SEL_TEXT_COLOR        168


// These properties are not bound, they are always extracted from the BASEPROPERTY_FONTDESCRIPTOR property
#define BASEPROPERTY_FONTDESCRIPTORPART_START           1000
#define BASEPROPERTY_FONTDESCRIPTORPART_NAME            1000    // OUString, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_STYLENAME       1001    // OUString, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_FAMILY          1002    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_CHARSET         1003    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_HEIGHT          1004    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_WEIGHT          1005    // Float, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_SLANT           1006    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_UNDERLINE       1007    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_STRIKEOUT       1008    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_WIDTH           1009    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_PITCH           1010    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_CHARWIDTH       1011    // Float, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_ORIENTATION     1012    // Float, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_KERNING         1013    // sal_Bool, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_WORDLINEMODE    1014    // sal_Bool, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_TYPE            1015    // sal_Int16, not bound
#define BASEPROPERTY_FONTDESCRIPTORPART_END             1015

#define PROPERTY_ALIGN_LEFT                     0
#define PROPERTY_ALIGN_CENTER                   1
#define PROPERTY_ALIGN_RIGHT                    2


TOOLKIT_DLLPUBLIC sal_uInt16        GetPropertyId( const OUString& rPropertyName );
const css::uno::Type*  GetPropertyType( sal_uInt16 nPropertyId );
TOOLKIT_DLLPUBLIC const OUString&    GetPropertyName( sal_uInt16 nPropertyId );
sal_Int16                           GetPropertyAttribs( sal_uInt16 nPropertyId );
sal_uInt16                          GetPropertyOrderNr( sal_uInt16 nPropertyId );
bool                            DoesDependOnOthers( sal_uInt16 nPropertyId );
bool                            CompareProperties( const css::uno::Any& r1, const css::uno::Any& r2 );


#endif // INCLUDED_TOOLKIT_HELPER_PROPERTY_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
