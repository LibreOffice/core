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

#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/sfxresid.hxx>
#include <svl/stritem.hxx>
#include <vcl/treelistentry.hxx>

#include <sal/macros.h>
#include <vcl/builderfactory.hxx>

#include <strings.hrc>
#include <sfx2/strings.hrc>
#include <svx/svxids.hrc>


// include interface declarations
#include <com/sun/star/awt/KeyModifier.hpp>
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
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

// include search util
#include <com/sun/star/util/SearchFlags.hpp>
#include <com/sun/star/util/SearchAlgorithms2.hpp>
#include <unotools/textsearch.hxx>

// include other projects
#include <comphelper/processfactory.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <vcl/svlbitm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>
// namespaces

using namespace css;

static const char FOLDERNAME_UICONFIG   [] = "Configurations2";

static const char MEDIATYPE_PROPNAME    [] = "MediaType";

static const sal_uInt16 KEYCODE_ARRAY[] =
{
    KEY_F1,
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
    KEY_MOD1 | KEY_SEMICOLON,
    KEY_MOD1 | KEY_QUOTERIGHT,
    KEY_MOD1 | KEY_BRACKETLEFT,
    KEY_MOD1 | KEY_BRACKETRIGHT,
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
    KEY_SHIFT | KEY_MOD1 | KEY_SEMICOLON,
    KEY_SHIFT | KEY_MOD1 | KEY_QUOTERIGHT,
    KEY_SHIFT | KEY_MOD1 | KEY_BRACKETLEFT,
    KEY_SHIFT | KEY_MOD1 | KEY_BRACKETRIGHT,
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
    KEY_MOD2 | KEY_SEMICOLON,
    KEY_MOD2 | KEY_QUOTERIGHT,
    KEY_MOD2 | KEY_BRACKETLEFT,
    KEY_MOD2 | KEY_BRACKETRIGHT,
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
    KEY_SHIFT | KEY_MOD2 | KEY_SEMICOLON,
    KEY_SHIFT | KEY_MOD2 | KEY_QUOTERIGHT,
    KEY_SHIFT | KEY_MOD2 | KEY_BRACKETLEFT,
    KEY_SHIFT | KEY_MOD2 | KEY_BRACKETRIGHT,
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
    KEY_MOD1 | KEY_MOD2 | KEY_SEMICOLON,
    KEY_MOD1 | KEY_MOD2 | KEY_QUOTERIGHT,
    KEY_MOD1 | KEY_MOD2 | KEY_BRACKETLEFT,
    KEY_MOD1 | KEY_MOD2 | KEY_BRACKETRIGHT,
    KEY_MOD1 | KEY_MOD2 | KEY_POINT,
    KEY_MOD1 | KEY_MOD2 | KEY_COMMA,
    KEY_MOD1 | KEY_MOD2 | KEY_TILDE,

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
    KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_SEMICOLON,
    KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_QUOTERIGHT,
    KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_BRACKETLEFT,
    KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_BRACKETRIGHT,
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
    KEY_SHIFT | KEY_MOD1 | KEY_MOD2 | KEY_DELETE

#ifdef __APPLE__
   ,KEY_MOD3 | KEY_0,
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
    KEY_MOD3 | KEY_SEMICOLON,
    KEY_MOD3 | KEY_QUOTERIGHT,
    KEY_MOD3 | KEY_BRACKETLEFT,
    KEY_MOD3 | KEY_BRACKETRIGHT,
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
    KEY_SHIFT | KEY_MOD3 | KEY_SEMICOLON,
    KEY_SHIFT | KEY_MOD3 | KEY_QUOTERIGHT,
    KEY_SHIFT | KEY_MOD3 | KEY_BRACKETLEFT,
    KEY_SHIFT | KEY_MOD3 | KEY_BRACKETRIGHT,
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
    KEY_SHIFT | KEY_MOD3 | KEY_DELETE
#endif
};

static const sal_uInt16 KEYCODE_ARRAY_SIZE = SAL_N_ELEMENTS(KEYCODE_ARRAY);

