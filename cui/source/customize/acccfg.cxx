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

#include "acccfg.hxx"
#include "cfgutil.hxx"
#include <dialmgr.hxx>

#include <sfx2/msg.hxx>
#include <sfx2/app.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/minfitem.hxx>
#include <sfx2/sfxresid.hxx>
#include <svl/stritem.hxx>
#include "svtools/treelistentry.hxx"

#include <sal/macros.h>
#include <vcl/builderfactory.hxx>

#include "cuires.hrc"
#include "acccfg.hrc"

#include <svx/svxids.hrc>


// include interface declarations
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFrame.hpp>
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

// include other projects
#include <comphelper/processfactory.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/svlbitm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>

// namespaces

using namespace css;

static const char MODULEPROP_SHORTNAME  [] = "ooSetupFactoryShortName";
static const char MODULEPROP_UINAME     [] = "ooSetupFactoryUIName";
static const char CMDPROP_UINAME        [] = "Name";

static const char FOLDERNAME_UICONFIG   [] = "Configurations2";

static const char MEDIATYPE_PROPNAME    [] = "MediaType";
static const char MEDIATYPE_UICONFIG    [] = "application/vnd.sun.xml.ui.configuration";

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
};

static const sal_uInt16 KEYCODE_ARRAY_SIZE = SAL_N_ELEMENTS(KEYCODE_ARRAY);

// seems to be needed to layout the list box, which shows all
// assignable shortcuts
static long AccCfgTabs[] =
{
    2,  // Number of Tabs
    0,
    120 // Function
};


class SfxAccCfgLBoxString_Impl : public SvLBoxString
{
public:
    SfxAccCfgLBoxString_Impl(SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& sText);

    virtual ~SfxAccCfgLBoxString_Impl();

    virtual void Paint(const Point& aPos, SvTreeListBox& rDevice, vcl::RenderContext& rRenderContext,
                       const SvViewDataEntry* pView, const SvTreeListEntry& rEntry) override;
};


SfxAccCfgLBoxString_Impl::SfxAccCfgLBoxString_Impl(SvTreeListEntry* pEntry, sal_uInt16 nFlags, const OUString& sText)
    : SvLBoxString(pEntry, nFlags, sText)
{}

SfxAccCfgLBoxString_Impl::~SfxAccCfgLBoxString_Impl()
{}

void SfxAccCfgLBoxString_Impl::Paint(const Point& aPos, SvTreeListBox& /*rDevice*/, vcl::RenderContext& rRenderContext,
                                     const SvViewDataEntry* /*pView*/, const SvTreeListEntry& rEntry)
{
    TAccInfo* pUserData = static_cast<TAccInfo*>(rEntry.GetUserData());
    if (!pUserData)
        return;

    if (pUserData->m_bIsConfigurable)
        rRenderContext.DrawText(aPos, GetText());
    else
        rRenderContext.DrawCtrlText(aPos, GetText(), 0, -1, DrawTextFlags::Disable);

}

VCL_BUILDER_DECL_FACTORY(SfxAccCfgTabListBox)
{
    WinBits nWinBits = WB_TABSTOP;

    OString sBorder = VclBuilder::extractCustomProperty(rMap);
    if (!sBorder.isEmpty())
       nWinBits |= WB_BORDER;

    rRet = VclPtr<SfxAccCfgTabListBox_Impl>::Create(pParent, nWinBits);
}

void SfxAccCfgTabListBox_Impl::InitEntry(SvTreeListEntry* pEntry,
                                         const OUString& rText,
                                         const Image& rImage1,
                                         const Image& rImage2,
                                         SvLBoxButtonKind eButtonKind)
{
    SvTabListBox::InitEntry(pEntry, rText, rImage1, rImage2, eButtonKind);
}

SfxAccCfgTabListBox_Impl::~SfxAccCfgTabListBox_Impl()
{
    disposeOnce();
}

