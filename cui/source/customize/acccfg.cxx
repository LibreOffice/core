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

// include own files

#include <acccfg.hxx>
#include <cfgutil.hxx>
#include <dialmgr.hxx>

#include <sfx2/filedlghelper.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/sfxresid.hxx>

#include <sal/macros.h>
#include <vcl/event.hxx>

#include <strings.hrc>
#include <sfx2/strings.hrc>
#include <svx/svxids.hrc>

// include interface declarations
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/frame/theUICommandDescription.hpp>
#include <com/sun/star/ui/GlobalAcceleratorConfiguration.hpp>
#include <com/sun/star/ui/theModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/UIConfigurationManager.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

// include search util
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>

// include other projects
#include <comphelper/processfactory.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <vcl/svapp.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <config_features.h>

#include <com/sun/star/frame/LayoutManager.hpp>

// namespaces

using namespace css;

constexpr OUString FOLDERNAME_UICONFIG = u"Configurations2"_ustr;

constexpr OUString MEDIATYPE_PROPNAME = u"MediaType"_ustr;

const sal_uInt16 KEYCODE_ARRAY[] = { KEY_F1,
                                     KEY_F2,
                                     KEY_F3,
                                     KEY_F4,
                                     KEY_F5,
                                     KEY_F6,
                                     KEY_F7,
                                     KEY_F8,
                                     KEY_F9,
                                     KEY_F10,
                                     KEY_F11,
                                     KEY_F12,
                                     KEY_F13,
                                     KEY_F14,
                                     KEY_F15,
                                     KEY_F16,

                                     KEY_DOWN,
                                     KEY_UP,
                                     KEY_LEFT,
                                     KEY_RIGHT,
                                     KEY_HOME,
                                     KEY_END,
                                     KEY_PAGEUP,
                                     KEY_PAGEDOWN,
                                     KEY_RETURN,
                                     KEY_ESCAPE,
                                     KEY_BACKSPACE,
                                     KEY_INSERT,
                                     KEY_DELETE,

                                     KEY_OPEN,
                                     KEY_CUT,
                                     KEY_COPY,
                                     KEY_PASTE,
                                     KEY_UNDO,
                                     KEY_REPEAT,
                                     KEY_FIND,
                                     KEY_PROPERTIES,
                                     KEY_FRONT,
                                     KEY_CONTEXTMENU,
                                     KEY_MENU,
                                     KEY_HELP,

                                     KEY_SHIFT | KEY_F1,
                                     KEY_SHIFT | KEY_F2,
                                     KEY_SHIFT | KEY_F3,
                                     KEY_SHIFT | KEY_F4,
                                     KEY_SHIFT | KEY_F5,
                                     KEY_SHIFT | KEY_F6,
                                     KEY_SHIFT | KEY_F7,
                                     KEY_SHIFT | KEY_F8,
                                     KEY_SHIFT | KEY_F9,
                                     KEY_SHIFT | KEY_F10,
                                     KEY_SHIFT | KEY_F11,
                                     KEY_SHIFT | KEY_F12,
                                     KEY_SHIFT | KEY_F13,
                                     KEY_SHIFT | KEY_F14,
                                     KEY_SHIFT | KEY_F15,
                                     KEY_SHIFT | KEY_F16,

                                     KEY_SHIFT | KEY_DOWN,
                                     KEY_SHIFT | KEY_UP,
                                     KEY_SHIFT | KEY_LEFT,
                                     KEY_SHIFT | KEY_RIGHT,
                                     KEY_SHIFT | KEY_HOME,
                                     KEY_SHIFT | KEY_END,
                                     KEY_SHIFT | KEY_PAGEUP,
                                     KEY_SHIFT | KEY_PAGEDOWN,
                                     KEY_SHIFT | KEY_RETURN,
                                     KEY_SHIFT | KEY_SPACE,
                                     KEY_SHIFT | KEY_ESCAPE,
                                     KEY_SHIFT | KEY_BACKSPACE,
                                     KEY_SHIFT | KEY_INSERT,
                                     KEY_SHIFT | KEY_DELETE,
                                     KEY_SHIFT | KEY_EQUAL,

                                     KEY_MOD1 | KEY_0,
                                     KEY_MOD1 | KEY_1,
                                     KEY_MOD1 | KEY_2,
                                     KEY_MOD1 | KEY_3,
                                     KEY_MOD1 | KEY_4,
                                     KEY_MOD1 | KEY_5,
                                     KEY_MOD1 | KEY_6,
                                     KEY_MOD1 | KEY_7,
                                     KEY_MOD1 | KEY_8,
                                     KEY_MOD1 | KEY_9,
                                     KEY_MOD1 | KEY_A,
                                     KEY_MOD1 | KEY_B,
                                     KEY_MOD1 | KEY_C,
                                     KEY_MOD1 | KEY_D,
                                     KEY_MOD1 | KEY_E,
                                     KEY_MOD1 | KEY_F,
                                     KEY_MOD1 | KEY_G,
                                     KEY_MOD1 | KEY_H,
                                     KEY_MOD1 | KEY_I,
                                     KEY_MOD1 | KEY_J,
                                     KEY_MOD1 | KEY_K,
                                     KEY_MOD1 | KEY_L,
                                     KEY_MOD1 | KEY_M,
                                     KEY_MOD1 | KEY_N,
                                     KEY_MOD1 | KEY_O,
                                     KEY_MOD1 | KEY_P,
                                     KEY_MOD1 | KEY_Q,
                                     KEY_MOD1 | KEY_R,
                                     KEY_MOD1 | KEY_S,
                                     KEY_MOD1 | KEY_T,
                                     KEY_MOD1 | KEY_U,
                                     KEY_MOD1 | KEY_V,
                                     KEY_MOD1 | KEY_W,
                                     KEY_MOD1 | KEY_X,
                                     KEY_MOD1 | KEY_Y,
                                     KEY_MOD1 | KEY_Z,
                                     KEY_MOD1 | KEY_NUMBERSIGN,
                                     KEY_MOD1 | KEY_COLON,
                                     KEY_MOD1 | KEY_SEMICOLON,
                                     KEY_MOD1 | KEY_QUOTELEFT,
                                     KEY_MOD1 | KEY_QUOTERIGHT,
                                     KEY_MOD1 | KEY_BRACKETLEFT,
                                     KEY_MOD1 | KEY_BRACKETRIGHT,
                                     KEY_MOD1 | KEY_RIGHTCURLYBRACKET,
                                     KEY_MOD1 | KEY_POINT,
                                     KEY_MOD1 | KEY_COMMA,
                                     KEY_MOD1 | KEY_TILDE,
                                     KEY_MOD1 | KEY_TAB,

                                     KEY_MOD1 | KEY_F1,
                                     KEY_MOD1 | KEY_F2,
                                     KEY_MOD1 | KEY_F3,
                                     KEY_MOD1 | KEY_F4,
                                     KEY_MOD1 | KEY_F5,
                                     KEY_MOD1 | KEY_F6,
                                     KEY_MOD1 | KEY_F7,
                                     KEY_MOD1 | KEY_F8,
                                     KEY_MOD1 | KEY_F9,
                                     KEY_MOD1 | KEY_F10,
                                     KEY_MOD1 | KEY_F11,
                                     KEY_MOD1 | KEY_F12,
                                     KEY_MOD1 | KEY_F13,
                                     KEY_MOD1 | KEY_F14,
                                     KEY_MOD1 | KEY_F15,
                                     KEY_MOD1 | KEY_F16,

                                     KEY_MOD1 | KEY_DOWN,
                                     KEY_MOD1 | KEY_UP,
                                     KEY_MOD1 | KEY_LEFT,
                                     KEY_MOD1 | KEY_RIGHT,
                                     KEY_MOD1 | KEY_HOME,
                                     KEY_MOD1 | KEY_END,
                                     KEY_MOD1 | KEY_PAGEUP,
                                     KEY_MOD1 | KEY_PAGEDOWN,
                                     KEY_MOD1 | KEY_RETURN,
                                     KEY_MOD1 | KEY_SPACE,
                                     KEY_MOD1 | KEY_BACKSPACE,
                                     KEY_MOD1 | KEY_INSERT,
                                     KEY_MOD1 | KEY_DELETE,

                                     KEY_MOD1 | KEY_ADD,
                                     KEY_MOD1 | KEY_SUBTRACT,
                                     KEY_MOD1 | KEY_MULTIPLY,
                                     KEY_MOD1 | KEY_DIVIDE,
                                     KEY_MOD1 | KEY_EQUAL,

                                     KEY_SHIFT | KEY_MOD1 | KEY_0,
                                     KEY_SHIFT | KEY_MOD1 | KEY_1,
                                     KEY_SHIFT | KEY_MOD1 | KEY_2,
                                     KEY_SHIFT | KEY_MOD1 | KEY_3,
                                     KEY_SHIFT | KEY_MOD1 | KEY_4,
                                     KEY_SHIFT | KEY_MOD1 | KEY_5,
                                     KEY_SHIFT | KEY_MOD1 | KEY_6,
                                     KEY_SHIFT | KEY_MOD1 | KEY_7,
                                     KEY_SHIFT | KEY_MOD1 | KEY_8,
                                     KEY_SHIFT | KEY_MOD1 | KEY_9,
                                     KEY_SHIFT | KEY_MOD1 | KEY_A,
                                     KEY_SHIFT | KEY_MOD1 | KEY_B,
                                     KEY_SHIFT | KEY_MOD1 | KEY_C,
                                     KEY_SHIFT | KEY_MOD1 | KEY_D,
                                     KEY_SHIFT | KEY_MOD1 | KEY_E,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F,
                                     KEY_SHIFT | KEY_MOD1 | KEY_G,
                                     KEY_SHIFT | KEY_MOD1 | KEY_H,
                                     KEY_SHIFT | KEY_MOD1 | KEY_I,
                                     KEY_SHIFT | KEY_MOD1 | KEY_J,
                                     KEY_SHIFT | KEY_MOD1 | KEY_K,
                                     KEY_SHIFT | KEY_MOD1 | KEY_L,
                                     KEY_SHIFT | KEY_MOD1 | KEY_M,
                                     KEY_SHIFT | KEY_MOD1 | KEY_N,
                                     KEY_SHIFT | KEY_MOD1 | KEY_O,
                                     KEY_SHIFT | KEY_MOD1 | KEY_P,
                                     KEY_SHIFT | KEY_MOD1 | KEY_Q,
                                     KEY_SHIFT | KEY_MOD1 | KEY_R,
                                     KEY_SHIFT | KEY_MOD1 | KEY_S,
                                     KEY_SHIFT | KEY_MOD1 | KEY_T,
                                     KEY_SHIFT | KEY_MOD1 | KEY_U,
                                     KEY_SHIFT | KEY_MOD1 | KEY_V,
                                     KEY_SHIFT | KEY_MOD1 | KEY_W,
                                     KEY_SHIFT | KEY_MOD1 | KEY_X,
                                     KEY_SHIFT | KEY_MOD1 | KEY_Y,
                                     KEY_SHIFT | KEY_MOD1 | KEY_Z,
                                     KEY_SHIFT | KEY_MOD1 | KEY_NUMBERSIGN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_COLON,
                                     KEY_SHIFT | KEY_MOD1 | KEY_SEMICOLON,
                                     KEY_SHIFT | KEY_MOD1 | KEY_QUOTELEFT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_QUOTERIGHT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_BRACKETLEFT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_BRACKETRIGHT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_RIGHTCURLYBRACKET,
                                     KEY_SHIFT | KEY_MOD1 | KEY_POINT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_COMMA,
                                     KEY_SHIFT | KEY_MOD1 | KEY_TILDE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_TAB,

                                     KEY_SHIFT | KEY_MOD1 | KEY_F1,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F2,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F3,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F4,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F5,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F6,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F7,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F8,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F9,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F10,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F11,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F12,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F13,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F14,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F15,
                                     KEY_SHIFT | KEY_MOD1 | KEY_F16,

                                     KEY_SHIFT | KEY_MOD1 | KEY_DOWN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_UP,
                                     KEY_SHIFT | KEY_MOD1 | KEY_LEFT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_RIGHT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_HOME,
                                     KEY_SHIFT | KEY_MOD1 | KEY_END,
                                     KEY_SHIFT | KEY_MOD1 | KEY_PAGEUP,
                                     KEY_SHIFT | KEY_MOD1 | KEY_PAGEDOWN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_RETURN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_ESCAPE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_SPACE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_BACKSPACE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_INSERT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_DELETE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_EQUAL,

                                     KEY_MOD2 | KEY_0,
                                     KEY_MOD2 | KEY_1,
                                     KEY_MOD2 | KEY_2,
                                     KEY_MOD2 | KEY_3,
                                     KEY_MOD2 | KEY_4,
                                     KEY_MOD2 | KEY_5,
                                     KEY_MOD2 | KEY_6,
                                     KEY_MOD2 | KEY_7,
                                     KEY_MOD2 | KEY_8,
                                     KEY_MOD2 | KEY_9,
                                     KEY_MOD2 | KEY_A,
                                     KEY_MOD2 | KEY_B,
                                     KEY_MOD2 | KEY_C,
                                     KEY_MOD2 | KEY_D,
                                     KEY_MOD2 | KEY_E,
                                     KEY_MOD2 | KEY_F,
                                     KEY_MOD2 | KEY_G,
                                     KEY_MOD2 | KEY_H,
                                     KEY_MOD2 | KEY_I,
                                     KEY_MOD2 | KEY_J,
                                     KEY_MOD2 | KEY_K,
                                     KEY_MOD2 | KEY_L,
                                     KEY_MOD2 | KEY_M,
                                     KEY_MOD2 | KEY_N,
                                     KEY_MOD2 | KEY_O,
                                     KEY_MOD2 | KEY_P,
                                     KEY_MOD2 | KEY_Q,
                                     KEY_MOD2 | KEY_R,
                                     KEY_MOD2 | KEY_S,
                                     KEY_MOD2 | KEY_T,
                                     KEY_MOD2 | KEY_U,
                                     KEY_MOD2 | KEY_V,
                                     KEY_MOD2 | KEY_W,
                                     KEY_MOD2 | KEY_X,
                                     KEY_MOD2 | KEY_Y,
                                     KEY_MOD2 | KEY_Z,
                                     KEY_MOD2 | KEY_NUMBERSIGN,
                                     KEY_MOD2 | KEY_COLON,
                                     KEY_MOD2 | KEY_SEMICOLON,
                                     KEY_MOD2 | KEY_QUOTELEFT,
                                     KEY_MOD2 | KEY_QUOTERIGHT,
                                     KEY_MOD2 | KEY_BRACKETLEFT,
                                     KEY_MOD2 | KEY_BRACKETRIGHT,
                                     KEY_MOD2 | KEY_RIGHTCURLYBRACKET,
                                     KEY_MOD2 | KEY_POINT,
                                     KEY_MOD2 | KEY_COMMA,
                                     KEY_MOD2 | KEY_TILDE,

                                     KEY_MOD2 | KEY_F1,
                                     KEY_MOD2 | KEY_F2,
                                     KEY_MOD2 | KEY_F3,
                                     KEY_MOD2 | KEY_F4,
                                     KEY_MOD2 | KEY_F5,
                                     KEY_MOD2 | KEY_F6,
                                     KEY_MOD2 | KEY_F7,
                                     KEY_MOD2 | KEY_F8,
                                     KEY_MOD2 | KEY_F9,
                                     KEY_MOD2 | KEY_F10,
                                     KEY_MOD2 | KEY_F11,
                                     KEY_MOD2 | KEY_F12,
                                     KEY_MOD2 | KEY_F13,
                                     KEY_MOD2 | KEY_F14,
                                     KEY_MOD2 | KEY_F15,
                                     KEY_MOD2 | KEY_F16,

                                     KEY_MOD2 | KEY_DOWN,
                                     KEY_MOD2 | KEY_UP,
                                     KEY_MOD2 | KEY_LEFT,
                                     KEY_MOD2 | KEY_RIGHT,
                                     KEY_MOD2 | KEY_HOME,
                                     KEY_MOD2 | KEY_END,
                                     KEY_MOD2 | KEY_PAGEUP,
                                     KEY_MOD2 | KEY_PAGEDOWN,
                                     KEY_MOD2 | KEY_RETURN,
                                     KEY_MOD2 | KEY_SPACE,
                                     KEY_MOD2 | KEY_BACKSPACE,
                                     KEY_MOD2 | KEY_INSERT,
                                     KEY_MOD2 | KEY_DELETE,
                                     KEY_MOD2 | KEY_EQUAL,

                                     KEY_SHIFT | KEY_MOD2 | KEY_0,
                                     KEY_SHIFT | KEY_MOD2 | KEY_1,
                                     KEY_SHIFT | KEY_MOD2 | KEY_2,
                                     KEY_SHIFT | KEY_MOD2 | KEY_3,
                                     KEY_SHIFT | KEY_MOD2 | KEY_4,
                                     KEY_SHIFT | KEY_MOD2 | KEY_5,
                                     KEY_SHIFT | KEY_MOD2 | KEY_6,
                                     KEY_SHIFT | KEY_MOD2 | KEY_7,
                                     KEY_SHIFT | KEY_MOD2 | KEY_8,
                                     KEY_SHIFT | KEY_MOD2 | KEY_9,
                                     KEY_SHIFT | KEY_MOD2 | KEY_A,
                                     KEY_SHIFT | KEY_MOD2 | KEY_B,
                                     KEY_SHIFT | KEY_MOD2 | KEY_C,
                                     KEY_SHIFT | KEY_MOD2 | KEY_D,
                                     KEY_SHIFT | KEY_MOD2 | KEY_E,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F,
                                     KEY_SHIFT | KEY_MOD2 | KEY_G,
                                     KEY_SHIFT | KEY_MOD2 | KEY_H,
                                     KEY_SHIFT | KEY_MOD2 | KEY_I,
                                     KEY_SHIFT | KEY_MOD2 | KEY_J,
                                     KEY_SHIFT | KEY_MOD2 | KEY_K,
                                     KEY_SHIFT | KEY_MOD2 | KEY_L,
                                     KEY_SHIFT | KEY_MOD2 | KEY_M,
                                     KEY_SHIFT | KEY_MOD2 | KEY_N,
                                     KEY_SHIFT | KEY_MOD2 | KEY_O,
                                     KEY_SHIFT | KEY_MOD2 | KEY_P,
                                     KEY_SHIFT | KEY_MOD2 | KEY_Q,
                                     KEY_SHIFT | KEY_MOD2 | KEY_R,
                                     KEY_SHIFT | KEY_MOD2 | KEY_S,
                                     KEY_SHIFT | KEY_MOD2 | KEY_T,
                                     KEY_SHIFT | KEY_MOD2 | KEY_U,
                                     KEY_SHIFT | KEY_MOD2 | KEY_V,
                                     KEY_SHIFT | KEY_MOD2 | KEY_W,
                                     KEY_SHIFT | KEY_MOD2 | KEY_X,
                                     KEY_SHIFT | KEY_MOD2 | KEY_Y,
                                     KEY_SHIFT | KEY_MOD2 | KEY_Z,
                                     KEY_SHIFT | KEY_MOD2 | KEY_NUMBERSIGN,
                                     KEY_SHIFT | KEY_MOD2 | KEY_COLON,
                                     KEY_SHIFT | KEY_MOD2 | KEY_SEMICOLON,
                                     KEY_SHIFT | KEY_MOD2 | KEY_QUOTELEFT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_QUOTERIGHT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_BRACKETLEFT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_BRACKETRIGHT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_RIGHTCURLYBRACKET,
                                     KEY_SHIFT | KEY_MOD2 | KEY_POINT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_COMMA,
                                     KEY_SHIFT | KEY_MOD2 | KEY_TILDE,

                                     KEY_SHIFT | KEY_MOD2 | KEY_F1,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F2,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F3,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F4,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F5,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F6,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F7,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F8,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F9,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F10,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F11,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F12,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F13,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F14,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F15,
                                     KEY_SHIFT | KEY_MOD2 | KEY_F16,

                                     KEY_SHIFT | KEY_MOD2 | KEY_DOWN,
                                     KEY_SHIFT | KEY_MOD2 | KEY_UP,
                                     KEY_SHIFT | KEY_MOD2 | KEY_LEFT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_RIGHT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_HOME,
                                     KEY_SHIFT | KEY_MOD2 | KEY_END,
                                     KEY_SHIFT | KEY_MOD2 | KEY_PAGEUP,
                                     KEY_SHIFT | KEY_MOD2 | KEY_PAGEDOWN,
                                     KEY_SHIFT | KEY_MOD2 | KEY_RETURN,
                                     KEY_SHIFT | KEY_MOD2 | KEY_ESCAPE,
                                     KEY_SHIFT | KEY_MOD2 | KEY_SPACE,
                                     KEY_SHIFT | KEY_MOD2 | KEY_BACKSPACE,
                                     KEY_SHIFT | KEY_MOD2 | KEY_INSERT,
                                     KEY_SHIFT | KEY_MOD2 | KEY_DELETE,
                                     KEY_SHIFT | KEY_MOD2 | KEY_EQUAL,

                                     KEY_MOD1 | KEY_MOD2 | KEY_0,
                                     KEY_MOD1 | KEY_MOD2 | KEY_1,
                                     KEY_MOD1 | KEY_MOD2 | KEY_2,
                                     KEY_MOD1 | KEY_MOD2 | KEY_3,
                                     KEY_MOD1 | KEY_MOD2 | KEY_4,
                                     KEY_MOD1 | KEY_MOD2 | KEY_5,
                                     KEY_MOD1 | KEY_MOD2 | KEY_6,
                                     KEY_MOD1 | KEY_MOD2 | KEY_7,
                                     KEY_MOD1 | KEY_MOD2 | KEY_8,
                                     KEY_MOD1 | KEY_MOD2 | KEY_9,
                                     KEY_MOD1 | KEY_MOD2 | KEY_A,
                                     KEY_MOD1 | KEY_MOD2 | KEY_B,
                                     KEY_MOD1 | KEY_MOD2 | KEY_C,
                                     KEY_MOD1 | KEY_MOD2 | KEY_D,
                                     KEY_MOD1 | KEY_MOD2 | KEY_E,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F,
                                     KEY_MOD1 | KEY_MOD2 | KEY_G,
                                     KEY_MOD1 | KEY_MOD2 | KEY_H,
                                     KEY_MOD1 | KEY_MOD2 | KEY_I,
                                     KEY_MOD1 | KEY_MOD2 | KEY_J,
                                     KEY_MOD1 | KEY_MOD2 | KEY_K,
                                     KEY_MOD1 | KEY_MOD2 | KEY_L,
                                     KEY_MOD1 | KEY_MOD2 | KEY_M,
                                     KEY_MOD1 | KEY_MOD2 | KEY_N,
                                     KEY_MOD1 | KEY_MOD2 | KEY_O,
                                     KEY_MOD1 | KEY_MOD2 | KEY_P,
                                     KEY_MOD1 | KEY_MOD2 | KEY_Q,
                                     KEY_MOD1 | KEY_MOD2 | KEY_R,
                                     KEY_MOD1 | KEY_MOD2 | KEY_S,
                                     KEY_MOD1 | KEY_MOD2 | KEY_T,
                                     KEY_MOD1 | KEY_MOD2 | KEY_U,
                                     KEY_MOD1 | KEY_MOD2 | KEY_V,
                                     KEY_MOD1 | KEY_MOD2 | KEY_W,
                                     KEY_MOD1 | KEY_MOD2 | KEY_X,
                                     KEY_MOD1 | KEY_MOD2 | KEY_Y,
                                     KEY_MOD1 | KEY_MOD2 | KEY_Z,
                                     KEY_MOD1 | KEY_MOD2 | KEY_NUMBERSIGN,
                                     KEY_MOD1 | KEY_MOD2 | KEY_COLON,
                                     KEY_MOD1 | KEY_MOD2 | KEY_SEMICOLON,
                                     KEY_MOD1 | KEY_MOD2 | KEY_QUOTELEFT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_QUOTERIGHT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_BRACKETLEFT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_BRACKETRIGHT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_RIGHTCURLYBRACKET,
                                     KEY_MOD1 | KEY_MOD2 | KEY_POINT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_COMMA,
                                     KEY_MOD1 | KEY_MOD2 | KEY_TILDE,
                                     KEY_MOD1 | KEY_MOD2 | KEY_EQUAL,

                                     KEY_MOD1 | KEY_MOD2 | KEY_F1,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F2,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F3,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F4,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F5,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F6,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F7,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F8,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F9,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F10,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F11,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F12,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F13,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F14,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F15,
                                     KEY_MOD1 | KEY_MOD2 | KEY_F16,

                                     KEY_MOD1 | KEY_MOD2 | KEY_DOWN,
                                     KEY_MOD1 | KEY_MOD2 | KEY_UP,
                                     KEY_MOD1 | KEY_MOD2 | KEY_LEFT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_RIGHT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_HOME,
                                     KEY_MOD1 | KEY_MOD2 | KEY_END,
                                     KEY_MOD1 | KEY_MOD2 | KEY_PAGEUP,
                                     KEY_MOD1 | KEY_MOD2 | KEY_PAGEDOWN,
                                     KEY_MOD1 | KEY_MOD2 | KEY_RETURN,
                                     KEY_MOD1 | KEY_MOD2 | KEY_SPACE,
                                     KEY_MOD1 | KEY_MOD2 | KEY_BACKSPACE,
                                     KEY_MOD1 | KEY_MOD2 | KEY_INSERT,
                                     KEY_MOD1 | KEY_MOD2 | KEY_DELETE,

                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_0,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_1,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_2,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_3,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_4,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_5,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_6,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_7,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_8,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_9,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_A,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_B,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_C,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_D,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_E,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_G,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_H,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_I,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_J,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_K,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_L,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_M,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_N,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_O,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_P,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_Q,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_R,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_S,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_T,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_U,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_V,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_W,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_X,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_Y,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_Z,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_NUMBERSIGN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_COLON,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_SEMICOLON,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_QUOTELEFT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_QUOTERIGHT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_BRACKETLEFT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_BRACKETRIGHT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_RIGHTCURLYBRACKET,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_POINT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_COMMA,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_TILDE,

                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F1,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F2,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F3,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F4,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F5,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F6,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F7,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F8,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F9,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F10,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F11,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F12,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F13,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F14,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F15,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_F16,

                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_DOWN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_UP,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_LEFT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_RIGHT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_HOME,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_END,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_PAGEUP,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_PAGEDOWN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_RETURN,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_SPACE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_BACKSPACE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_INSERT,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_DELETE,
                                     KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_EQUAL

#ifdef __APPLE__
                                     ,
                                     KEY_MOD3 | KEY_0,
                                     KEY_MOD3 | KEY_1,
                                     KEY_MOD3 | KEY_2,
                                     KEY_MOD3 | KEY_3,
                                     KEY_MOD3 | KEY_4,
                                     KEY_MOD3 | KEY_5,
                                     KEY_MOD3 | KEY_6,
                                     KEY_MOD3 | KEY_7,
                                     KEY_MOD3 | KEY_8,
                                     KEY_MOD3 | KEY_9,
                                     KEY_MOD3 | KEY_A,
                                     KEY_MOD3 | KEY_B,
                                     KEY_MOD3 | KEY_C,
                                     KEY_MOD3 | KEY_D,
                                     KEY_MOD3 | KEY_E,
                                     KEY_MOD3 | KEY_F,
                                     KEY_MOD3 | KEY_G,
                                     KEY_MOD3 | KEY_H,
                                     KEY_MOD3 | KEY_I,
                                     KEY_MOD3 | KEY_J,
                                     KEY_MOD3 | KEY_K,
                                     KEY_MOD3 | KEY_L,
                                     KEY_MOD3 | KEY_M,
                                     KEY_MOD3 | KEY_N,
                                     KEY_MOD3 | KEY_O,
                                     KEY_MOD3 | KEY_P,
                                     KEY_MOD3 | KEY_Q,
                                     KEY_MOD3 | KEY_R,
                                     KEY_MOD3 | KEY_S,
                                     KEY_MOD3 | KEY_T,
                                     KEY_MOD3 | KEY_U,
                                     KEY_MOD3 | KEY_V,
                                     KEY_MOD3 | KEY_W,
                                     KEY_MOD3 | KEY_X,
                                     KEY_MOD3 | KEY_Y,
                                     KEY_MOD3 | KEY_Z,
                                     KEY_MOD2 | KEY_NUMBERSIGN,
                                     KEY_MOD3 | KEY_COLON,
                                     KEY_MOD3 | KEY_SEMICOLON,
                                     KEY_MOD3 | KEY_QUOTELEFT,
                                     KEY_MOD3 | KEY_QUOTERIGHT,
                                     KEY_MOD3 | KEY_BRACKETLEFT,
                                     KEY_MOD3 | KEY_BRACKETRIGHT,
                                     KEY_MOD3 | KEY_RIGHTCURLYBRACKET,
                                     KEY_MOD3 | KEY_POINT,
                                     KEY_MOD3 | KEY_COMMA,
                                     KEY_MOD3 | KEY_TILDE,
                                     KEY_MOD3 | KEY_TAB,

                                     KEY_MOD3 | KEY_F1,
                                     KEY_MOD3 | KEY_F2,
                                     KEY_MOD3 | KEY_F3,
                                     KEY_MOD3 | KEY_F4,
                                     KEY_MOD3 | KEY_F5,
                                     KEY_MOD3 | KEY_F6,
                                     KEY_MOD3 | KEY_F7,
                                     KEY_MOD3 | KEY_F8,
                                     KEY_MOD3 | KEY_F9,
                                     KEY_MOD3 | KEY_F10,
                                     KEY_MOD3 | KEY_F11,
                                     KEY_MOD3 | KEY_F12,
                                     KEY_MOD3 | KEY_F13,
                                     KEY_MOD3 | KEY_F14,
                                     KEY_MOD3 | KEY_F15,
                                     KEY_MOD3 | KEY_F16,

                                     KEY_MOD3 | KEY_DOWN,
                                     KEY_MOD3 | KEY_UP,
                                     KEY_MOD3 | KEY_LEFT,
                                     KEY_MOD3 | KEY_RIGHT,
                                     KEY_MOD3 | KEY_HOME,
                                     KEY_MOD3 | KEY_END,
                                     KEY_MOD3 | KEY_PAGEUP,
                                     KEY_MOD3 | KEY_PAGEDOWN,
                                     KEY_MOD3 | KEY_RETURN,
                                     KEY_MOD3 | KEY_SPACE,
                                     KEY_MOD3 | KEY_BACKSPACE,
                                     KEY_MOD3 | KEY_INSERT,
                                     KEY_MOD3 | KEY_DELETE,

                                     KEY_MOD3 | KEY_ADD,
                                     KEY_MOD3 | KEY_SUBTRACT,
                                     KEY_MOD3 | KEY_MULTIPLY,
                                     KEY_MOD3 | KEY_DIVIDE,
                                     KEY_MOD3 | KEY_EQUAL,

                                     KEY_SHIFT | KEY_MOD3 | KEY_0,
                                     KEY_SHIFT | KEY_MOD3 | KEY_1,
                                     KEY_SHIFT | KEY_MOD3 | KEY_2,
                                     KEY_SHIFT | KEY_MOD3 | KEY_3,
                                     KEY_SHIFT | KEY_MOD3 | KEY_4,
                                     KEY_SHIFT | KEY_MOD3 | KEY_5,
                                     KEY_SHIFT | KEY_MOD3 | KEY_6,
                                     KEY_SHIFT | KEY_MOD3 | KEY_7,
                                     KEY_SHIFT | KEY_MOD3 | KEY_8,
                                     KEY_SHIFT | KEY_MOD3 | KEY_9,
                                     KEY_SHIFT | KEY_MOD3 | KEY_A,
                                     KEY_SHIFT | KEY_MOD3 | KEY_B,
                                     KEY_SHIFT | KEY_MOD3 | KEY_C,
                                     KEY_SHIFT | KEY_MOD3 | KEY_D,
                                     KEY_SHIFT | KEY_MOD3 | KEY_E,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F,
                                     KEY_SHIFT | KEY_MOD3 | KEY_G,
                                     KEY_SHIFT | KEY_MOD3 | KEY_H,
                                     KEY_SHIFT | KEY_MOD3 | KEY_I,
                                     KEY_SHIFT | KEY_MOD3 | KEY_J,
                                     KEY_SHIFT | KEY_MOD3 | KEY_K,
                                     KEY_SHIFT | KEY_MOD3 | KEY_L,
                                     KEY_SHIFT | KEY_MOD3 | KEY_M,
                                     KEY_SHIFT | KEY_MOD3 | KEY_N,
                                     KEY_SHIFT | KEY_MOD3 | KEY_O,
                                     KEY_SHIFT | KEY_MOD3 | KEY_P,
                                     KEY_SHIFT | KEY_MOD3 | KEY_Q,
                                     KEY_SHIFT | KEY_MOD3 | KEY_R,
                                     KEY_SHIFT | KEY_MOD3 | KEY_S,
                                     KEY_SHIFT | KEY_MOD3 | KEY_T,
                                     KEY_SHIFT | KEY_MOD3 | KEY_U,
                                     KEY_SHIFT | KEY_MOD3 | KEY_V,
                                     KEY_SHIFT | KEY_MOD3 | KEY_W,
                                     KEY_SHIFT | KEY_MOD3 | KEY_X,
                                     KEY_SHIFT | KEY_MOD3 | KEY_Y,
                                     KEY_SHIFT | KEY_MOD3 | KEY_Z,
                                     KEY_SHIFT | KEY_MOD3 | KEY_NUMBERSIGN,
                                     KEY_SHIFT | KEY_MOD3 | KEY_COLON,
                                     KEY_SHIFT | KEY_MOD3 | KEY_SEMICOLON,
                                     KEY_SHIFT | KEY_MOD3 | KEY_QUOTELEFT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_QUOTERIGHT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_BRACKETLEFT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_BRACKETRIGHT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_RIGHTCURLYBRACKET,
                                     KEY_SHIFT | KEY_MOD3 | KEY_POINT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_COMMA,
                                     KEY_SHIFT | KEY_MOD3 | KEY_TILDE,
                                     KEY_SHIFT | KEY_MOD3 | KEY_TAB,

                                     KEY_SHIFT | KEY_MOD3 | KEY_F1,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F2,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F3,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F4,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F5,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F6,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F7,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F8,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F9,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F10,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F11,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F12,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F13,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F14,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F15,
                                     KEY_SHIFT | KEY_MOD3 | KEY_F16,

                                     KEY_SHIFT | KEY_MOD3 | KEY_DOWN,
                                     KEY_SHIFT | KEY_MOD3 | KEY_UP,
                                     KEY_SHIFT | KEY_MOD3 | KEY_LEFT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_RIGHT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_HOME,
                                     KEY_SHIFT | KEY_MOD3 | KEY_END,
                                     KEY_SHIFT | KEY_MOD3 | KEY_PAGEUP,
                                     KEY_SHIFT | KEY_MOD3 | KEY_PAGEDOWN,
                                     KEY_SHIFT | KEY_MOD3 | KEY_RETURN,
                                     KEY_SHIFT | KEY_MOD3 | KEY_ESCAPE,
                                     KEY_SHIFT | KEY_MOD3 | KEY_SPACE,
                                     KEY_SHIFT | KEY_MOD3 | KEY_BACKSPACE,
                                     KEY_SHIFT | KEY_MOD3 | KEY_INSERT,
                                     KEY_SHIFT | KEY_MOD3 | KEY_DELETE,
                                     KEY_SHIFT | KEY_MOD3 | KEY_EQUAL
#endif
};