extern "C" SAL_DLLPUBLIC_EXPORT void makeSfxAccCfgTabListBox(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)
{
    WinBits nWinBits = WB_TABSTOP;

    OUString sBorder = BuilderUtils::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<SfxAccCfgTabListBox_Impl>::Create(pParent, nWinBits);
}

SfxAccCfgTabListBox_Impl::~SfxAccCfgTabListBox_Impl()
{
    disposeOnce();
}

/** select the entry, which match the current key input ... excepting
    keys, which are used for the dialog itself.
  */
void SfxAccCfgTabListBox_Impl::KeyInput(const KeyEvent& aKey)
{
    vcl::KeyCode aCode1 = aKey.GetKeyCode();
    sal_uInt16 nCode1 = aCode1.GetCode();
    sal_uInt16 nMod1 = aCode1.GetModifier();

    // is it related to our list box ?
    if (
        (nCode1 != KEY_DOWN    ) &&
        (nCode1 != KEY_UP      ) &&
        (nCode1 != KEY_LEFT    ) &&
        (nCode1 != KEY_RIGHT   ) &&
        (nCode1 != KEY_PAGEUP  ) &&
        (nCode1 != KEY_PAGEDOWN)
       )
    {
        SvTreeListEntry* pEntry = First();
        while (pEntry)
        {
            TAccInfo* pUserData = static_cast<TAccInfo*>(pEntry->GetUserData());
            if (pUserData)
            {
                sal_uInt16 nCode2 = pUserData->m_aKey.GetCode();
                sal_uInt16 nMod2  = pUserData->m_aKey.GetModifier();

                if ((nCode1 == nCode2) &&
                    (nMod1  == nMod2 ))
                {
                    Select(pEntry);
                    MakeVisible(pEntry);
                    return;
                }
            }
            pEntry = Next(pEntry);
        }
    }

    // no - handle it as normal dialog input
    SvTabListBox::KeyInput(aKey);
}