void SfxAccCfgTabListBox_Impl::dispose()
{
    m_pAccelConfigPage.clear();
    SvTabListBox::dispose();
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

SfxAcceleratorConfigPage::SfxAcceleratorConfigPage( vcl::Window* pParent, const SfxItemSet& aSet )
    : SfxTabPage(pParent, "AccelConfigPage", "cui/ui/accelconfigpage.ui", &aSet)
    , m_pMacroInfoItem()
    , m_pStringItem()
    , m_pFontItem()
    , m_pFileDlg(nullptr)
    , aLoadAccelConfigStr(CUI_RES(RID_SVXSTR_LOADACCELCONFIG))
    , aSaveAccelConfigStr(CUI_RES(RID_SVXSTR_SAVEACCELCONFIG))
    , aFilterCfgStr(CUI_RES(RID_SVXSTR_FILTERNAME_CFG))
    , m_bStylesInfoInitialized(false)
    , m_xGlobal()
    , m_xModule()
    , m_xAct()
{
    get(m_pOfficeButton, "office");
    get(m_pModuleButton, "module");
    get(m_pChangeButton, "change");
    get(m_pRemoveButton, "delete");
    get(m_pLoadButton, "load");
    get(m_pSaveButton, "save");
    get(m_pResetButton, "reset");
    get(m_pEntriesBox, "shortcuts");
    Size aSize(LogicToPixel(Size(174, 100), MAP_APPFONT));
    m_pEntriesBox->set_width_request(aSize.Width());
    m_pEntriesBox->set_height_request(aSize.Height());
    m_pEntriesBox->SetAccelConfigPage(this);
    get(m_pGroupLBox, "category");
    aSize = LogicToPixel(Size(78 , 91), MAP_APPFONT);
    m_pGroupLBox->set_width_request(aSize.Width());
    m_pGroupLBox->set_height_request(aSize.Height());
    get(m_pFunctionBox, "function");
    aSize = LogicToPixel(Size(88, 91), MAP_APPFONT);
    m_pFunctionBox->set_width_request(aSize.Width());
    m_pFunctionBox->set_height_request(aSize.Height());
    get(m_pKeyBox, "keys");
    aSize = LogicToPixel(Size(80, 91), MAP_APPFONT);
    m_pKeyBox->set_width_request(aSize.Width());
    m_pKeyBox->set_height_request(aSize.Height());

    aFilterAllStr = SfxResId( STR_SFX_FILTERNAME_ALL );

    // install handler functions
    m_pChangeButton->SetClickHdl( LINK( this, SfxAcceleratorConfigPage, ChangeHdl ));
    m_pRemoveButton->SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RemoveHdl ));
    m_pEntriesBox->SetSelectHdl ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_pGroupLBox->SetSelectHdl  ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_pFunctionBox->SetSelectHdl( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_pKeyBox->SetSelectHdl     ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    m_pLoadButton->SetClickHdl  ( LINK( this, SfxAcceleratorConfigPage, Load      ));
    m_pSaveButton->SetClickHdl  ( LINK( this, SfxAcceleratorConfigPage, Save      ));
    m_pResetButton->SetClickHdl ( LINK( this, SfxAcceleratorConfigPage, Default   ));
    m_pOfficeButton->SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));
    m_pModuleButton->SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));

    // initialize Entriesbox
    m_pEntriesBox->SetStyle(m_pEntriesBox->GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN);
    m_pEntriesBox->SetSelectionMode(SINGLE_SELECTION);
    m_pEntriesBox->SetTabs(&AccCfgTabs[0]);
    m_pEntriesBox->Resize(); // OS: Hack for right selection
    m_pEntriesBox->SetSpaceBetweenEntries(0);
    m_pEntriesBox->SetDragDropMode(DragDropMode::NONE);

    // detect max keyname width
    long nMaxWidth  = 0;
    for ( sal_uInt16 i = 0; i < KEYCODE_ARRAY_SIZE; ++i )
    {
        long nTmp = GetTextWidth( vcl::KeyCode( KEYCODE_ARRAY[i] ).GetName() );
        if ( nTmp > nMaxWidth )
            nMaxWidth = nTmp;
    }
    // recalc second tab
    long nNewTab = PixelToLogic( Size( nMaxWidth, 0 ), MAP_APPFONT ).Width();
    nNewTab = nNewTab + 5; // additional space
    m_pEntriesBox->SetTab( 1, nNewTab );

    // initialize GroupBox
    m_pGroupLBox->SetFunctionListBox(m_pFunctionBox);

    // initialize KeyBox
    m_pKeyBox->SetStyle(m_pKeyBox->GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_SORT);
}

SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
    disposeOnce();
}

void SfxAcceleratorConfigPage::dispose()
{
    // free memory - remove all dynamic user data
    SvTreeListEntry* pEntry = m_pEntriesBox->First();
    while (pEntry)
    {
        TAccInfo* pUserData = static_cast<TAccInfo*>(pEntry->GetUserData());
        delete pUserData;
        pEntry = m_pEntriesBox->Next(pEntry);
    }

    pEntry = m_pKeyBox->First();
    while (pEntry)
    {
        TAccInfo* pUserData = static_cast<TAccInfo*>(pEntry->GetUserData());
        delete pUserData;
        pEntry = m_pKeyBox->Next(pEntry);
    }

    m_pEntriesBox->Clear();
    m_pKeyBox->Clear();

    delete m_pFileDlg;
    m_pFileDlg = nullptr;

    m_pEntriesBox.clear();
    m_pOfficeButton.clear();
    m_pModuleButton.clear();
    m_pChangeButton.clear();
    m_pRemoveButton.clear();
    m_pGroupLBox.clear();
    m_pFunctionBox.clear();
    m_pKeyBox.clear();
    m_pLoadButton.clear();
    m_pSaveButton.clear();
    m_pResetButton.clear();

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
        m_sModuleShortName = lModuleProps.getUnpackedValueOrDefault(MODULEPROP_SHORTNAME, OUString());
        m_sModuleUIName    = lModuleProps.getUnpackedValueOrDefault(MODULEPROP_UINAME   , OUString());

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

/** Initialize text columns with own class to enable custom painting
    This is needed as we have to paint disabled entries by ourself. No support for that in the
    original SvTabListBox!
  */
void SfxAcceleratorConfigPage::CreateCustomItems(SvTreeListEntry* pEntry,
                                                 const OUString& sCol1 ,
                                                 const OUString& sCol2)
{
    std::unique_ptr<SfxAccCfgLBoxString_Impl> pStringItem1(
            new SfxAccCfgLBoxString_Impl(pEntry, 0, sCol1));
    pEntry->ReplaceItem(std::move(pStringItem1), 1);

    std::unique_ptr<SfxAccCfgLBoxString_Impl> pStringItem2(
            new SfxAccCfgLBoxString_Impl(pEntry, 0, sCol2));
    pEntry->ReplaceItem(std::move(pStringItem2), 2);
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

        m_aStylesInfo.setModel(xModel);
        m_pFunctionBox->SetStylesInfo(&m_aStylesInfo);
        m_pGroupLBox->SetStylesInfo(&m_aStylesInfo);
        m_bStylesInfoInitialized = true;
    }

    // Insert all editable accelerators into list box. It is possible
    // that some accelerators are not mapped on the current system/keyboard
    // but we don't want to lose these mappings.
    sal_Int32 c1 = KEYCODE_ARRAY_SIZE;
    sal_Int32 i1  = 0;
    sal_Int32 nListPos = 0;
    for (i1 = 0; i1 < c1; ++i1)
    {
        vcl::KeyCode aKey = KEYCODE_ARRAY[i1];
        OUString sKey = aKey.GetName();
        if (sKey.isEmpty())
            continue;
        TAccInfo*    pEntry   = new TAccInfo(i1, nListPos, aKey);
        SvTreeListEntry* pLBEntry = m_pEntriesBox->InsertEntryToColumn(sKey, nullptr, TREELIST_APPEND, 0xFFFF);
        pLBEntry->SetUserData(pEntry);
    }

    // Assign all commands to its shortcuts - reading the accelerator config.
    uno::Sequence<awt::KeyEvent> lKeys = xAccMgr->getAllKeyEvents();
    sal_Int32 c2 = lKeys.getLength();
    sal_Int32 i2 = 0;
    sal_uInt16 nCol = m_pEntriesBox->TabCount()-1;

    for (i2=0; i2<c2; ++i2)
    {
        const awt::KeyEvent& aAWTKey  = lKeys[i2];
              OUString     sCommand = xAccMgr->getCommandByKeyEvent(aAWTKey);
              OUString     sLabel   = GetLabel4Command(sCommand);
              vcl::KeyCode aKeyCode = svt::AcceleratorExecute::st_AWTKey2VCLKey(aAWTKey);
              sal_uLong    nPos     = MapKeyCodeToPos(aKeyCode);

        if (nPos == TREELIST_ENTRY_NOTFOUND)
            continue;

        m_pEntriesBox->SetEntryText(sLabel, nPos, nCol);

        SvTreeListEntry* pLBEntry = m_pEntriesBox->GetEntry(nullptr, nPos);
        TAccInfo* pEntry = static_cast<TAccInfo*>(pLBEntry->GetUserData());

        pEntry->m_bIsConfigurable = true;
        pEntry->m_sCommand = sCommand;
        CreateCustomItems(pLBEntry, SvTabListBox::GetEntryText(pLBEntry, 0), sLabel);
    }

    // Map the VCL hardcoded key codes and mark them as not changeable
    sal_uLong c3 = Application::GetReservedKeyCodeCount();
    sal_uLong i3 = 0;
    for (i3 = 0; i3 < c3; ++i3)
    {
        const vcl::KeyCode* pKeyCode = Application::GetReservedKeyCode(i3);
        sal_uLong nPos = MapKeyCodeToPos(*pKeyCode);

        if (nPos == TREELIST_ENTRY_NOTFOUND)
            continue;

        // Hardcoded function mapped so no ID possible and mark entry as not changeable
        SvTreeListEntry* pLBEntry = m_pEntriesBox->GetEntry(nullptr, nPos);
        TAccInfo* pEntry = static_cast<TAccInfo*>(pLBEntry->GetUserData());

        pEntry->m_bIsConfigurable = false;
        CreateCustomItems(pLBEntry, SvTabListBox::GetEntryText(pLBEntry, 0), OUString());
    }
}