const sal_uInt16 KEYCODE_ARRAY_SIZE = std::size(KEYCODE_ARRAY);

/** select the entry, which match the current key input ... excepting
    keys, which are used for the dialog itself.
  */
IMPL_LINK(SfxAcceleratorConfigPage, KeyInputHdl, const KeyEvent&, rKey, bool)
{
    vcl::KeyCode aCode1 = rKey.GetKeyCode();
    sal_uInt16 nCode1 = aCode1.GetCode();
    sal_uInt16 nMod1 = aCode1.GetModifier();

    // is it related to our list box ?
    if ((nCode1 == KEY_DOWN) || (nCode1 == KEY_UP) || (nCode1 == KEY_LEFT) || (nCode1 == KEY_RIGHT)
        || (nCode1 == KEY_PAGEUP) || (nCode1 == KEY_PAGEDOWN))
        // no - handle it as normal dialog input
        return false;

    for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
    {
        TAccInfo* pUserData = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(i));
        if (pUserData)
        {
            sal_uInt16 nCode2 = pUserData->m_aKey.GetCode();
            sal_uInt16 nMod2 = pUserData->m_aKey.GetModifier();

            if (nCode1 == nCode2 && nMod1 == nMod2)
            {
                m_xEntriesBox->select(i);
                m_xEntriesBox->scroll_to_row(i);
                return true;
            }
        }
    }

    // no - handle it as normal dialog input
    return false;
}