SfxAcceleratorConfigPage::SfxAcceleratorConfigPage(TabPageParent pParent, const SfxItemSet& aSet )
    : SfxTabPage(pParent, "cui/ui/accelconfigpage.ui", "AccelConfigPage", &aSet)
    , m_pMacroInfoItem()
    , aLoadAccelConfigStr(CuiResId(RID_SVXSTR_LOADACCELCONFIG))
    , aSaveAccelConfigStr(CuiResId(RID_SVXSTR_SAVEACCELCONFIG))
    , aFilterCfgStr(CuiResId(RID_SVXSTR_FILTERNAME_CFG))
    , m_bStylesInfoInitialized(false)
    , m_xGlobal()
    , m_xModule()
    , m_xAct()
    , m_aUpdateDataTimer("UpdateDataTimer")
    , m_xEntriesBox(m_xBuilder->weld_tree_view("shortcuts"))
    , m_xOfficeButton(m_xBuilder->weld_radio_button("office"))
    , m_xModuleButton(m_xBuilder->weld_radio_button("module"))
    , m_xChangeButton(m_xBuilder->weld_button("change"))
    , m_xRemoveButton(m_xBuilder->weld_button("delete"))
    , m_xGroupLBox(new CuiConfigGroupListBox(m_xBuilder->weld_tree_view("category")))
    , m_xFunctionBox(new CuiConfigFunctionListBox(m_xBuilder->weld_tree_view("function")))
    , m_xKeyBox(m_xBuilder->weld_tree_view("keys"))
    , m_xSearchEdit(m_xBuilder->weld_entry("searchEntry"))
    , m_xLoadButton(m_xBuilder->weld_button("load"))
    , m_xSaveButton(m_xBuilder->weld_button("save"))
    , m_xResetButton(m_xBuilder->weld_button("reset"))
{
    Size aSize(LogicToPixel(Size(174, 100), MapMode(MapUnit::MapAppFont)));
    m_xEntriesBox->set_size_request(aSize.Width(), aSize.Height());
    aSize = LogicToPixel(Size(78 , 91), MapMode(MapUnit::MapAppFont));
    m_xGroupLBox->set_size_request(aSize.Width(), aSize.Height());
    aSize = LogicToPixel(Size(88, 91), MapMode(MapUnit::MapAppFont));
    m_xFunctionBox->set_size_request(aSize.Width(), aSize.Height());
    aSize = LogicToPixel(Size(80, 91), MapMode(MapUnit::MapAppFont));
    m_xKeyBox->set_size_request(aSize.Width(), aSize.Height());

    aFilterAllStr = SfxResId( STR_SFX_FILTERNAME_ALL );

    // install handler functions
    m_xChangeButton->connect_clicked( LINK( this, SfxAcceleratorConfigPage, ChangeHdl ));
    m_xRemoveButton->connect_clicked( LINK( this, SfxAcceleratorConfigPage, RemoveHdl ));
    m_xEntriesBox->connect_changed ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_xGroupLBox->connect_changed  ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_xFunctionBox->connect_changed( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_xKeyBox->connect_changed     ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_xLoadButton->connect_clicked  ( LINK( this, SfxAcceleratorConfigPage, Load      ));
    m_xSaveButton->connect_clicked  ( LINK( this, SfxAcceleratorConfigPage, Save      ));
    m_xResetButton->connect_clicked ( LINK( this, SfxAcceleratorConfigPage, Default   ));
    m_xOfficeButton->connect_clicked( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));
    m_xModuleButton->connect_clicked( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));
    m_xSearchEdit->connect_changed( LINK( this, SfxAcceleratorConfigPage, SearchUpdateHdl ));
    m_xSearchEdit->connect_focus_out(LINK(this, SfxAcceleratorConfigPage, FocusOut_Impl));

    // detect max keyname width
    int nMaxWidth  = 0;
    for (unsigned short i : KEYCODE_ARRAY)
    {
        int nTmp = m_xEntriesBox->get_pixel_size(vcl::KeyCode(i).GetName()).Width();
        if ( nTmp > nMaxWidth )
            nMaxWidth = nTmp;
    }
    // recalc second tab
    auto nNewTab = nMaxWidth + 5; // additional space

    // initialize Entriesbox
    std::vector<int> aWidths;
    aWidths.push_back(nNewTab);
    m_xEntriesBox->set_column_fixed_widths(aWidths);

    //Initialize search util
    m_options.AlgorithmType2 = util::SearchAlgorithms2::ABSOLUTE;
    m_options.transliterateFlags |= TransliterationFlags::IGNORE_CASE;
    m_options.searchFlag |= (util::SearchFlags::REG_NOT_BEGINOFLINE |
                                        util::SearchFlags::REG_NOT_ENDOFLINE);
    // initialize GroupBox
    m_xGroupLBox->SetFunctionListBox(m_xFunctionBox.get());

    // initialize KeyBox
    m_xKeyBox->make_sorted();

    m_aUpdateDataTimer.SetInvokeHandler(LINK(this, SfxAcceleratorConfigPage, ImplUpdateDataHdl));
    m_aUpdateDataTimer.SetDebugName( "SfxAcceleratorConfigPage UpdateDataTimer" );
    m_aUpdateDataTimer.SetTimeout(EDIT_UPDATEDATA_TIMEOUT);

    m_aFillGroupIdle.SetInvokeHandler(LINK(this, SfxAcceleratorConfigPage, TimeOut_Impl));
    m_aFillGroupIdle.SetPriority(TaskPriority::HIGHEST);
    m_aFillGroupIdle.SetDebugName("SfxAcceleratorConfigPage m_aFillGroupIdle");
}

SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
    disposeOnce();
}