void SfxAcceleratorConfigPage::Apply(const uno::Reference<ui::XAcceleratorConfiguration>& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    // Go through the list from the bottom to the top ...
    // because logical accelerator must be preferred instead of
    // physical ones!
    SvTreeListEntry* pEntry = m_pEntriesBox->First();
    while (pEntry)
    {
        TAccInfo* pUserData = static_cast<TAccInfo*>(pEntry->GetUserData());
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
        catch(const uno::RuntimeException&)
        {
            throw;
        }
        catch(const uno::Exception&)
        {
        }

        pEntry = m_pEntriesBox->Next(pEntry);
    }
}

void SfxAcceleratorConfigPage::ResetConfig()
{
    m_pEntriesBox->Clear();
}

IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, Load, Button*, void)
{
    // ask for filename, where we should load the new config data from
    StartFileDialog( 0, aLoadAccelConfigStr );
}

IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, Save, Button*, void)
{
    StartFileDialog( WB_SAVEAS, aSaveAccelConfigStr );
}

IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, Default, Button*, void)
{
    uno::Reference<form::XReset> xReset(m_xAct, uno::UNO_QUERY);
    if (xReset.is())
        xReset->reset();

    m_pEntriesBox->SetUpdateMode(false);
    ResetConfig();
    Init(m_xAct);
    m_pEntriesBox->SetUpdateMode(true);
    m_pEntriesBox->Invalidate();
    m_pEntriesBox->Select(m_pEntriesBox->GetEntry(nullptr, 0));
}

IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, ChangeHdl, Button*, void)
{
    sal_uLong nPos = SvTreeList::GetRelPos( m_pEntriesBox->FirstSelected() );
    TAccInfo* pEntry = static_cast<TAccInfo*>(m_pEntriesBox->GetEntry(nullptr, nPos)->GetUserData());
    OUString sNewCommand = m_pFunctionBox->GetCurCommand();
    OUString sLabel = m_pFunctionBox->GetCurLabel();
    if (sLabel.isEmpty())
        sLabel = GetLabel4Command(sNewCommand);

    pEntry->m_sCommand = sNewCommand;
    sal_uInt16 nCol = m_pEntriesBox->TabCount() - 1;
    m_pEntriesBox->SetEntryText(sLabel, nPos, nCol);

    m_pFunctionBox->GetSelectHdl().Call( m_pFunctionBox );
}

IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, RemoveHdl, Button*, void)
{
    // get selected entry
    sal_uLong nPos = SvTreeList::GetRelPos( m_pEntriesBox->FirstSelected() );
    TAccInfo* pEntry = static_cast<TAccInfo*>(m_pEntriesBox->GetEntry(nullptr, nPos)->GetUserData());

    // remove function name from selected entry
    sal_uInt16 nCol = m_pEntriesBox->TabCount() - 1;
    m_pEntriesBox->SetEntryText( OUString(), nPos, nCol );
    (pEntry->m_sCommand).clear();

    m_pFunctionBox->GetSelectHdl().Call( m_pFunctionBox );
}

IMPL_LINK_TYPED( SfxAcceleratorConfigPage, SelectHdl, SvTreeListBox*, pListBox, void )
{
    // disable help
    Help::ShowBalloon( this, Point(), Rectangle(), OUString() );
    if (pListBox == m_pEntriesBox)
    {
        sal_uLong nPos = SvTreeList::GetRelPos( m_pEntriesBox->FirstSelected() );
        TAccInfo* pEntry = static_cast<TAccInfo*>(m_pEntriesBox->GetEntry(nullptr, nPos)->GetUserData());
        OUString sPossibleNewCommand = m_pFunctionBox->GetCurCommand();

        m_pRemoveButton->Enable( false );
        m_pChangeButton->Enable( false );

        if (pEntry->m_bIsConfigurable)
        {
            if (pEntry->isConfigured())
                m_pRemoveButton->Enable();
            m_pChangeButton->Enable( pEntry->m_sCommand != sPossibleNewCommand );
        }
    }
    else if ( pListBox == m_pGroupLBox )
    {
        m_pGroupLBox->GroupSelected();
        if ( !m_pFunctionBox->FirstSelected() )
            m_pChangeButton->Enable( false );
    }
    else if ( pListBox == m_pFunctionBox )
    {
        m_pRemoveButton->Enable( false );
        m_pChangeButton->Enable( false );

        // #i36994 First selected can return zero!
        SvTreeListEntry* pLBEntry = m_pEntriesBox->FirstSelected();
        if ( pLBEntry != nullptr )
        {
            sal_uLong nPos = SvTreeList::GetRelPos( pLBEntry );
            TAccInfo* pEntry = static_cast<TAccInfo*>(m_pEntriesBox->GetEntry(nullptr, nPos)->GetUserData());
            OUString sPossibleNewCommand = m_pFunctionBox->GetCurCommand();

            if (pEntry->m_bIsConfigurable)
            {
                if (pEntry->isConfigured())
                    m_pRemoveButton->Enable();
                m_pChangeButton->Enable( pEntry->m_sCommand != sPossibleNewCommand );
            }

            // update key box
            m_pKeyBox->Clear();
            SvTreeListEntry* pIt = m_pEntriesBox->First();
            while ( pIt )
            {
                TAccInfo* pUserData = static_cast<TAccInfo*>(pIt->GetUserData());
                if ( pUserData && pUserData->m_sCommand == sPossibleNewCommand )
                {
                    TAccInfo* pU1 = new TAccInfo(-1, -1, pUserData->m_aKey);
                    SvTreeListEntry* pE1 = m_pKeyBox->InsertEntry( pUserData->m_aKey.GetName(), nullptr, true );
                    pE1->SetUserData(pU1);
                    pE1->EnableChildrenOnDemand( false );
                }
                pIt = m_pEntriesBox->Next(pIt);
            }
        }
    }
    else
    {
        // goto selected "key" entry of the key box
        SvTreeListEntry* pE2 = nullptr;
        TAccInfo* pU2 = nullptr;
        sal_uLong nP2 = TREELIST_ENTRY_NOTFOUND;
        SvTreeListEntry* pE3 = nullptr;

        pE2 = m_pKeyBox->FirstSelected();
        if (pE2)
            pU2 = static_cast<TAccInfo*>(pE2->GetUserData());
        if (pU2)
            nP2 = MapKeyCodeToPos(pU2->m_aKey);
        if (nP2 != TREELIST_ENTRY_NOTFOUND)
            pE3 = m_pEntriesBox->GetEntry( nullptr, nP2 );
        if (pE3)
        {
            m_pEntriesBox->Select( pE3 );
            m_pEntriesBox->MakeVisible( pE3 );
        }
    }
}

IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, RadioHdl, Button*, void)
{
    uno::Reference<ui::XAcceleratorConfiguration> xOld = m_xAct;

    if (m_pOfficeButton->IsChecked())
        m_xAct = m_xGlobal;
    else if (m_pModuleButton->IsChecked())
        m_xAct = m_xModule;

    // nothing changed? => do nothing!
    if ( m_xAct.is() && ( xOld == m_xAct ) )
        return;

    m_pEntriesBox->SetUpdateMode( false );
    ResetConfig();
    Init(m_xAct);
    m_pEntriesBox->SetUpdateMode( true );
    m_pEntriesBox->Invalidate();

    m_pGroupLBox->Init(m_xContext, m_xFrame, m_sModuleLongName, true);

    // pb: #133213# do not select NULL entries
    SvTreeListEntry* pEntry = m_pEntriesBox->GetEntry( nullptr, 0 );
    if ( pEntry )
        m_pEntriesBox->Select( pEntry );
    pEntry = m_pGroupLBox->GetEntry( nullptr, 0 );
    if ( pEntry )
        m_pGroupLBox->Select( pEntry );

    m_pFunctionBox->GetSelectHdl().Call( m_pFunctionBox );
}


IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, LoadHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sCfgName;
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
        sCfgName = m_pFileDlg->GetPath();

    if ( sCfgName.isEmpty() )
        return;

    GetTabDialog()->EnterWait();

    uno::Reference<frame::XModel> xDoc;
    uno::Reference<ui::XUIConfigurationManager> xCfgMgr;
    uno::Reference<embed::XStorage> xRootStorage; // we must hold the root storage alive, if xCfgMgr is used!

    try
    {
        // first check if URL points to a document already loaded
        xDoc = SearchForAlreadyLoadedDoc(sCfgName);
        if (xDoc.is())
        {
            // Get ui config manager. There should always be one at the model.
            uno::Reference<ui::XUIConfigurationManagerSupplier> xCfgSupplier(xDoc, uno::UNO_QUERY_THROW);
            xCfgMgr = xCfgSupplier->getUIConfigurationManager();
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
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
        }

        if (xCfgMgr.is())
        {
            // open the configuration and update our UI
            uno::Reference<ui::XAcceleratorConfiguration> xTempAccMgr(xCfgMgr->getShortCutManager(), uno::UNO_QUERY_THROW);

            m_pEntriesBox->SetUpdateMode(false);
            ResetConfig();
            Init(xTempAccMgr);
            m_pEntriesBox->SetUpdateMode(true);
            m_pEntriesBox->Invalidate();
            m_pEntriesBox->Select(m_pEntriesBox->GetEntry(nullptr, 0));

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


IMPL_LINK_NOARG_TYPED(SfxAcceleratorConfigPage, SaveHdl, sfx2::FileDialogHelper*, void)
{
    assert(m_pFileDlg);

    OUString sCfgName;
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
        sCfgName = m_pFileDlg->GetPath();

    if ( sCfgName.isEmpty() )
        return;

    GetTabDialog()->EnterWait();

    uno::Reference<frame::XModel> xDoc;
    uno::Reference<ui::XUIConfigurationManager> xCfgMgr;
    uno::Reference<embed::XStorage> xRootStorage;

    try
    {
        // first check if URL points to a document already loaded
        xDoc = SearchForAlreadyLoadedDoc(sCfgName);
        if (xDoc.is())
        {
            // get config manager, force creation if there was none before
            uno::Reference<ui::XUIConfigurationManagerSupplier> xCfgSupplier(xDoc, uno::UNO_QUERY_THROW);
            xCfgMgr = xCfgSupplier->getUIConfigurationManager();
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
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
                xUIConfigProps->setPropertyValue(MEDIATYPE_PROPNAME, uno::makeAny(OUString(MEDIATYPE_UICONFIG)));

            uno::Reference<ui::XUIConfigurationManager2> xCfgMgr2 = ui::UIConfigurationManager::create(m_xContext);
            xCfgMgr2->setStorage(xUIConfig);
            xCfgMgr.set( xCfgMgr2, uno::UNO_QUERY_THROW );
        }

        if (xCfgMgr.is())
        {
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


void SfxAcceleratorConfigPage::StartFileDialog( WinBits nBits, const OUString& rTitle )
{
    bool bSave = ( ( nBits & WB_SAVEAS ) == WB_SAVEAS );
    short nDialogType = bSave ? ui::dialogs::TemplateDescription::FILESAVE_AUTOEXTENSION
                              : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
    delete m_pFileDlg;
    m_pFileDlg = new sfx2::FileDialogHelper( nDialogType, 0 );

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
    OUString sButtonText = m_pModuleButton->GetText();
    sButtonText = sButtonText.replaceFirst("$(MODULE)", m_sModuleUIName);
    m_pModuleButton->SetText(sButtonText);

    if (m_xModule.is())
        m_pModuleButton->Check();
    else
    {
        m_pModuleButton->Hide();
        m_pOfficeButton->Check();
    }

    RadioHdl(nullptr);

    const SfxPoolItem* pMacroItem=nullptr;
    if( SfxItemState::SET == rSet->GetItemState( SID_MACROINFO, true, &pMacroItem ) )
    {
        m_pMacroInfoItem = &dynamic_cast<const SfxMacroInfoItem&>(*pMacroItem);
        m_pGroupLBox->SelectMacro( m_pMacroInfoItem );
    }
    else
    {
        const SfxPoolItem* pStringItem=nullptr;
        if( SfxItemState::SET == rSet->GetItemState( SID_CHARMAP, true, &pStringItem ) )
            m_pStringItem = dynamic_cast<const SfxStringItem*>( pStringItem  );

        const SfxPoolItem* pFontItem=nullptr;
        if( SfxItemState::SET == rSet->GetItemState( SID_ATTR_SPECIALCHAR, true, &pFontItem ) )
            m_pFontItem = dynamic_cast<const SfxStringItem*>( pFontItem  );
    }
}


sal_uLong SfxAcceleratorConfigPage::MapKeyCodeToPos(const vcl::KeyCode& aKey) const
{
    sal_uInt16 nCode1 = aKey.GetCode() + aKey.GetModifier();
    SvTreeListEntry* pEntry = m_pEntriesBox->First();
    sal_uLong i = 0;

    while (pEntry)
    {
        TAccInfo* pUserData = static_cast<TAccInfo*>(pEntry->GetUserData());
        if (pUserData)
        {
            sal_uInt16 nCode2 = pUserData->m_aKey.GetCode()+pUserData->m_aKey.GetModifier();
            if (nCode1 == nCode2)
                return i;
        }
        pEntry = m_pEntriesBox->Next(pEntry);
        ++i;
    }

    return TREELIST_ENTRY_NOTFOUND;
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
            OUString sLabel = lProps.getUnpackedValueOrDefault(CMDPROP_UINAME, OUString());
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

uno::Reference<frame::XModel> SfxAcceleratorConfigPage::SearchForAlreadyLoadedDoc(const OUString& /*sName*/)
{
    return uno::Reference<frame::XModel>();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