SfxAcceleratorConfigPage::SfxAcceleratorConfigPage(weld::Container* pPage,
                                                   weld::DialogController* pController,
                                                   const SfxItemSet& aSet)
    : SfxTabPage(pPage, pController, u"cui/ui/accelconfigpage.ui"_ustr, u"AccelConfigPage"_ustr,
                 &aSet)
#if HAVE_FEATURE_SCRIPTING
    , m_pMacroInfoItem()
#endif
    , aLoadAccelConfigStr(CuiResId(RID_CUISTR_LOADACCELCONFIG))
    , aSaveAccelConfigStr(CuiResId(RID_CUISTR_SAVEACCELCONFIG))
    , aFilterAllStr(SfxResId(STR_SFX_FILTERNAME_ALL))
    , aFilterCfgStr(CuiResId(RID_CUISTR_FILTERNAME_CFG))
    , m_bStylesInfoInitialized(false)
    , m_aUpdateDataTimer("SfxAcceleratorConfigPage UpdateDataTimer")
    , m_aFillGroupIdle("SfxAcceleratorConfigPage m_aFillGroupIdle")
    , m_xEntriesBox(m_xBuilder->weld_tree_view(u"shortcuts"_ustr))
    , m_xOfficeButton(m_xBuilder->weld_radio_button(u"office"_ustr))
    , m_xModuleButton(m_xBuilder->weld_radio_button(u"module"_ustr))
    , m_xChangeButton(m_xBuilder->weld_button(u"change"_ustr))
    , m_xRemoveButton(m_xBuilder->weld_button(u"delete"_ustr))
    , m_xGroupLBox(new CuiConfigGroupListBox(m_xBuilder->weld_tree_view(u"category"_ustr)))
    , m_xFunctionBox(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view(u"function"_ustr)))
    , m_xKeyBox(m_xBuilder->weld_tree_view(u"keys"_ustr))
    , m_xSearchEdit(m_xBuilder->weld_entry(u"searchEntry"_ustr))
    , m_xLoadButton(m_xBuilder->weld_button(u"load"_ustr))
    , m_xSaveButton(m_xBuilder->weld_button(u"save"_ustr))
    , m_xResetButton(m_xBuilder->weld_button(u"reset"_ustr))
{
    Size aSize(m_xEntriesBox->get_approximate_digit_width() * 40,
               m_xEntriesBox->get_height_rows(10));
    m_xEntriesBox->set_size_request(aSize.Width(), aSize.Height());
    aSize = Size(m_xEntriesBox->get_approximate_digit_width() * 19,
                 m_xEntriesBox->get_height_rows(9));
    m_xGroupLBox->set_size_request(aSize.Width(), aSize.Height());
    aSize = Size(m_xEntriesBox->get_approximate_digit_width() * 21,
                 m_xEntriesBox->get_height_rows(9));
    m_xFunctionBox->set_size_request(aSize.Width(), aSize.Height());
    aSize = Size(m_xEntriesBox->get_approximate_digit_width() * 20,
                 m_xEntriesBox->get_height_rows(9));
    m_xKeyBox->set_size_request(aSize.Width(), aSize.Height());

    // install handler functions
    m_xChangeButton->connect_clicked(LINK(this, SfxAcceleratorConfigPage, ChangeHdl));
    m_xRemoveButton->connect_clicked(LINK(this, SfxAcceleratorConfigPage, RemoveHdl));
    m_xEntriesBox->connect_changed(LINK(this, SfxAcceleratorConfigPage, SelectHdl));
    m_xEntriesBox->connect_key_press(LINK(this, SfxAcceleratorConfigPage, KeyInputHdl));
    m_xGroupLBox->connect_changed(LINK(this, SfxAcceleratorConfigPage, SelectHdl));
    m_xFunctionBox->connect_changed(LINK(this, SfxAcceleratorConfigPage, SelectHdl));
    m_xKeyBox->connect_changed(LINK(this, SfxAcceleratorConfigPage, SelectHdl));
    m_xLoadButton->connect_clicked(LINK(this, SfxAcceleratorConfigPage, Load));
    m_xSaveButton->connect_clicked(LINK(this, SfxAcceleratorConfigPage, Save));
    m_xResetButton->connect_clicked(LINK(this, SfxAcceleratorConfigPage, Default));
    m_xOfficeButton->connect_toggled(LINK(this, SfxAcceleratorConfigPage, RadioHdl));
    m_xSearchEdit->connect_changed(LINK(this, SfxAcceleratorConfigPage, SearchUpdateHdl));
    m_xSearchEdit->connect_focus_out(LINK(this, SfxAcceleratorConfigPage, FocusOut_Impl));

    // detect max keyname width
    int nMaxWidth = 0;
    for (unsigned short i : KEYCODE_ARRAY)
    {
        int nTmp = m_xEntriesBox->get_pixel_size(vcl::KeyCode(i).GetName()).Width();
        if (nTmp > nMaxWidth)
            nMaxWidth = nTmp;
    }
    // recalc second tab
    auto nNewTab = nMaxWidth + 5; // additional space

    // initialize Entriesbox
    std::vector<int> aWidths{ nNewTab };
    m_xEntriesBox->set_column_fixed_widths(aWidths);

    //Initialize search util
    m_options.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    m_options.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_options.searchFlag
        |= (util::SearchFlags::REG_NOT_BEGINOFLINE | util::SearchFlags::REG_NOT_ENDOFLINE);
    // initialize GroupBox
    m_xGroupLBox->SetFunctionListBox(m_xFunctionBox.get());

    // initialize KeyBox
    m_xKeyBox->make_sorted();

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SfxAcceleratorConfigPage, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_aFillGroupIdle.SetInvokeHandler(LINK(this, SfxAcceleratorConfigPage, TimeOut_Impl));
    m_aFillGroupIdle.SetPriority(TaskPriority::HIGHEST);
}

SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
    m_aFillGroupIdle.Stop();

    // free memory - remove all dynamic user data
    for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
    {
        TAccInfo* pUserData = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(i));
        delete pUserData;
    }
}

void SfxAcceleratorConfigPage::InitAccCfg()
{
    // already initialized ?
    if (m_xContext.is())
        return; // yes -> do nothing

    try
    {
        // no - initialize this instance
        m_xContext = comphelper::getProcessComponentContext();

        m_xUICmdDescription = frame::theUICommandDescription::get(m_xContext);

        // get the current active frame, which should be our "parent"
        // for this session
        m_xFrame = GetFrame();
        if (!m_xFrame.is())
        {
            uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create(m_xContext);
            m_xFrame = xDesktop->getActiveFrame();
        }

        // identify module
        uno::Reference<frame::XModuleManager2> xModuleManager
            = frame::ModuleManager::create(m_xContext);
        m_sModuleLongName = xModuleManager->identify(m_xFrame);
        comphelper::SequenceAsHashMap lModuleProps(xModuleManager->getByName(m_sModuleLongName));
        m_sModuleUIName
            = lModuleProps.getUnpackedValueOrDefault(u"ooSetupFactoryUIName"_ustr, OUString());

        // get global accelerator configuration
        m_xGlobal = css::ui::GlobalAcceleratorConfiguration::create(m_xContext);

        // get module accelerator configuration

        uno::Reference<ui::XModuleUIConfigurationManagerSupplier> xModuleCfgSupplier(
            ui::theModuleUIConfigurationManagerSupplier::get(m_xContext));
        uno::Reference<ui::XUIConfigurationManager> xUICfgManager
            = xModuleCfgSupplier->getUIConfigurationManager(m_sModuleLongName);
        m_xModule = xUICfgManager->getShortCutManager();
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        m_xContext.clear();
    }
}