void SfxAcceleratorConfigPage::dispose()
{
    m_aFillGroupIdle.Stop();

    // free memory - remove all dynamic user data
    for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
    {
        TAccInfo* pUserData = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(i).toInt64());
        delete pUserData;
    }

    SfxTabPage::dispose();
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
        if ( !m_xFrame.is() )
        {
            uno::Reference<frame::XDesktop2> xDesktop = frame::Desktop::create( m_xContext );
            m_xFrame = xDesktop->getActiveFrame();
        }

        // identify module
        uno::Reference<frame::XModuleManager2> xModuleManager =
                 frame::ModuleManager::create(m_xContext);
        m_sModuleLongName = xModuleManager->identify(m_xFrame);
        comphelper::SequenceAsHashMap lModuleProps(xModuleManager->getByName(m_sModuleLongName));
        m_sModuleUIName    = lModuleProps.getUnpackedValueOrDefault("ooSetupFactoryUIName", OUString());

        // get global accelerator configuration
        m_xGlobal = css::ui::GlobalAcceleratorConfiguration::create(m_xContext);

        // get module accelerator configuration

        uno::Reference<ui::XModuleUIConfigurationManagerSupplier> xModuleCfgSupplier(
            ui::theModuleUIConfigurationManagerSupplier::get(m_xContext));
        uno::Reference<ui::XUIConfigurationManager> xUICfgManager =
            xModuleCfgSupplier->getUIConfigurationManager(m_sModuleLongName);
        m_xModule = xUICfgManager->getShortCutManager();
    }
    catch(const uno::RuntimeException&)
    {
        throw;
    }
    catch(const uno::Exception&)
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
        TAccInfo*    pEntry   = new TAccInfo(i1, 0/*nListPos*/, aKey);
        m_xEntriesBox->append(OUString::number(reinterpret_cast<sal_Int64>(pEntry)), sKey);
        int nPos = m_xEntriesBox->n_children() - 1;
        m_xEntriesBox->set_text(nPos, OUString(), 1);
        m_xEntriesBox->set_sensitive(nPos, true);
    }

    // Assign all commands to its shortcuts - reading the accelerator config.
    uno::Sequence<awt::KeyEvent> lKeys = xAccMgr->getAllKeyEvents();
    sal_Int32 c2 = lKeys.getLength();
    sal_Int32 i2 = 0;

    for (i2=0; i2<c2; ++i2)
    {
        const awt::KeyEvent& aAWTKey  = lKeys[i2];
        OUString sCommand = xAccMgr->getCommandByKeyEvent(aAWTKey);
        OUString sLabel = GetLabel4Command(sCommand);
        vcl::KeyCode aKeyCode = svt::AcceleratorExecute::st_AWTKey2VCLKey(aAWTKey);
        sal_Int32 nPos = MapKeyCodeToPos(aKeyCode);

        if (nPos == -1)
            continue;

        m_xEntriesBox->set_text(nPos, sLabel, 1);

        TAccInfo* pEntry = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(nPos).toInt64());
        pEntry->m_bIsConfigurable = true;

        pEntry->m_sCommand = sCommand;
    }

    // Map the VCL hardcoded key codes and mark them as not changeable
    sal_uLong c3 = Application::GetReservedKeyCodeCount();
    sal_uLong i3 = 0;
    for (i3 = 0; i3 < c3; ++i3)
    {
        const vcl::KeyCode* pKeyCode = Application::GetReservedKeyCode(i3);
        sal_Int32 nPos = MapKeyCodeToPos(*pKeyCode);

        if (nPos == -1)
            continue;

        // Hardcoded function mapped so no ID possible and mark entry as not changeable
        TAccInfo* pEntry = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(nPos).toInt64());
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
        TAccInfo* pUserData = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(i).toInt64());
        OUString sCommand;
        awt::KeyEvent aAWTKey;

        if (pUserData)
        {
            sCommand = pUserData->m_sCommand;
            aAWTKey  = svt::AcceleratorExecute::st_VCLKey2AWTKey(pUserData->m_aKey);
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

void SfxAcceleratorConfigPage::ResetConfig()
{
    m_xEntriesBox->clear();
}

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
    StartFileDialog( StartFileDialogType::Open, aLoadAccelConfigStr );
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Save, weld::Button&, void)
{
    StartFileDialog( StartFileDialogType::SaveAs, aSaveAccelConfigStr );
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

    TAccInfo* pEntry = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(nPos).toInt64());
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

    TAccInfo* pEntry = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(nPos).toInt64());

    // remove function name from selected entry
    m_xEntriesBox->set_text(nPos, OUString(), 1);
    pEntry->m_sCommand.clear();

    SelectHdl(m_xFunctionBox->get_widget());
}