void SfxAcceleratorConfigPage::Init(const uno::Reference<ui::XAcceleratorConfiguration>& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    if (!m_bStylesInfoInitialized)
    {
        uno::Reference<frame::XController> xController;
        uno::Reference<frame::XModel> xModel;
        if (m_xFrame.is())
            xController = m_xFrame->getController();
        if (xController.is())
            xModel = xController->getModel();

        m_aStylesInfo.init(m_sModuleLongName, xModel);
        m_xGroupLBox->SetStylesInfo(&m_aStylesInfo);
        m_bStylesInfoInitialized = true;
    }

    // Insert all editable accelerators into list box. It is possible
    // that some accelerators are not mapped on the current system/keyboard
    // but we don't want to lose these mappings.
    for (sal_Int32 i1 = 0; i1 < KEYCODE_ARRAY_SIZE; ++i1)
    {
        vcl::KeyCode aKey = KEYCODE_ARRAY[i1];
        OUString sKey = aKey.GetName();
        if (sKey.isEmpty())
            continue;
        TAccInfo* pEntry = new TAccInfo(i1, 0 /*nListPos*/, aKey);
        m_xEntriesBox->append(weld::toId(pEntry), sKey);
        int nPos = m_xEntriesBox->n_children() - 1;
        m_xEntriesBox->set_text(nPos, OUString(), 1);
        m_xEntriesBox->set_sensitive(nPos, true);
    }

    // Assign all commands to its shortcuts - reading the accelerator config.
    uno::Sequence<awt::KeyEvent> lKeys = xAccMgr->getAllKeyEvents();
    sal_Int32 c2 = lKeys.getLength();
    sal_Int32 i2 = 0;

    for (i2 = 0; i2 < c2; ++i2)
    {
        const awt::KeyEvent& aAWTKey = lKeys[i2];
        OUString sCommand = xAccMgr->getCommandByKeyEvent(aAWTKey);
        OUString sLabel = GetLabel4Command(sCommand);
        vcl::KeyCode aKeyCode = svt::AcceleratorExecute::st_AWTKey2VCLKey(aAWTKey);
        sal_Int32 nPos = MapKeyCodeToPos(aKeyCode);

        if (nPos == -1)
            continue;

        m_xEntriesBox->set_text(nPos, sLabel, 1);

        TAccInfo* pEntry = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(nPos));
        pEntry->m_bIsConfigurable = true;

        pEntry->m_sCommand = sCommand;
    }

    // Map the VCL hardcoded key codes and mark them as not changeable
    size_t c3 = Application::GetReservedKeyCodeCount();
    size_t i3 = 0;
    for (i3 = 0; i3 < c3; ++i3)
    {
        const vcl::KeyCode* pKeyCode = Application::GetReservedKeyCode(i3);
        sal_Int32 nPos = MapKeyCodeToPos(*pKeyCode);

        if (nPos == -1)
            continue;

        // Hardcoded function mapped so no ID possible and mark entry as not changeable
        TAccInfo* pEntry = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(nPos));
        pEntry->m_bIsConfigurable = false;

        m_xEntriesBox->set_sensitive(nPos, false);
    }
}

void SfxAcceleratorConfigPage::Apply(const uno::Reference<ui::XAcceleratorConfiguration>& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    // Go through the list from the bottom to the top ...
    // because logical accelerator must be preferred instead of
    // physical ones!
    for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
    {
        TAccInfo* pUserData = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(i));
        OUString sCommand;
        awt::KeyEvent aAWTKey;

        if (pUserData)
        {
            sCommand = pUserData->m_sCommand;
            aAWTKey = svt::AcceleratorExecute::st_VCLKey2AWTKey(pUserData->m_aKey);
        }

        try
        {
            if (!sCommand.isEmpty())
                xAccMgr->setKeyEvent(aAWTKey, sCommand);
            else
                xAccMgr->removeKeyEvent(aAWTKey);
        }
        catch (const uno::RuntimeException&)
        {
            throw;
        }
        catch (const uno::Exception&)
        {
        }
    }
}

void SfxAcceleratorConfigPage::ResetConfig() { m_xEntriesBox->clear(); }

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, ImplUpdateDataHdl, Timer*, void)
{
    SelectHdl(m_xGroupLBox->get_widget());
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, SearchUpdateHdl, weld::Entry&, void)
{
    m_aUpdateDataTimer.Start();
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, FocusOut_Impl, weld::Widget&, void)
{
    if (m_aUpdateDataTimer.IsActive())
    {
        m_aUpdateDataTimer.Stop();
        m_aUpdateDataTimer.Invoke();
    }
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Load, weld::Button&, void)
{
    // ask for filename, where we should load the new config data from
    StartFileDialog(StartFileDialogType::Open, aLoadAccelConfigStr);
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Save, weld::Button&, void)
{
    StartFileDialog(StartFileDialogType::SaveAs, aSaveAccelConfigStr);
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Default, weld::Button&, void)
{
    uno::Reference<form::XReset> xReset(m_xAct, uno::UNO_QUERY);
    if (xReset.is())
        xReset->reset();

    m_xEntriesBox->freeze();
    ResetConfig();
    Init(m_xAct);
    m_xEntriesBox->thaw();
    m_xEntriesBox->select(0);
    SelectHdl(*m_xEntriesBox);
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, ChangeHdl, weld::Button&, void)
{
    int nPos = m_xEntriesBox->get_selected_index();
    if (nPos == -1)
        return;

    TAccInfo* pEntry = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(nPos));
    OUString sNewCommand = m_xFunctionBox->GetCurCommand();
    OUString sLabel = m_xFunctionBox->GetCurLabel();
    if (sLabel.isEmpty())
        sLabel = GetLabel4Command(sNewCommand);

    pEntry->m_sCommand = sNewCommand;
    m_xEntriesBox->set_text(nPos, sLabel, 1);

    SelectHdl(m_xFunctionBox->get_widget());
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, RemoveHdl, weld::Button&, void)
{
    // get selected entry
    int nPos = m_xEntriesBox->get_selected_index();
    if (nPos == -1)
        return;

    TAccInfo* pEntry = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(nPos));

    // remove function name from selected entry
    m_xEntriesBox->set_text(nPos, OUString(), 1);
    pEntry->m_sCommand.clear();

    SelectHdl(m_xFunctionBox->get_widget());
}