IMPL_LINK(SfxAcceleratorConfigPage, SelectHdl, weld::TreeView&, rListBox, void)
{
    // disable help
    Help::ShowBalloon( this, Point(), ::tools::Rectangle(), OUString() );
    if (&rListBox == m_xEntriesBox.get())
    {
        TAccInfo* pEntry = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_selected_id().toInt64());

        OUString sPossibleNewCommand = m_xFunctionBox->GetCurCommand();

        m_xRemoveButton->set_sensitive( false );
        m_xChangeButton->set_sensitive( false );

        if (pEntry->m_bIsConfigurable)
        {
            if (pEntry->isConfigured())
                m_xRemoveButton->set_sensitive(true);
            m_xChangeButton->set_sensitive( pEntry->m_sCommand != sPossibleNewCommand );
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
        m_xRemoveButton->set_sensitive( false );
        m_xChangeButton->set_sensitive( false );

        // #i36994 First selected can return zero!
        TAccInfo* pEntry = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_selected_id().toInt64());
        if (pEntry)
        {
            OUString sPossibleNewCommand = m_xFunctionBox->GetCurCommand();

            if (pEntry->m_bIsConfigurable)
            {
                if (pEntry->isConfigured())
                    m_xRemoveButton->set_sensitive(true);
                m_xChangeButton->set_sensitive( pEntry->m_sCommand != sPossibleNewCommand );
            }

            // update key box
            m_xKeyBox->clear();
            for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
            {
                TAccInfo* pUserData = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(i).toInt64());
                if (pUserData && pUserData->m_sCommand == sPossibleNewCommand)
                {
                    m_xKeyBox->append(OUString::number(reinterpret_cast<sal_Int64>(pUserData)), pUserData->m_aKey.GetName());
                }
            }
        }
    }
    else
    {
        // goto selected "key" entry of the key box
        int nP2 = -1;
        TAccInfo* pU2 = reinterpret_cast<TAccInfo*>(m_xKeyBox->get_selected_id().toInt64());
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

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, RadioHdl, weld::Button&, void)
{
    uno::Reference<ui::XAcceleratorConfiguration> xOld = m_xAct;

    if (m_xOfficeButton->get_active())
        m_xAct = m_xGlobal;
    else if (m_xModuleButton->get_active())
        m_xAct = m_xModule;

    // nothing changed? => do nothing!
    if ( m_xAct.is() && ( xOld == m_xAct ) )
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
    weld::Window* pDialog = GetDialogFrameWeld();
    // perhaps the tabpage is part of a SingleTabDialog then pDialog == nullptr
    std::unique_ptr<weld::WaitObject> xWait(pDialog ? new weld::WaitObject(pDialog) : nullptr);

    weld::TreeView& rTreeView = m_xGroupLBox->get_widget();
    SelectHdl(rTreeView);
}