IMPL_LINK(SfxAcceleratorConfigPage, SelectHdl, weld::TreeView&, rListBox, void)
{
    if (&rListBox == m_xEntriesBox.get())
    {
        TAccInfo* pEntry = weld::fromId<TAccInfo*>(m_xEntriesBox->get_selected_id());

        OUString sPossibleNewCommand = m_xFunctionBox->GetCurCommand();

        m_xRemoveButton->set_sensitive(false);
        m_xChangeButton->set_sensitive(false);

        if (pEntry && pEntry->m_bIsConfigurable)
        {
            if (pEntry->isConfigured())
                m_xRemoveButton->set_sensitive(true);
            m_xChangeButton->set_sensitive(pEntry->m_sCommand != sPossibleNewCommand);
        }
    }
    else if (&rListBox == &m_xGroupLBox->get_widget())
    {
        m_xGroupLBox->GroupSelected();

        // Pause redraw (Do not redraw at each removal)
        m_xFunctionBox->freeze();
        // Apply the search filter to the functions list
        OUString aSearchTerm(m_xSearchEdit->get_text());
        int nMatchFound = applySearchFilter(aSearchTerm);
        // Resume redraw
        m_xFunctionBox->thaw();
        if (nMatchFound != -1)
        {
            m_xFunctionBox->select(nMatchFound);
            SelectHdl(m_xFunctionBox->get_widget());
        }
        else
        {
            m_xKeyBox->clear();
            m_xChangeButton->set_sensitive(false);
        }
    }
    else if (&rListBox == &m_xFunctionBox->get_widget())
    {
        m_xRemoveButton->set_sensitive(false);
        m_xChangeButton->set_sensitive(false);

        // #i36994 First selected can return null!
        TAccInfo* pEntry = weld::fromId<TAccInfo*>(m_xEntriesBox->get_selected_id());
        if (pEntry)
        {
            OUString sPossibleNewCommand = m_xFunctionBox->GetCurCommand();

            if (pEntry->m_bIsConfigurable)
            {
                if (pEntry->isConfigured())
                    m_xRemoveButton->set_sensitive(true);
                m_xChangeButton->set_sensitive(pEntry->m_sCommand != sPossibleNewCommand
                                               && !sPossibleNewCommand.isEmpty());
            }

            // update key box
            m_xKeyBox->clear();
            if (!sPossibleNewCommand.isEmpty())
            {
                for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
                {
                    TAccInfo* pUserData = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(i));
                    if (pUserData && pUserData->m_sCommand == sPossibleNewCommand)
                    {
                        m_xKeyBox->append(weld::toId(pUserData), pUserData->m_aKey.GetName());
                    }
                }
            }
        }
    }
    else
    {
        // goto selected "key" entry of the key box
        int nP2 = -1;
        TAccInfo* pU2 = weld::fromId<TAccInfo*>(m_xKeyBox->get_selected_id());
        if (pU2)
            nP2 = MapKeyCodeToPos(pU2->m_aKey);
        if (nP2 != -1)
        {
            m_xEntriesBox->select(nP2);
            m_xEntriesBox->scroll_to_row(nP2);
            SelectHdl(*m_xEntriesBox);
        }
    }
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, RadioHdl, weld::Toggleable&, void)
{
    uno::Reference<ui::XAcceleratorConfiguration> xOld = m_xAct;

    if (m_xOfficeButton->get_active())
        m_xAct = m_xGlobal;
    else if (m_xModuleButton->get_active())
        m_xAct = m_xModule;

    // nothing changed? => do nothing!
    if (m_xAct.is() && (xOld == m_xAct))
        return;

    m_xEntriesBox->freeze();
    ResetConfig();
    Init(m_xAct);
    m_xEntriesBox->thaw();

    m_xGroupLBox->Init(m_xContext, m_xFrame, m_sModuleLongName, true);

    // pb: #133213# do not select NULL entries
    if (m_xEntriesBox->n_children())
        m_xEntriesBox->select(0);

    m_aFillGroupIdle.Start();
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, TimeOut_Impl, Timer*, void)
{
    // activating the selection, typically "all commands", can take a long time
    // -> show wait cursor and disable input
    weld::WaitObject aWaitObject(GetFrameWeld());

    weld::TreeView& rTreeView = m_xGroupLBox->get_widget();
    SelectHdl(rTreeView);
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, LoadHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sCfgName;
    if (ERRCODE_NONE == m_pFileDlg->GetError())
        sCfgName = m_pFileDlg->GetPath();

    if (sCfgName.isEmpty())
        return;

    weld::WaitObject aWaitObject(GetFrameWeld());

    uno::Reference<ui::XUIConfigurationManager> xCfgMgr;
    uno::Reference<embed::XStorage>
        xRootStorage; // we must hold the root storage alive, if xCfgMgr is used!

    try
    {
        // don't forget to release the storage afterwards!
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory(
            embed::StorageFactory::create(m_xContext));
        uno::Sequence<uno::Any> lArgs{ uno::Any(sCfgName),
                                       uno::Any(css::embed::ElementModes::READ) };

        xRootStorage.set(xStorageFactory->createInstanceWithArguments(lArgs), uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> xUIConfig
            = xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, embed::ElementModes::READ);
        if (xUIConfig.is())
        {
            uno::Reference<ui::XUIConfigurationManager2> xCfgMgr2
                = ui::UIConfigurationManager::create(m_xContext);
            xCfgMgr2->setStorage(xUIConfig);
            xCfgMgr.set(xCfgMgr2, uno::UNO_QUERY_THROW);
        }

        if (xCfgMgr.is())
        {
            // open the configuration and update our UI
            uno::Reference<ui::XAcceleratorConfiguration> xTempAccMgr(xCfgMgr->getShortCutManager(),
                                                                      uno::UNO_SET_THROW);

            m_xEntriesBox->freeze();
            ResetConfig();
            Init(xTempAccMgr);
            m_xEntriesBox->thaw();
            if (m_xEntriesBox->n_children())
            {
                m_xEntriesBox->select(0);
                SelectHdl(m_xFunctionBox->get_widget());
            }
        }

        // don't forget to close the new opened storage!
        // We are the owner of it.
        if (xRootStorage.is())
        {
            uno::Reference<lang::XComponent> xComponent;
            xComponent.set(xCfgMgr, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            xRootStorage->dispose();
        }
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
    }
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, SaveHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sCfgName;
    if (ERRCODE_NONE == m_pFileDlg->GetError())
        sCfgName = m_pFileDlg->GetPath();

    if (sCfgName.isEmpty())
        return;

    weld::WaitObject aWaitObject(GetFrameWeld());

    uno::Reference<embed::XStorage> xRootStorage;

    try
    {
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory(
            embed::StorageFactory::create(m_xContext));
        uno::Sequence<uno::Any> lArgs{ uno::Any(sCfgName), uno::Any(embed::ElementModes::WRITE) };

        xRootStorage.set(xStorageFactory->createInstanceWithArguments(lArgs), uno::UNO_QUERY_THROW);

        uno::Reference<embed::XStorage> xUIConfig(
            xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, embed::ElementModes::WRITE),
            uno::UNO_SET_THROW);
        uno::Reference<beans::XPropertySet> xUIConfigProps(xUIConfig, uno::UNO_QUERY_THROW);

        // set the correct media type if the storage was new created
        OUString sMediaType;
        xUIConfigProps->getPropertyValue(MEDIATYPE_PROPNAME) >>= sMediaType;
        if (sMediaType.isEmpty())
            xUIConfigProps->setPropertyValue(
                MEDIATYPE_PROPNAME, uno::Any(u"application/vnd.sun.xml.ui.configuration"_ustr));

        uno::Reference<ui::XUIConfigurationManager2> xCfgMgr
            = ui::UIConfigurationManager::create(m_xContext);
        xCfgMgr->setStorage(xUIConfig);

        // get the target configuration access and update with all shortcuts
        // which are set currently at the UI!
        // Don't copy the m_xAct content to it... because m_xAct will be updated
        // from the UI on pressing the button "OK" only. And inbetween it's not up to date!
        uno::Reference<ui::XAcceleratorConfiguration> xTargetAccMgr(xCfgMgr->getShortCutManager(),
                                                                    uno::UNO_SET_THROW);
        Apply(xTargetAccMgr);

        // commit (order is important!)
        uno::Reference<ui::XUIConfigurationPersistence> xCommit1(xTargetAccMgr,
                                                                 uno::UNO_QUERY_THROW);
        uno::Reference<ui::XUIConfigurationPersistence> xCommit2(xCfgMgr, uno::UNO_QUERY_THROW);
        xCommit1->store();
        xCommit2->store();

        if (xRootStorage.is())
        {
            // Commit root storage
            uno::Reference<embed::XTransactedObject> xCommit3(xRootStorage, uno::UNO_QUERY_THROW);
            xCommit3->commit();
        }

        if (xRootStorage.is())
        {
            if (xCfgMgr.is())
                xCfgMgr->dispose();
            xRootStorage->dispose();
        }
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
    }
}

void SfxAcceleratorConfigPage::StartFileDialog(StartFileDialogType nType, const OUString& rTitle)
{
    bool bSave = nType == StartFileDialogType::SaveAs;
    short nDialogType = bSave ? ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION
                              : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
    m_pFileDlg.reset(
        new sfx2::FileDialogHelper(nDialogType, FileDialogFlags::NONE, GetFrameWeld()));

    m_pFileDlg->SetTitle(rTitle);
    m_pFileDlg->AddFilter(aFilterAllStr, FILEDIALOG_FILTER_ALL);
    m_pFileDlg->AddFilter(aFilterCfgStr, u"*.cfg"_ustr);
    m_pFileDlg->SetCurrentFilter(aFilterCfgStr);
    m_pFileDlg->SetContext(sfx2::FileDialogHelper::AcceleratorConfig);

    Link<sfx2::FileDialogHelper*, void> aDlgClosedLink
        = bSave ? LINK(this, SfxAcceleratorConfigPage, SaveHdl)
                : LINK(this, SfxAcceleratorConfigPage, LoadHdl);
    m_pFileDlg->StartExecuteModal(aDlgClosedLink);
}

bool SfxAcceleratorConfigPage::FillItemSet(SfxItemSet*)
{
    Apply(m_xAct);
    try
    {
        m_xAct->store();
        css::uno::Reference<css::beans::XPropertySet> xFrameProps(m_xFrame,
                                                                  css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::frame::XLayoutManager> xLayoutManager;
        xFrameProps->getPropertyValue(u"LayoutManager"_ustr) >>= xLayoutManager;
        css::uno::Reference<css::beans::XPropertySet> xLayoutProps(xLayoutManager,
                                                                   css::uno::UNO_QUERY_THROW);
        xLayoutProps->setPropertyValue(u"RefreshContextToolbarToolTip"_ustr, css::uno::Any(true));
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
        return false;
    }

    return true;
}

void SfxAcceleratorConfigPage::Reset(const SfxItemSet* rSet)
{
    // open accelerator configs
    // Note: It initialize some other members too, which are needed here ...
    // e.g. m_sModuleUIName!
    InitAccCfg();

    // change the description of the radio button, which switch to the module
    // dependent accelerator configuration
    OUString sButtonText = m_xModuleButton->get_label();
    sButtonText
        = m_xModuleButton->strip_mnemonic(sButtonText).replaceFirst("$(MODULE)", m_sModuleUIName);
    m_xModuleButton->set_label(sButtonText);

    if (m_xModule.is())
        m_xModuleButton->set_active(true);
    else
    {
        m_xModuleButton->hide();
        m_xOfficeButton->set_active(true);
    }

    RadioHdl(*m_xOfficeButton);

#if HAVE_FEATURE_SCRIPTING
    if (const SfxMacroInfoItem* pMacroItem = rSet->GetItemIfSet(SID_MACROINFO))
    {
        m_pMacroInfoItem = pMacroItem;
        m_xGroupLBox->SelectMacro(m_pMacroInfoItem);
    }
#else
    (void)rSet;
#endif
}

sal_Int32 SfxAcceleratorConfigPage::MapKeyCodeToPos(const vcl::KeyCode& aKey) const
{
    sal_uInt16 nCode1 = aKey.GetCode() + aKey.GetModifier();
    for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
    {
        TAccInfo* pUserData = weld::fromId<TAccInfo*>(m_xEntriesBox->get_id(i));
        if (pUserData)
        {
            sal_uInt16 nCode2 = pUserData->m_aKey.GetCode() + pUserData->m_aKey.GetModifier();
            if (nCode1 == nCode2)
                return i;
        }
    }

    return -1;
}

OUString SfxAcceleratorConfigPage::GetLabel4Command(const OUString& sCommand)
{
    try
    {
        // check global command configuration first
        uno::Reference<container::XNameAccess> xModuleConf;
        m_xUICmdDescription->getByName(m_sModuleLongName) >>= xModuleConf;
        if (xModuleConf.is())
        {
            ::comphelper::SequenceAsHashMap lProps(xModuleConf->getByName(sCommand));
            OUString sLabel = lProps.getUnpackedValueOrDefault(u"Name"_ustr, OUString());
            if (!sLabel.isEmpty())
                return sLabel;
        }
    }
    catch (const uno::RuntimeException&)
    {
        throw;
    }
    catch (const uno::Exception&)
    {
    }

    // may be it's a style URL .. they must be handled special
    SfxStyleInfo_Impl aStyle;
    aStyle.sCommand = sCommand;
    if (SfxStylesInfo_Impl::parseStyleCommand(aStyle))
    {
        m_aStylesInfo.getLabel4Style(aStyle);
        return aStyle.sLabel;
    }

    return sCommand;
}

/*
 * Remove entries which doesn't contain the search term
 */
int SfxAcceleratorConfigPage::applySearchFilter(OUString const& rSearchTerm)
{
    if (rSearchTerm.isEmpty())
        return -1;

    m_options.searchString = rSearchTerm;
    utl::TextSearch textSearch(m_options);

    for (int i = m_xFunctionBox->n_children(); i > 0; --i)
    {
        int nEntry = i - 1;
        OUString aStr = m_xFunctionBox->get_text(nEntry);
        sal_Int32 aStartPos = 0;
        sal_Int32 aEndPos = aStr.getLength();

        if (!textSearch.SearchForward(aStr, &aStartPos, &aEndPos))
            m_xFunctionBox->remove(nEntry);
    }

    return m_xFunctionBox->n_children() ? 0 : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