IMPL_LINK_NOARG(SfxAcceleratorConfigPage, LoadHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sCfgName;
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
        sCfgName = m_pFileDlg->GetPath();

    if ( sCfgName.isEmpty() )
        return;

    GetTabDialog()->EnterWait();

    uno::Reference<ui::XUIConfigurationManager> xCfgMgr;
    uno::Reference<embed::XStorage> xRootStorage; // we must hold the root storage alive, if xCfgMgr is used!

    try
    {
        // don't forget to release the storage afterwards!
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory(embed::StorageFactory::create(m_xContext));
        uno::Sequence<uno::Any> lArgs(2);
        lArgs[0] <<= sCfgName;
        lArgs[1] <<= css::embed::ElementModes::READ;

        xRootStorage.set(xStorageFactory->createInstanceWithArguments(lArgs), uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> xUIConfig = xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, embed::ElementModes::READ);
        if (xUIConfig.is())
        {
            uno::Reference<ui::XUIConfigurationManager2> xCfgMgr2 = ui::UIConfigurationManager::create(m_xContext);
            xCfgMgr2->setStorage(xUIConfig);
            xCfgMgr.set(xCfgMgr2, uno::UNO_QUERY_THROW);
        }

        if (xCfgMgr.is())
        {
            // open the configuration and update our UI
            uno::Reference<ui::XAcceleratorConfiguration> xTempAccMgr(xCfgMgr->getShortCutManager(), uno::UNO_QUERY_THROW);

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
            xComponent.set(xRootStorage, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch(const uno::RuntimeException&)
    {
        throw;
    }
    catch(const uno::Exception&)
    {}

    GetTabDialog()->LeaveWait();
}


IMPL_LINK_NOARG(SfxAcceleratorConfigPage, SaveHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sCfgName;
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
        sCfgName = m_pFileDlg->GetPath();

    if ( sCfgName.isEmpty() )
        return;

    GetTabDialog()->EnterWait();

    uno::Reference<embed::XStorage> xRootStorage;

    try
    {
        uno::Reference<lang::XSingleServiceFactory> xStorageFactory(embed::StorageFactory::create(m_xContext));
        uno::Sequence<uno::Any> lArgs(2);
        lArgs[0] <<= sCfgName;
        lArgs[1] <<= embed::ElementModes::WRITE;

        xRootStorage.set( xStorageFactory->createInstanceWithArguments(lArgs),
                          uno::UNO_QUERY_THROW);

        uno::Reference<embed::XStorage> xUIConfig(
                            xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, embed::ElementModes::WRITE),
                            uno::UNO_QUERY_THROW);
        uno::Reference<beans::XPropertySet> xUIConfigProps(
                            xUIConfig,
                            uno::UNO_QUERY_THROW);

        // set the correct media type if the storage was new created
        OUString sMediaType;
        xUIConfigProps->getPropertyValue(MEDIATYPE_PROPNAME) >>= sMediaType;
        if (sMediaType.isEmpty())
            xUIConfigProps->setPropertyValue(MEDIATYPE_PROPNAME, uno::Any(OUString("application/vnd.sun.xml.ui.configuration")));

        uno::Reference<ui::XUIConfigurationManager2> xCfgMgr = ui::UIConfigurationManager::create(m_xContext);
        xCfgMgr->setStorage(xUIConfig);

        // get the target configuration access and update with all shortcuts
        // which are set currently at the UI!
        // Don't copy the m_xAct content to it... because m_xAct will be updated
        // from the UI on pressing the button "OK" only. And inbetween it's not up to date!
        uno::Reference<ui::XAcceleratorConfiguration> xTargetAccMgr(xCfgMgr->getShortCutManager(), uno::UNO_QUERY_THROW);
        Apply(xTargetAccMgr);

        // commit (order is important!)
        uno::Reference<ui::XUIConfigurationPersistence> xCommit1(xTargetAccMgr, uno::UNO_QUERY_THROW);
        uno::Reference<ui::XUIConfigurationPersistence> xCommit2(xCfgMgr      , uno::UNO_QUERY_THROW);
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
            uno::Reference<lang::XComponent> xComponent(xCfgMgr, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            xComponent.set(xRootStorage, uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch(const uno::RuntimeException&)
    {
        throw;
    }
    catch(const uno::Exception&)
    {}

    GetTabDialog()->LeaveWait();
}


void SfxAcceleratorConfigPage::StartFileDialog( StartFileDialogType nType, const OUString& rTitle )
{
    bool bSave = nType == StartFileDialogType::SaveAs;
    short nDialogType = bSave ? ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION
                              : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
    m_pFileDlg.reset(new sfx2::FileDialogHelper(nDialogType, FileDialogFlags::NONE, GetFrameWeld()));

    m_pFileDlg->SetTitle( rTitle );
    m_pFileDlg->AddFilter( aFilterAllStr, FILEDIALOG_FILTER_ALL );
    m_pFileDlg->AddFilter( aFilterCfgStr, "*.cfg" );
    m_pFileDlg->SetCurrentFilter( aFilterCfgStr );

    Link<sfx2::FileDialogHelper*,void> aDlgClosedLink = bSave ? LINK( this, SfxAcceleratorConfigPage, SaveHdl )
                                : LINK( this, SfxAcceleratorConfigPage, LoadHdl );
    m_pFileDlg->StartExecuteModal( aDlgClosedLink );
}


bool SfxAcceleratorConfigPage::FillItemSet( SfxItemSet* )
{
    Apply(m_xAct);
    try
    {
        m_xAct->store();
    }
    catch(const uno::RuntimeException&)
    {
        throw;
    }
    catch(const uno::Exception&)
    {
        return false;
    }

    return true;
}


void SfxAcceleratorConfigPage::Reset( const SfxItemSet* rSet )
{
    // open accelerator configs
    // Note: It initialize some other members too, which are needed here ...
    // e.g. m_sModuleUIName!
    InitAccCfg();

    // change the description of the radio button, which switch to the module
    // dependent accelerator configuration
    OUString sButtonText = m_xModuleButton->get_label();
    sButtonText = m_xModuleButton->strip_mnemonic(sButtonText).replaceFirst("$(MODULE)", m_sModuleUIName);
    m_xModuleButton->set_label(sButtonText);

    if (m_xModule.is())
        m_xModuleButton->set_active(true);
    else
    {
        m_xModuleButton->hide();
        m_xOfficeButton->set_active(true);
    }

    RadioHdl(*m_xOfficeButton);

    const SfxPoolItem* pMacroItem=nullptr;
    if( SfxItemState::SET == rSet->GetItemState( SID_MACROINFO, true, &pMacroItem ) )
    {
        m_pMacroInfoItem = &dynamic_cast<const SfxMacroInfoItem&>(*pMacroItem);
        m_xGroupLBox->SelectMacro( m_pMacroInfoItem );
    }
}

sal_Int32 SfxAcceleratorConfigPage::MapKeyCodeToPos(const vcl::KeyCode& aKey) const
{
    sal_uInt16 nCode1 = aKey.GetCode() + aKey.GetModifier();
    for (int i = 0, nCount = m_xEntriesBox->n_children(); i < nCount; ++i)
    {
        TAccInfo* pUserData = reinterpret_cast<TAccInfo*>(m_xEntriesBox->get_id(i).toInt64());
        if (pUserData)
        {
            sal_uInt16 nCode2 = pUserData->m_aKey.GetCode()+pUserData->m_aKey.GetModifier();
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
            OUString sLabel = lProps.getUnpackedValueOrDefault("Name", OUString());
            if (!sLabel.isEmpty())
                return sLabel;
        }
    }
    catch(const uno::RuntimeException&)
    {
        throw;
    }
    catch(const uno::Exception&)
    {}

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
int SfxAcceleratorConfigPage::applySearchFilter(OUString const & rSearchTerm)
{
    if (rSearchTerm.isEmpty())
        return -1;

    m_options.searchString = rSearchTerm;
    utl::TextSearch textSearch( m_options );

    for (int i = m_xFunctionBox->n_children(); i > 0; --i)
    {
        int nEntry = i - 1;
        OUString aStr = m_xFunctionBox->get_text(nEntry);
        sal_Int32 aStartPos = 0;
        sal_Int32 aEndPos = aStr.getLength();

        if (!textSearch.SearchForward( aStr, &aStartPos, &aEndPos ))
            m_xFunctionBox->remove(nEntry);
    }

    return m_xFunctionBox->n_children() ? 0 : -1;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
