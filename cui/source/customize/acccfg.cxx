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

//-----------------------------------------------
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

#include <sal/macros.h>

#include "cuires.hrc"
#include "acccfg.hrc"

#include <svx/svxids.hrc> // SID_CHARMAP, SID_ATTR_SPECIALCHAR

//-----------------------------------------------
// include interface declarations
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/form/XReset.hpp>
#include <com/sun/star/ui/ModuleUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>

#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/XUIConfigurationManager.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>

#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>

//-----------------------------------------------
// include other projects
#include <comphelper/componentcontext.hxx>
#include <comphelper/processfactory.hxx>
#include <svtools/acceleratorexecute.hxx>
#include <svtools/svlbitm.hxx>
#include <vcl/svapp.hxx>
#include <vcl/help.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/sequenceashashmap.hxx>

//-----------------------------------------------
// namespaces

#ifdef css
    #error "ambigous praeprozessor directive for css ..."
#else
    namespace css = ::com::sun::star;
#endif

using namespace com::sun::star;


//-----------------------------------------------
static ::rtl::OUString SERVICE_STORAGEFACTORY           (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.embed.StorageFactory"                        ));
static ::rtl::OUString SERVICE_UICONFIGMGR              (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.UIConfigurationManager"              ));
static ::rtl::OUString SERVICE_DESKTOP                  (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop"                               ));
static ::rtl::OUString SERVICE_MODULEMANAGER            (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.ModuleManager"                  ));
static ::rtl::OUString SERVICE_GLOBALACCCFG             (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ui.GlobalAcceleratorConfiguration"      ));
static ::rtl::OUString SERVICE_UICMDDESCRIPTION         (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.UICommandDescription"             ));

static ::rtl::OUString MODULEPROP_SHORTNAME             (RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryShortName"                                  ));
static ::rtl::OUString MODULEPROP_UINAME                (RTL_CONSTASCII_USTRINGPARAM("ooSetupFactoryUIName"                                     ));
static ::rtl::OUString CMDPROP_UINAME                   (RTL_CONSTASCII_USTRINGPARAM("Name"                                                     ));

static ::rtl::OUString FOLDERNAME_UICONFIG              (RTL_CONSTASCII_USTRINGPARAM("Configurations2"                                          ));

static ::rtl::OUString MEDIATYPE_PROPNAME               (RTL_CONSTASCII_USTRINGPARAM("MediaType"                                                ));
static ::rtl::OUString MEDIATYPE_UICONFIG               (RTL_CONSTASCII_USTRINGPARAM("application/vnd.sun.xml.ui.configuration"                 ));

//-----------------------------------------------
static sal_uInt16 KEYCODE_ARRAY[] =
{
    KEY_F1       ,
    KEY_F2       ,
    KEY_F3       ,
    KEY_F4       ,
    KEY_F5       ,
    KEY_F6       ,
    KEY_F7       ,
    KEY_F8       ,
    KEY_F9       ,
    KEY_F10      ,
    KEY_F11      ,
    KEY_F12      ,
    KEY_F13      ,
    KEY_F14      ,
    KEY_F15      ,
    KEY_F16      ,

    KEY_DOWN     ,
    KEY_UP       ,
    KEY_LEFT     ,
    KEY_RIGHT    ,
    KEY_HOME     ,
    KEY_END      ,
    KEY_PAGEUP   ,
    KEY_PAGEDOWN ,
    KEY_RETURN   ,
    KEY_ESCAPE   ,
    KEY_BACKSPACE,
    KEY_INSERT   ,
    KEY_DELETE   ,

    KEY_OPEN        ,
    KEY_CUT         ,
    KEY_COPY        ,
    KEY_PASTE       ,
    KEY_UNDO        ,
    KEY_REPEAT      ,
    KEY_FIND        ,
    KEY_PROPERTIES  ,
    KEY_FRONT       ,
    KEY_CONTEXTMENU ,
    KEY_MENU        ,
    KEY_HELP        ,

    KEY_F1        | KEY_SHIFT,
    KEY_F2        | KEY_SHIFT,
    KEY_F3        | KEY_SHIFT,
    KEY_F4        | KEY_SHIFT,
    KEY_F5        | KEY_SHIFT,
    KEY_F6        | KEY_SHIFT,
    KEY_F7        | KEY_SHIFT,
    KEY_F8        | KEY_SHIFT,
    KEY_F9        | KEY_SHIFT,
    KEY_F10       | KEY_SHIFT,
    KEY_F11       | KEY_SHIFT,
    KEY_F12       | KEY_SHIFT,
    KEY_F13       | KEY_SHIFT,
    KEY_F14       | KEY_SHIFT,
    KEY_F15       | KEY_SHIFT,
    KEY_F16       | KEY_SHIFT,

    KEY_DOWN      | KEY_SHIFT,
    KEY_UP        | KEY_SHIFT,
    KEY_LEFT      | KEY_SHIFT,
    KEY_RIGHT     | KEY_SHIFT,
    KEY_HOME      | KEY_SHIFT,
    KEY_END       | KEY_SHIFT,
    KEY_PAGEUP    | KEY_SHIFT,
    KEY_PAGEDOWN  | KEY_SHIFT,
    KEY_RETURN    | KEY_SHIFT,
    KEY_SPACE     | KEY_SHIFT,
    KEY_ESCAPE    | KEY_SHIFT,
    KEY_BACKSPACE | KEY_SHIFT,
    KEY_INSERT    | KEY_SHIFT,
    KEY_DELETE    | KEY_SHIFT,

    KEY_0         | KEY_MOD1 ,
    KEY_1         | KEY_MOD1 ,
    KEY_2         | KEY_MOD1 ,
    KEY_3         | KEY_MOD1 ,
    KEY_4         | KEY_MOD1 ,
    KEY_5         | KEY_MOD1 ,
    KEY_6         | KEY_MOD1 ,
    KEY_7         | KEY_MOD1 ,
    KEY_8         | KEY_MOD1 ,
    KEY_9         | KEY_MOD1 ,
    KEY_A         | KEY_MOD1 ,
    KEY_B         | KEY_MOD1 ,
    KEY_C         | KEY_MOD1 ,
    KEY_D         | KEY_MOD1 ,
    KEY_E         | KEY_MOD1 ,
    KEY_F         | KEY_MOD1 ,
    KEY_G         | KEY_MOD1 ,
    KEY_H         | KEY_MOD1 ,
    KEY_I         | KEY_MOD1 ,
    KEY_J         | KEY_MOD1 ,
    KEY_K         | KEY_MOD1 ,
    KEY_L         | KEY_MOD1 ,
    KEY_M         | KEY_MOD1 ,
    KEY_N         | KEY_MOD1 ,
    KEY_O         | KEY_MOD1 ,
    KEY_P         | KEY_MOD1 ,
    KEY_Q         | KEY_MOD1 ,
    KEY_R         | KEY_MOD1 ,
    KEY_S         | KEY_MOD1 ,
    KEY_T         | KEY_MOD1 ,
    KEY_U         | KEY_MOD1 ,
    KEY_V         | KEY_MOD1 ,
    KEY_W         | KEY_MOD1 ,
    KEY_X         | KEY_MOD1 ,
    KEY_Y         | KEY_MOD1 ,
    KEY_Z         | KEY_MOD1 ,
    KEY_SEMICOLON    | KEY_MOD1 ,
    KEY_BRACKETLEFT  | KEY_MOD1 ,
    KEY_BRACKETRIGHT | KEY_MOD1,
    KEY_POINT    | KEY_MOD1 ,

    KEY_F1        | KEY_MOD1 ,
    KEY_F2        | KEY_MOD1 ,
    KEY_F3        | KEY_MOD1 ,
    KEY_F4        | KEY_MOD1 ,
    KEY_F5        | KEY_MOD1 ,
    KEY_F6        | KEY_MOD1 ,
    KEY_F7        | KEY_MOD1 ,
    KEY_F8        | KEY_MOD1 ,
    KEY_F9        | KEY_MOD1 ,
    KEY_F10       | KEY_MOD1 ,
    KEY_F11       | KEY_MOD1 ,
    KEY_F12       | KEY_MOD1 ,
    KEY_F13       | KEY_MOD1 ,
    KEY_F14       | KEY_MOD1 ,
    KEY_F15       | KEY_MOD1 ,
    KEY_F16       | KEY_MOD1 ,

    KEY_DOWN      | KEY_MOD1 ,
    KEY_UP        | KEY_MOD1 ,
    KEY_LEFT      | KEY_MOD1 ,
    KEY_RIGHT     | KEY_MOD1 ,
    KEY_HOME      | KEY_MOD1 ,
    KEY_END       | KEY_MOD1 ,
    KEY_PAGEUP    | KEY_MOD1 ,
    KEY_PAGEDOWN  | KEY_MOD1 ,
    KEY_RETURN    | KEY_MOD1 ,
    KEY_SPACE     | KEY_MOD1 ,
    KEY_BACKSPACE | KEY_MOD1 ,
    KEY_INSERT    | KEY_MOD1 ,
    KEY_DELETE    | KEY_MOD1 ,

    KEY_ADD       | KEY_MOD1 ,
    KEY_SUBTRACT  | KEY_MOD1 ,
    KEY_MULTIPLY  | KEY_MOD1 ,
    KEY_DIVIDE    | KEY_MOD1 ,

    KEY_0         | KEY_SHIFT | KEY_MOD1,
    KEY_1         | KEY_SHIFT | KEY_MOD1,
    KEY_2         | KEY_SHIFT | KEY_MOD1,
    KEY_3         | KEY_SHIFT | KEY_MOD1,
    KEY_4         | KEY_SHIFT | KEY_MOD1,
    KEY_5         | KEY_SHIFT | KEY_MOD1,
    KEY_6         | KEY_SHIFT | KEY_MOD1,
    KEY_7         | KEY_SHIFT | KEY_MOD1,
    KEY_8         | KEY_SHIFT | KEY_MOD1,
    KEY_9         | KEY_SHIFT | KEY_MOD1,
    KEY_A         | KEY_SHIFT | KEY_MOD1,
    KEY_B         | KEY_SHIFT | KEY_MOD1,
    KEY_C         | KEY_SHIFT | KEY_MOD1,
    KEY_D         | KEY_SHIFT | KEY_MOD1,
    KEY_E         | KEY_SHIFT | KEY_MOD1,
    KEY_F         | KEY_SHIFT | KEY_MOD1,
    KEY_G         | KEY_SHIFT | KEY_MOD1,
    KEY_H         | KEY_SHIFT | KEY_MOD1,
    KEY_I         | KEY_SHIFT | KEY_MOD1,
    KEY_J         | KEY_SHIFT | KEY_MOD1,
    KEY_K         | KEY_SHIFT | KEY_MOD1,
    KEY_L         | KEY_SHIFT | KEY_MOD1,
    KEY_M         | KEY_SHIFT | KEY_MOD1,
    KEY_N         | KEY_SHIFT | KEY_MOD1,
    KEY_O         | KEY_SHIFT | KEY_MOD1,
    KEY_P         | KEY_SHIFT | KEY_MOD1,
    KEY_Q         | KEY_SHIFT | KEY_MOD1,
    KEY_R         | KEY_SHIFT | KEY_MOD1,
    KEY_S         | KEY_SHIFT | KEY_MOD1,
    KEY_T         | KEY_SHIFT | KEY_MOD1,
    KEY_U         | KEY_SHIFT | KEY_MOD1,
    KEY_V         | KEY_SHIFT | KEY_MOD1,
    KEY_W         | KEY_SHIFT | KEY_MOD1,
    KEY_X         | KEY_SHIFT | KEY_MOD1,
    KEY_Y         | KEY_SHIFT | KEY_MOD1,
    KEY_Z         | KEY_SHIFT | KEY_MOD1,
    KEY_SEMICOLON    | KEY_SHIFT | KEY_MOD1 ,
    KEY_BRACKETLEFT  | KEY_SHIFT | KEY_MOD1 ,
    KEY_BRACKETRIGHT | KEY_SHIFT | KEY_MOD1,
    KEY_POINT    | KEY_SHIFT | KEY_MOD1,

    KEY_F1        | KEY_SHIFT | KEY_MOD1,
    KEY_F2        | KEY_SHIFT | KEY_MOD1,
    KEY_F3        | KEY_SHIFT | KEY_MOD1,
    KEY_F4        | KEY_SHIFT | KEY_MOD1,
    KEY_F5        | KEY_SHIFT | KEY_MOD1,
    KEY_F6        | KEY_SHIFT | KEY_MOD1,
    KEY_F7        | KEY_SHIFT | KEY_MOD1,
    KEY_F8        | KEY_SHIFT | KEY_MOD1,
    KEY_F9        | KEY_SHIFT | KEY_MOD1,
    KEY_F10       | KEY_SHIFT | KEY_MOD1,
    KEY_F11       | KEY_SHIFT | KEY_MOD1,
    KEY_F12       | KEY_SHIFT | KEY_MOD1,
    KEY_F13       | KEY_SHIFT | KEY_MOD1,
    KEY_F14       | KEY_SHIFT | KEY_MOD1,
    KEY_F15       | KEY_SHIFT | KEY_MOD1,
    KEY_F16       | KEY_SHIFT | KEY_MOD1,

    KEY_DOWN      | KEY_SHIFT | KEY_MOD1,
    KEY_UP        | KEY_SHIFT | KEY_MOD1,
    KEY_LEFT      | KEY_SHIFT | KEY_MOD1,
    KEY_RIGHT     | KEY_SHIFT | KEY_MOD1,
    KEY_HOME      | KEY_SHIFT | KEY_MOD1,
    KEY_END       | KEY_SHIFT | KEY_MOD1,
    KEY_PAGEUP    | KEY_SHIFT | KEY_MOD1,
    KEY_PAGEDOWN  | KEY_SHIFT | KEY_MOD1,
    KEY_RETURN    | KEY_SHIFT | KEY_MOD1,
    KEY_SPACE     | KEY_SHIFT | KEY_MOD1,
    KEY_BACKSPACE | KEY_SHIFT | KEY_MOD1,
    KEY_INSERT    | KEY_SHIFT | KEY_MOD1,
    KEY_DELETE    | KEY_SHIFT | KEY_MOD1,

    KEY_0         | KEY_MOD2 ,
    KEY_1         | KEY_MOD2 ,
    KEY_2         | KEY_MOD2 ,
    KEY_3         | KEY_MOD2 ,
    KEY_4         | KEY_MOD2 ,
    KEY_5         | KEY_MOD2 ,
    KEY_6         | KEY_MOD2 ,
    KEY_7         | KEY_MOD2 ,
    KEY_8         | KEY_MOD2 ,
    KEY_9         | KEY_MOD2 ,
    KEY_A         | KEY_MOD2 ,
    KEY_B         | KEY_MOD2 ,
    KEY_C         | KEY_MOD2 ,
    KEY_D         | KEY_MOD2 ,
    KEY_E         | KEY_MOD2 ,
    KEY_F         | KEY_MOD2 ,
    KEY_G         | KEY_MOD2 ,
    KEY_H         | KEY_MOD2 ,
    KEY_I         | KEY_MOD2 ,
    KEY_J         | KEY_MOD2 ,
    KEY_K         | KEY_MOD2 ,
    KEY_L         | KEY_MOD2 ,
    KEY_M         | KEY_MOD2 ,
    KEY_N         | KEY_MOD2 ,
    KEY_O         | KEY_MOD2 ,
    KEY_P         | KEY_MOD2 ,
    KEY_Q         | KEY_MOD2 ,
    KEY_R         | KEY_MOD2 ,
    KEY_S         | KEY_MOD2 ,
    KEY_T         | KEY_MOD2 ,
    KEY_U         | KEY_MOD2 ,
    KEY_V         | KEY_MOD2 ,
    KEY_W         | KEY_MOD2 ,
    KEY_X         | KEY_MOD2 ,
    KEY_Y         | KEY_MOD2 ,
    KEY_Z         | KEY_MOD2 ,
    KEY_SEMICOLON    | KEY_MOD2 ,
    KEY_BRACKETLEFT  | KEY_MOD2 ,
    KEY_BRACKETRIGHT | KEY_MOD2,
    KEY_POINT    | KEY_MOD2 ,

    KEY_F1        | KEY_MOD2 ,
    KEY_F2        | KEY_MOD2 ,
    KEY_F3        | KEY_MOD2 ,
    KEY_F4        | KEY_MOD2 ,
    KEY_F5        | KEY_MOD2 ,
    KEY_F6        | KEY_MOD2 ,
    KEY_F7        | KEY_MOD2 ,
    KEY_F8        | KEY_MOD2 ,
    KEY_F9        | KEY_MOD2 ,
    KEY_F10       | KEY_MOD2 ,
    KEY_F11       | KEY_MOD2 ,
    KEY_F12       | KEY_MOD2 ,
    KEY_F13       | KEY_MOD2 ,
    KEY_F14       | KEY_MOD2 ,
    KEY_F15       | KEY_MOD2 ,
    KEY_F16       | KEY_MOD2 ,

    KEY_DOWN      | KEY_MOD2 ,
    KEY_UP        | KEY_MOD2 ,
    KEY_LEFT      | KEY_MOD2 ,
    KEY_RIGHT     | KEY_MOD2 ,
    KEY_HOME      | KEY_MOD2 ,
    KEY_END       | KEY_MOD2 ,
    KEY_PAGEUP    | KEY_MOD2 ,
    KEY_PAGEDOWN  | KEY_MOD2 ,
    KEY_RETURN    | KEY_MOD2 ,
    KEY_SPACE     | KEY_MOD2 ,
    KEY_BACKSPACE | KEY_MOD2 ,
    KEY_INSERT    | KEY_MOD2 ,
    KEY_DELETE    | KEY_MOD2 ,

    KEY_0         | KEY_SHIFT | KEY_MOD2,
    KEY_1         | KEY_SHIFT | KEY_MOD2,
    KEY_2         | KEY_SHIFT | KEY_MOD2,
    KEY_3         | KEY_SHIFT | KEY_MOD2,
    KEY_4         | KEY_SHIFT | KEY_MOD2,
    KEY_5         | KEY_SHIFT | KEY_MOD2,
    KEY_6         | KEY_SHIFT | KEY_MOD2,
    KEY_7         | KEY_SHIFT | KEY_MOD2,
    KEY_8         | KEY_SHIFT | KEY_MOD2,
    KEY_9         | KEY_SHIFT | KEY_MOD2,
    KEY_A         | KEY_SHIFT | KEY_MOD2,
    KEY_B         | KEY_SHIFT | KEY_MOD2,
    KEY_C         | KEY_SHIFT | KEY_MOD2,
    KEY_D         | KEY_SHIFT | KEY_MOD2,
    KEY_E         | KEY_SHIFT | KEY_MOD2,
    KEY_F         | KEY_SHIFT | KEY_MOD2,
    KEY_G         | KEY_SHIFT | KEY_MOD2,
    KEY_H         | KEY_SHIFT | KEY_MOD2,
    KEY_I         | KEY_SHIFT | KEY_MOD2,
    KEY_J         | KEY_SHIFT | KEY_MOD2,
    KEY_K         | KEY_SHIFT | KEY_MOD2,
    KEY_L         | KEY_SHIFT | KEY_MOD2,
    KEY_M         | KEY_SHIFT | KEY_MOD2,
    KEY_N         | KEY_SHIFT | KEY_MOD2,
    KEY_O         | KEY_SHIFT | KEY_MOD2,
    KEY_P         | KEY_SHIFT | KEY_MOD2,
    KEY_Q         | KEY_SHIFT | KEY_MOD2,
    KEY_R         | KEY_SHIFT | KEY_MOD2,
    KEY_S         | KEY_SHIFT | KEY_MOD2,
    KEY_T         | KEY_SHIFT | KEY_MOD2,
    KEY_U         | KEY_SHIFT | KEY_MOD2,
    KEY_V         | KEY_SHIFT | KEY_MOD2,
    KEY_W         | KEY_SHIFT | KEY_MOD2,
    KEY_X         | KEY_SHIFT | KEY_MOD2,
    KEY_Y         | KEY_SHIFT | KEY_MOD2,
    KEY_Z         | KEY_SHIFT | KEY_MOD2,
    KEY_SEMICOLON    | KEY_SHIFT | KEY_MOD2 ,
    KEY_BRACKETLEFT  | KEY_SHIFT | KEY_MOD2 ,
    KEY_BRACKETRIGHT | KEY_SHIFT | KEY_MOD2,
    KEY_POINT    | KEY_SHIFT | KEY_MOD2,

    KEY_F1        | KEY_SHIFT | KEY_MOD2,
    KEY_F2        | KEY_SHIFT | KEY_MOD2,
    KEY_F3        | KEY_SHIFT | KEY_MOD2,
    KEY_F4        | KEY_SHIFT | KEY_MOD2,
    KEY_F5        | KEY_SHIFT | KEY_MOD2,
    KEY_F6        | KEY_SHIFT | KEY_MOD2,
    KEY_F7        | KEY_SHIFT | KEY_MOD2,
    KEY_F8        | KEY_SHIFT | KEY_MOD2,
    KEY_F9        | KEY_SHIFT | KEY_MOD2,
    KEY_F10       | KEY_SHIFT | KEY_MOD2,
    KEY_F11       | KEY_SHIFT | KEY_MOD2,
    KEY_F12       | KEY_SHIFT | KEY_MOD2,
    KEY_F13       | KEY_SHIFT | KEY_MOD2,
    KEY_F14       | KEY_SHIFT | KEY_MOD2,
    KEY_F15       | KEY_SHIFT | KEY_MOD2,
    KEY_F16       | KEY_SHIFT | KEY_MOD2,

    KEY_DOWN      | KEY_SHIFT | KEY_MOD2,
    KEY_UP        | KEY_SHIFT | KEY_MOD2,
    KEY_LEFT      | KEY_SHIFT | KEY_MOD2,
    KEY_RIGHT     | KEY_SHIFT | KEY_MOD2,
    KEY_HOME      | KEY_SHIFT | KEY_MOD2,
    KEY_END       | KEY_SHIFT | KEY_MOD2,
    KEY_PAGEUP    | KEY_SHIFT | KEY_MOD2,
    KEY_PAGEDOWN  | KEY_SHIFT | KEY_MOD2,
    KEY_RETURN    | KEY_SHIFT | KEY_MOD2,
    KEY_SPACE     | KEY_SHIFT | KEY_MOD2,
    KEY_BACKSPACE | KEY_SHIFT | KEY_MOD2,
    KEY_INSERT    | KEY_SHIFT | KEY_MOD2,
    KEY_DELETE    | KEY_SHIFT | KEY_MOD2,

    KEY_0         | KEY_MOD1 | KEY_MOD2 ,
    KEY_1         | KEY_MOD1 | KEY_MOD2 ,
    KEY_2         | KEY_MOD1 | KEY_MOD2 ,
    KEY_3         | KEY_MOD1 | KEY_MOD2 ,
    KEY_4         | KEY_MOD1 | KEY_MOD2 ,
    KEY_5         | KEY_MOD1 | KEY_MOD2 ,
    KEY_6         | KEY_MOD1 | KEY_MOD2 ,
    KEY_7         | KEY_MOD1 | KEY_MOD2 ,
    KEY_8         | KEY_MOD1 | KEY_MOD2 ,
    KEY_9         | KEY_MOD1 | KEY_MOD2 ,
    KEY_A         | KEY_MOD1 | KEY_MOD2 ,
    KEY_B         | KEY_MOD1 | KEY_MOD2 ,
    KEY_C         | KEY_MOD1 | KEY_MOD2 ,
    KEY_D         | KEY_MOD1 | KEY_MOD2 ,
    KEY_E         | KEY_MOD1 | KEY_MOD2 ,
    KEY_F         | KEY_MOD1 | KEY_MOD2 ,
    KEY_G         | KEY_MOD1 | KEY_MOD2 ,
    KEY_H         | KEY_MOD1 | KEY_MOD2 ,
    KEY_I         | KEY_MOD1 | KEY_MOD2 ,
    KEY_J         | KEY_MOD1 | KEY_MOD2 ,
    KEY_K         | KEY_MOD1 | KEY_MOD2 ,
    KEY_L         | KEY_MOD1 | KEY_MOD2 ,
    KEY_M         | KEY_MOD1 | KEY_MOD2 ,
    KEY_N         | KEY_MOD1 | KEY_MOD2 ,
    KEY_O         | KEY_MOD1 | KEY_MOD2 ,
    KEY_P         | KEY_MOD1 | KEY_MOD2 ,
    KEY_Q         | KEY_MOD1 | KEY_MOD2 ,
    KEY_R         | KEY_MOD1 | KEY_MOD2 ,
    KEY_S         | KEY_MOD1 | KEY_MOD2 ,
    KEY_T         | KEY_MOD1 | KEY_MOD2 ,
    KEY_U         | KEY_MOD1 | KEY_MOD2 ,
    KEY_V         | KEY_MOD1 | KEY_MOD2 ,
    KEY_W         | KEY_MOD1 | KEY_MOD2 ,
    KEY_X         | KEY_MOD1 | KEY_MOD2 ,
    KEY_Y         | KEY_MOD1 | KEY_MOD2 ,
    KEY_Z         | KEY_MOD1 | KEY_MOD2 ,

    KEY_F1        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F2        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F3        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F4        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F5        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F6        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F7        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F8        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F9        | KEY_MOD1 | KEY_MOD2 ,
    KEY_F10       | KEY_MOD1 | KEY_MOD2 ,
    KEY_F11       | KEY_MOD1 | KEY_MOD2 ,
    KEY_F12       | KEY_MOD1 | KEY_MOD2 ,
    KEY_F13       | KEY_MOD1 | KEY_MOD2 ,
    KEY_F14       | KEY_MOD1 | KEY_MOD2 ,
    KEY_F15       | KEY_MOD1 | KEY_MOD2 ,
    KEY_F16       | KEY_MOD1 | KEY_MOD2 ,

    KEY_DOWN      | KEY_MOD1 | KEY_MOD2 ,
    KEY_UP        | KEY_MOD1 | KEY_MOD2 ,
    KEY_LEFT      | KEY_MOD1 | KEY_MOD2 ,
    KEY_RIGHT     | KEY_MOD1 | KEY_MOD2 ,
    KEY_HOME      | KEY_MOD1 | KEY_MOD2 ,
    KEY_END       | KEY_MOD1 | KEY_MOD2 ,
    KEY_PAGEUP    | KEY_MOD1 | KEY_MOD2 ,
    KEY_PAGEDOWN  | KEY_MOD1 | KEY_MOD2 ,
    KEY_RETURN    | KEY_MOD1 | KEY_MOD2 ,
    KEY_SPACE     | KEY_MOD1 | KEY_MOD2 ,
    KEY_BACKSPACE | KEY_MOD1 | KEY_MOD2 ,
    KEY_INSERT    | KEY_MOD1 | KEY_MOD2 ,
    KEY_DELETE    | KEY_MOD1 | KEY_MOD2 ,

    KEY_0         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_1         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_2         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_3         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_4         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_5         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_6         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_7         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_8         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_9         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_A         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_B         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_C         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_D         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_E         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_G         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_H         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_I         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_J         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_K         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_L         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_M         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_N         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_O         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_P         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_Q         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_R         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_S         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_T         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_U         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_V         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_W         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_X         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_Y         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_Z         | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_SEMICOLON    | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_BRACKETLEFT  | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_BRACKETRIGHT | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_POINT    | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,

    KEY_F1        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F2        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F3        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F4        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F5        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F6        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F7        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F8        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F9        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F10       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F11       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F12       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F13       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F14       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F15       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_F16       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,

    KEY_DOWN      | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_UP        | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_LEFT      | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_RIGHT     | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_HOME      | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_END       | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_PAGEUP    | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_PAGEDOWN  | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_RETURN    | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_SPACE     | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_BACKSPACE | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_INSERT    | KEY_SHIFT | KEY_MOD1 | KEY_MOD2,
    KEY_DELETE    | KEY_SHIFT | KEY_MOD1 | KEY_MOD2
};

static sal_uInt16 KEYCODE_ARRAY_SIZE = SAL_N_ELEMENTS(KEYCODE_ARRAY);

//-----------------------------------------------
// seems to be needed to layout the list box, which shows all
// assignable shortcuts
static long AccCfgTabs[] =
{
    2,  // Number of Tabs
    0,
    120 // Function
};

//-----------------------------------------------
class SfxAccCfgLBoxString_Impl : public SvLBoxString
{
    public:
    SfxAccCfgLBoxString_Impl(      SvLBoxEntry* pEntry,
                                   sal_uInt16       nFlags,
                             const String&      sText );

    virtual ~SfxAccCfgLBoxString_Impl();

    virtual void Paint(const Point&       aPos   ,
                             SvLBox&      rDevice,
                             sal_uInt16       nFlags ,
                             SvLBoxEntry* pEntry );
};

//-----------------------------------------------
SfxAccCfgLBoxString_Impl::SfxAccCfgLBoxString_Impl(      SvLBoxEntry* pEntry,
                                                         sal_uInt16       nFlags,
                                                   const String&      sText )
        : SvLBoxString(pEntry, nFlags, sText)
{
}

//-----------------------------------------------
SfxAccCfgLBoxString_Impl::~SfxAccCfgLBoxString_Impl()
{
}

//-----------------------------------------------
void SfxAccCfgLBoxString_Impl::Paint(const Point&       aPos   ,
                                           SvLBox&      rDevice,
                                           sal_uInt16       /*nFlags*/,
                                           SvLBoxEntry* pEntry )
{

    if (!pEntry)
        return;

    TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
    if (!pUserData)
        return;

    if (pUserData->m_bIsConfigurable)
        rDevice.DrawText(aPos, GetText());
    else
        rDevice.DrawCtrlText(aPos, GetText(), 0, STRING_LEN, TEXT_DRAW_DISABLE);

}

//-----------------------------------------------
void SfxAccCfgTabListBox_Impl::InitEntry(      SvLBoxEntry* pEntry ,
                                         const XubString&   sText  ,
                                         const Image&       aImage1,
                                         const Image&       aImage2,
                                               SvLBoxButtonKind eButtonKind)
{
    SvTabListBox::InitEntry(pEntry, sText, aImage1, aImage2, eButtonKind);
}

//-----------------------------------------------
/** select the entry, which match the current key input ... excepting
    keys, which are used for the dialog itself.
  */
void SfxAccCfgTabListBox_Impl::KeyInput(const KeyEvent& aKey)
{
    KeyCode aCode1 = aKey.GetKeyCode();
    sal_uInt16  nCode1 = aCode1.GetCode();
    sal_uInt16  nMod1  = aCode1.GetModifier();

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
        SvLBoxEntry* pEntry = First();
        while (pEntry)
        {
            TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
            if (pUserData)
            {
                sal_uInt16 nCode2 = pUserData->m_aKey.GetCode();
                sal_uInt16 nMod2  = pUserData->m_aKey.GetModifier();
                if (
                    (nCode1 == nCode2) &&
                    (nMod1  == nMod2 )
                   )
                {
                    Select     (pEntry);
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

//-----------------------------------------------
SfxAcceleratorConfigPage::SfxAcceleratorConfigPage( Window* pParent, const SfxItemSet& aSet )
    : SfxTabPage              (pParent, CUI_RES(RID_SVXPAGE_KEYBOARD), aSet)
    , m_pMacroInfoItem        ()
    , m_pStringItem        ()
    , m_pFontItem        ()
    , m_pFileDlg              (NULL)
    , aEntriesBox             (this   , this, CUI_RES(BOX_ACC_ENTRIES   ))
    , aKeyboardGroup          (this   , CUI_RES(GRP_ACC_KEYBOARD        ))
    , aOfficeButton           (this   , CUI_RES(RB_OFFICE               ))
    , aModuleButton           (this   , CUI_RES(RB_MODULE               ))
    , aChangeButton           (this   , CUI_RES(BTN_ACC_CHANGE          ))
    , aRemoveButton           (this   , CUI_RES(BTN_ACC_REMOVE          ))
    , aGroupText              (this   , CUI_RES(TXT_ACC_GROUP           ))
    , pGroupLBox(new SfxConfigGroupListBox_Impl( this, CUI_RES(BOX_ACC_GROUP), SFX_SLOT_ACCELCONFIG ))
    , aFunctionText           (this   , CUI_RES(TXT_ACC_FUNCTION        ))
    , pFunctionBox(new SfxConfigFunctionListBox_Impl( this, CUI_RES( BOX_ACC_FUNCTION )))
    , aKeyText                (this   , CUI_RES(TXT_ACC_KEY             ))
    , aKeyBox                 (this   , CUI_RES(BOX_ACC_KEY             ))
    , aFunctionsGroup         (this   , CUI_RES(GRP_ACC_FUNCTIONS       ))
    , aLoadButton             (this   , CUI_RES(BTN_LOAD                ))
    , aSaveButton             (this   , CUI_RES(BTN_SAVE                ))
    , aResetButton            (this   , CUI_RES(BTN_RESET               ))
    , aLoadAccelConfigStr             ( CUI_RES( STR_LOADACCELCONFIG ) )
    , aSaveAccelConfigStr             ( CUI_RES( STR_SAVEACCELCONFIG ) )
    , aFilterCfgStr                   ( CUI_RES( STR_FILTERNAME_CFG ) )
    , m_bStylesInfoInitialized(sal_False)
    , m_xGlobal               ()
    , m_xModule               ()
    , m_xAct                  ()
{
    FreeResource();

    aFilterAllStr = String( SfxResId( STR_SFX_FILTERNAME_ALL ) );

// install handler functions
    aChangeButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, ChangeHdl ));
    aRemoveButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RemoveHdl ));
    aEntriesBox.SetSelectHdl ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    pGroupLBox->SetSelectHdl  ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    pFunctionBox->SetSelectHdl( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    aKeyBox.SetSelectHdl     ( LINK( this, SfxAcceleratorConfigPage, SelectHdl ));
    aLoadButton.SetClickHdl  ( LINK( this, SfxAcceleratorConfigPage, Load      ));
    aSaveButton.SetClickHdl  ( LINK( this, SfxAcceleratorConfigPage, Save      ));
    aResetButton.SetClickHdl ( LINK( this, SfxAcceleratorConfigPage, Default   ));
    aOfficeButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));
    aModuleButton.SetClickHdl( LINK( this, SfxAcceleratorConfigPage, RadioHdl  ));

    // initialize Entriesbox
    aEntriesBox.SetStyle(aEntriesBox.GetStyle()|WB_HSCROLL|WB_CLIPCHILDREN);
    aEntriesBox.SetSelectionMode(SINGLE_SELECTION);
    aEntriesBox.SetTabs(&AccCfgTabs[0], MAP_APPFONT);
    aEntriesBox.Resize(); // OS: Hack for right selection
    aEntriesBox.SetSpaceBetweenEntries(0);
    aEntriesBox.SetDragDropMode(0);

    // detect max keyname width
    long nMaxWidth  = 0;
    for ( sal_uInt16 i = 0; i < KEYCODE_ARRAY_SIZE; ++i )
    {
        long nTmp = GetTextWidth( KeyCode( KEYCODE_ARRAY[i] ).GetName() );
        if ( nTmp > nMaxWidth )
            nMaxWidth = nTmp;
    }
    // recalc second tab
    long nNewTab = PixelToLogic( Size( nMaxWidth, 0 ), MAP_APPFONT ).Width();
    nNewTab = nNewTab + 5; // additional space
    aEntriesBox.SetTab( 1, nNewTab );

    // initialize GroupBox
    pGroupLBox->SetFunctionListBox(pFunctionBox);

    // initialize KeyBox
    aKeyBox.SetStyle(aKeyBox.GetStyle()|WB_CLIPCHILDREN|WB_HSCROLL|WB_SORT);
}

//-----------------------------------------------
SfxAcceleratorConfigPage::~SfxAcceleratorConfigPage()
{
    // free memory - remove all dynamic user data
    SvLBoxEntry* pEntry = aEntriesBox.First();
    while (pEntry)
    {
        TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
        if (pUserData)
            delete pUserData;
        pEntry = aEntriesBox.Next(pEntry);
    }

    pEntry = aKeyBox.First();
    while (pEntry)
    {
        TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
        if (pUserData)
            delete pUserData;
        pEntry = aKeyBox.Next(pEntry);
    }

    aEntriesBox.Clear();
    aKeyBox.Clear();

    delete m_pFileDlg;
    delete pGroupLBox;
    delete pFunctionBox;
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::InitAccCfg()
{
    // already initialized ?
    if (m_xSMGR.is())
        return; // yes -> do nothing

    try
    {
        // no - initialize this instance
        m_xSMGR = ::comphelper::getProcessServiceFactory();

        m_xUICmdDescription = css::uno::Reference< css::container::XNameAccess >(m_xSMGR->createInstance(SERVICE_UICMDDESCRIPTION), css::uno::UNO_QUERY_THROW);

        // get the current active frame, which should be our "parent"
        // for this session
        m_xFrame = GetFrame();
        if ( !m_xFrame.is() )
        {
            css::uno::Reference< css::frame::XFramesSupplier > xDesktop(m_xSMGR->createInstance(SERVICE_DESKTOP), css::uno::UNO_QUERY_THROW);
            m_xFrame = xDesktop->getActiveFrame();
        }

        // identify module
        css::uno::Reference< css::frame::XModuleManager > xModuleManager    (m_xSMGR->createInstance(SERVICE_MODULEMANAGER), css::uno::UNO_QUERY_THROW);
        css::uno::Reference< css::container::XNameAccess > xModuleManagerCont(xModuleManager                                , css::uno::UNO_QUERY_THROW);
        m_sModuleLongName = xModuleManager->identify(m_xFrame);
        ::comphelper::SequenceAsHashMap lModuleProps(xModuleManagerCont->getByName(m_sModuleLongName));
        m_sModuleShortName = lModuleProps.getUnpackedValueOrDefault(MODULEPROP_SHORTNAME, ::rtl::OUString());
        m_sModuleUIName    = lModuleProps.getUnpackedValueOrDefault(MODULEPROP_UINAME   , ::rtl::OUString());

        // get global accelerator configuration
        m_xGlobal = css::uno::Reference< css::ui::XAcceleratorConfiguration >(m_xSMGR->createInstance(SERVICE_GLOBALACCCFG), css::uno::UNO_QUERY_THROW);

        // get module accelerator configuration

        css::uno::Reference< css::ui::XModuleUIConfigurationManagerSupplier > xModuleCfgSupplier(css::ui::ModuleUIConfigurationManagerSupplier::create(comphelper::ComponentContext(m_xSMGR).getUNOContext()));
        css::uno::Reference< css::ui::XUIConfigurationManager > xUICfgManager = xModuleCfgSupplier->getUIConfigurationManager(m_sModuleLongName);
        m_xModule = css::uno::Reference< css::ui::XAcceleratorConfiguration >(xUICfgManager->getShortCutManager(), css::uno::UNO_QUERY_THROW);
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        { m_xSMGR.clear(); }
}

//-----------------------------------------------
/** Initialize text columns with own class to enable custom painting
    This is needed as we have to paint disabled entries by ourself. No support for that in the
    original SvTabListBox!
  */
void SfxAcceleratorConfigPage::CreateCustomItems(      SvLBoxEntry* pEntry,
                                                 const String&      sCol1 ,
                                                 const String&      sCol2 )
{
    SfxAccCfgLBoxString_Impl* pStringItem = new SfxAccCfgLBoxString_Impl(pEntry, 0, sCol1);
    pEntry->ReplaceItem(pStringItem, 1);

    pStringItem = new SfxAccCfgLBoxString_Impl(pEntry, 0, sCol2);
    pEntry->ReplaceItem(pStringItem, 2);
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::Init(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    if (!m_bStylesInfoInitialized)
    {
        css::uno::Reference< css::frame::XController > xController;
        css::uno::Reference< css::frame::XModel > xModel;
        if (m_xFrame.is())
            xController = m_xFrame->getController();
        if (xController.is())
            xModel = xController->getModel();

        m_aStylesInfo.setModel(xModel);
        pFunctionBox->SetStylesInfo(&m_aStylesInfo);
        pGroupLBox->SetStylesInfo(&m_aStylesInfo);
        m_bStylesInfoInitialized = sal_True;
    }

    // Insert all editable accelerators into list box. It is possible
    // that some accelerators are not mapped on the current system/keyboard
    // but we don't want to lose these mappings.
    sal_uInt16 c1       = KEYCODE_ARRAY_SIZE;
    sal_uInt16 i1       = 0;
    sal_uInt16 nListPos = 0;
    for (i1=0; i1<c1; ++i1)
    {
        KeyCode aKey = KEYCODE_ARRAY[i1];
        String  sKey = aKey.GetName();
        if (!sKey.Len())
            continue;
        TAccInfo*    pEntry   = new TAccInfo(i1, nListPos, aKey);
        SvLBoxEntry* pLBEntry = aEntriesBox.InsertEntryToColumn(sKey, 0L, LIST_APPEND, 0xFFFF);
        pLBEntry->SetUserData(pEntry);
    }

    // Assign all commands to its shortcuts - reading the accelerator config.
    css::uno::Sequence< css::awt::KeyEvent > lKeys = xAccMgr->getAllKeyEvents();
    sal_Int32                                c2    = lKeys.getLength();
    sal_Int32                                i2    = 0;
    sal_uInt16                                   nCol  = aEntriesBox.TabCount()-1;

    for (i2=0; i2<c2; ++i2)
    {
        const css::awt::KeyEvent& aAWTKey  = lKeys[i2];
              ::rtl::OUString     sCommand = xAccMgr->getCommandByKeyEvent(aAWTKey);
              String              sLabel   = GetLabel4Command(sCommand);
              KeyCode             aKeyCode = ::svt::AcceleratorExecute::st_AWTKey2VCLKey(aAWTKey);
              sal_uInt16              nPos     = MapKeyCodeToPos(aKeyCode);

        if (nPos == LISTBOX_ENTRY_NOTFOUND)
            continue;

        aEntriesBox.SetEntryText(sLabel, nPos, nCol);

        SvLBoxEntry* pLBEntry = aEntriesBox.GetEntry(0, nPos);
        TAccInfo*    pEntry   = (TAccInfo*)pLBEntry->GetUserData();

        pEntry->m_bIsConfigurable = sal_True;
        pEntry->m_sCommand        = sCommand;
        CreateCustomItems(pLBEntry, aEntriesBox.GetEntryText(pLBEntry, 0), sLabel);
    }

    // Map the VCL hardcoded key codes and mark them as not changeable
    sal_uLong c3 = Application::GetReservedKeyCodeCount();
    sal_uLong i3 = 0;
    for (i3=0; i3<c3; ++i3)
    {
        const KeyCode* pKeyCode = Application::GetReservedKeyCode(i3);
              sal_uInt16   nPos     = MapKeyCodeToPos(*pKeyCode);

        if (nPos == LISTBOX_ENTRY_NOTFOUND)
            continue;

        // Hardcoded function mapped so no ID possible and mark entry as not changeable
        SvLBoxEntry* pLBEntry = aEntriesBox.GetEntry(0, nPos);
        TAccInfo*    pEntry   = (TAccInfo*)pLBEntry->GetUserData();

        pEntry->m_bIsConfigurable = sal_False;
        CreateCustomItems(pLBEntry, aEntriesBox.GetEntryText(pLBEntry, 0), String());
    }
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::Apply(const css::uno::Reference< css::ui::XAcceleratorConfiguration >& xAccMgr)
{
    if (!xAccMgr.is())
        return;

    // Go through the list from the bottom to the top ...
    // because logical accelerator must be preferred instead of
    // physical ones!
    SvLBoxEntry* pEntry = aEntriesBox.First();
    while (pEntry)
    {
        TAccInfo*          pUserData = (TAccInfo*)pEntry->GetUserData();
        ::rtl::OUString    sCommand  ;
        css::awt::KeyEvent aAWTKey   ;

        if (pUserData)
        {
            sCommand = pUserData->m_sCommand;
            aAWTKey  = ::svt::AcceleratorExecute::st_VCLKey2AWTKey(pUserData->m_aKey);
        }

        try
        {
            if (!sCommand.isEmpty())
                xAccMgr->setKeyEvent(aAWTKey, sCommand);
            else
                xAccMgr->removeKeyEvent(aAWTKey);
        }
        catch(const css::uno::RuntimeException&)
            { throw; }
        catch(const css::uno::Exception&)
            {}

        pEntry = aEntriesBox.Next(pEntry);
    }
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::ResetConfig()
{
    aEntriesBox.Clear();
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Load)
{
    // ask for filename, where we should load the new config data from
    StartFileDialog( 0, aLoadAccelConfigStr );
    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Save)
{
    StartFileDialog( WB_SAVEAS, aSaveAccelConfigStr );
    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, Default)
{
    css::uno::Reference< css::form::XReset > xReset(m_xAct, css::uno::UNO_QUERY);
    if (xReset.is())
        xReset->reset();

    aEntriesBox.SetUpdateMode(sal_False);
    ResetConfig();
    Init(m_xAct);
    aEntriesBox.SetUpdateMode(sal_True);
    aEntriesBox.Invalidate();
    aEntriesBox.Select(aEntriesBox.GetEntry(0, 0));

    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, ChangeHdl)
{
    sal_uInt16    nPos        = (sal_uInt16) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    TAccInfo* pEntry      = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
    String    sNewCommand = pFunctionBox->GetCurCommand();
    String    sLabel      = pFunctionBox->GetCurLabel();
    if (!sLabel.Len())
        sLabel = GetLabel4Command(sNewCommand);

    pEntry->m_sCommand = sNewCommand;
    sal_uInt16 nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText(sLabel, nPos, nCol);

    ((Link &) pFunctionBox->GetSelectHdl()).Call( pFunctionBox );
    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, RemoveHdl)
{
    // get selected entry
    sal_uInt16    nPos   = (sal_uInt16) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
    TAccInfo* pEntry = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();

    // remove function name from selected entry
    sal_uInt16 nCol = aEntriesBox.TabCount() - 1;
    aEntriesBox.SetEntryText( String(), nPos, nCol );
    pEntry->m_sCommand = ::rtl::OUString();

    ((Link &) pFunctionBox->GetSelectHdl()).Call( pFunctionBox );
    return 0;
}

//-----------------------------------------------
IMPL_LINK( SfxAcceleratorConfigPage, SelectHdl, Control*, pListBox )
{
    // disable help
    Help::ShowBalloon( this, Point(), String() );
    if ( pListBox == &aEntriesBox )
    {
        sal_uInt16          nPos                = (sal_uInt16) aEntriesBox.GetModel()->GetRelPos( aEntriesBox.FirstSelected() );
        TAccInfo*       pEntry              = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
        ::rtl::OUString sPossibleNewCommand = pFunctionBox->GetCurCommand();

        aRemoveButton.Enable( sal_False );
        aChangeButton.Enable( sal_False );

        if (pEntry->m_bIsConfigurable)
        {
            if (pEntry->isConfigured())
                aRemoveButton.Enable( sal_True );
            aChangeButton.Enable( pEntry->m_sCommand != sPossibleNewCommand );
        }
    }
    else if ( pListBox == pGroupLBox )
    {
        pGroupLBox->GroupSelected();
        if ( !pFunctionBox->FirstSelected() )
            aChangeButton.Enable( sal_False );
    }
    else if ( pListBox == pFunctionBox )
    {
        aRemoveButton.Enable( sal_False );
        aChangeButton.Enable( sal_False );

        // #i36994 First selected can return zero!
        SvLBoxEntry*    pLBEntry = aEntriesBox.FirstSelected();
        if ( pLBEntry != 0 )
        {
            sal_uInt16          nPos                = (sal_uInt16) aEntriesBox.GetModel()->GetRelPos( pLBEntry );
            TAccInfo*       pEntry              = (TAccInfo*)aEntriesBox.GetEntry(0, nPos)->GetUserData();
            ::rtl::OUString sPossibleNewCommand = pFunctionBox->GetCurCommand();

            if (pEntry->m_bIsConfigurable)
            {
                if (pEntry->isConfigured())
                    aRemoveButton.Enable( sal_True );
                aChangeButton.Enable( pEntry->m_sCommand != sPossibleNewCommand );
            }

            // update key box
            aKeyBox.Clear();
            SvLBoxEntry* pIt = aEntriesBox.First();
            while ( pIt )
            {
                TAccInfo* pUserData = (TAccInfo*)pIt->GetUserData();
                if ( pUserData && pUserData->m_sCommand == sPossibleNewCommand )
                {
                    TAccInfo*    pU1 = new TAccInfo(-1, -1, pUserData->m_aKey);
                    SvLBoxEntry* pE1 = aKeyBox.InsertEntry( pUserData->m_aKey.GetName(), 0L, sal_True, LIST_APPEND );
                    pE1->SetUserData(pU1);
                    pE1->EnableChildrenOnDemand( sal_False );
                }
                pIt = aEntriesBox.Next(pIt);
            }
        }
    }
    else
    {
        // goto selected "key" entry of the key box
        SvLBoxEntry* pE2 = 0;
        TAccInfo*    pU2 = 0;
        sal_uInt16       nP2 = LISTBOX_ENTRY_NOTFOUND;
        SvLBoxEntry* pE3 = 0;

        pE2 = aKeyBox.FirstSelected();
        if (pE2)
            pU2 = (TAccInfo*)pE2->GetUserData();
        if (pU2)
            nP2 = MapKeyCodeToPos(pU2->m_aKey);
        if (nP2 != LISTBOX_ENTRY_NOTFOUND)
            pE3 = aEntriesBox.GetEntry( 0, nP2 );
        if (pE3)
        {
            aEntriesBox.Select( pE3 );
            aEntriesBox.MakeVisible( pE3 );
        }
    }

    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, RadioHdl)
{
    css::uno::Reference< css::ui::XAcceleratorConfiguration > xOld = m_xAct;

    if (aOfficeButton.IsChecked())
        m_xAct = m_xGlobal;
    else if (aModuleButton.IsChecked())
        m_xAct = m_xModule;

    // nothing changed? => do nothing!
    if ( m_xAct.is() && ( xOld == m_xAct ) )
        return 0;

    aEntriesBox.SetUpdateMode( sal_False );
    ResetConfig();
    Init(m_xAct);
    aEntriesBox.SetUpdateMode( sal_True );
    aEntriesBox.Invalidate();

     pGroupLBox->Init(m_xSMGR, m_xFrame, m_sModuleLongName);

    // pb: #133213# do not select NULL entries
    SvLBoxEntry* pEntry = aEntriesBox.GetEntry( 0, 0 );
    if ( pEntry )
        aEntriesBox.Select( pEntry );
    pEntry = pGroupLBox->GetEntry( 0, 0 );
    if ( pEntry )
        pGroupLBox->Select( pEntry );

    ((Link &) pFunctionBox->GetSelectHdl()).Call( pFunctionBox );
    return 1L;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, LoadHdl)
{
    DBG_ASSERT( m_pFileDlg, "SfxInternetPage::DialogClosedHdl(): no file dialog" );

    ::rtl::OUString sCfgName;
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
        sCfgName = m_pFileDlg->GetPath();

    if ( sCfgName.isEmpty() )
        return 0;

    GetTabDialog()->EnterWait();

    css::uno::Reference< css::frame::XModel >                xDoc        ;
    css::uno::Reference< css::ui::XUIConfigurationManager > xCfgMgr     ;
    css::uno::Reference< css::embed::XStorage >              xRootStorage; // we must hold the root storage alive, if xCfgMgr is used!

    try
    {
        // first check if URL points to a document already loaded
        xDoc = SearchForAlreadyLoadedDoc(sCfgName);
        if (xDoc.is())
        {
            // Get ui config manager. There should always be one at the model.
            css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xCfgSupplier(xDoc, css::uno::UNO_QUERY_THROW);
            xCfgMgr = xCfgSupplier->getUIConfigurationManager();
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
            // dont forget to release the storage afterwards!
            css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory(m_xSMGR->createInstance(SERVICE_STORAGEFACTORY), css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::uno::Any >                     lArgs(2);
            lArgs[0] <<= sCfgName;
            lArgs[1] <<= css::embed::ElementModes::READ;

            xRootStorage = css::uno::Reference< css::embed::XStorage >(xStorageFactory->createInstanceWithArguments(lArgs), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::embed::XStorage > xUIConfig = xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, css::embed::ElementModes::READ);
            if (xUIConfig.is())
            {
                xCfgMgr = css::uno::Reference< css::ui::XUIConfigurationManager >(m_xSMGR->createInstance(SERVICE_UICONFIGMGR), css::uno::UNO_QUERY_THROW);
                css::uno::Reference< css::ui::XUIConfigurationStorage > xCfgMgrStore(xCfgMgr, css::uno::UNO_QUERY_THROW);
                xCfgMgrStore->setStorage(xUIConfig);
            }
        }

        if (xCfgMgr.is())
        {
            // open the configuration and update our UI
            css::uno::Reference< css::ui::XAcceleratorConfiguration > xTempAccMgr(xCfgMgr->getShortCutManager(), css::uno::UNO_QUERY_THROW);

            aEntriesBox.SetUpdateMode(sal_False);
            ResetConfig();
            Init(xTempAccMgr);
            aEntriesBox.SetUpdateMode(sal_True);
            aEntriesBox.Invalidate();
            aEntriesBox.Select(aEntriesBox.GetEntry(0, 0));

        }

        // dont forget to close the new opened storage!
        // We are the owner of it.
        if (xRootStorage.is())
        {
            css::uno::Reference< css::lang::XComponent > xComponent;
            xComponent = css::uno::Reference< css::lang::XComponent >(xCfgMgr, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            xComponent = css::uno::Reference< css::lang::XComponent >(xRootStorage, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}

    GetTabDialog()->LeaveWait();

    return 0;
}

//-----------------------------------------------
IMPL_LINK_NOARG(SfxAcceleratorConfigPage, SaveHdl)
{
    DBG_ASSERT( m_pFileDlg, "SfxInternetPage::DialogClosedHdl(): no file dialog" );

    ::rtl::OUString sCfgName;
    if ( ERRCODE_NONE == m_pFileDlg->GetError() )
        sCfgName = m_pFileDlg->GetPath();

    if ( sCfgName.isEmpty() )
        return 0;

    GetTabDialog()->EnterWait();

    css::uno::Reference< css::frame::XModel >                xDoc        ;
    css::uno::Reference< css::ui::XUIConfigurationManager > xCfgMgr     ;
    css::uno::Reference< css::embed::XStorage >              xRootStorage;

    try
    {
        // first check if URL points to a document already loaded
        xDoc = SearchForAlreadyLoadedDoc(sCfgName);
        if (xDoc.is())
        {
            // get config manager, force creation if there was none before
            css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > xCfgSupplier(xDoc, css::uno::UNO_QUERY_THROW);
            xCfgMgr = xCfgSupplier->getUIConfigurationManager();
        }
        else
        {
            // URL doesn't point to a loaded document, try to access it as a single storage
            css::uno::Reference< css::lang::XSingleServiceFactory > xStorageFactory(m_xSMGR->createInstance(SERVICE_STORAGEFACTORY), css::uno::UNO_QUERY_THROW);
            css::uno::Sequence< css::uno::Any >                     lArgs(2);
            lArgs[0] <<= sCfgName;
            lArgs[1] <<= css::embed::ElementModes::WRITE;

            xRootStorage = css::uno::Reference< css::embed::XStorage >(
                                xStorageFactory->createInstanceWithArguments(lArgs),
                                css::uno::UNO_QUERY_THROW);

            css::uno::Reference< css::embed::XStorage > xUIConfig(
                                xRootStorage->openStorageElement(FOLDERNAME_UICONFIG, css::embed::ElementModes::WRITE),
                                css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::beans::XPropertySet > xUIConfigProps(
                                xUIConfig,
                                css::uno::UNO_QUERY_THROW);

            // set the correct media type if the storage was new created
            ::rtl::OUString sMediaType;
            xUIConfigProps->getPropertyValue(MEDIATYPE_PROPNAME) >>= sMediaType;
            if (sMediaType.isEmpty())
                xUIConfigProps->setPropertyValue(MEDIATYPE_PROPNAME, css::uno::makeAny(MEDIATYPE_UICONFIG));

            xCfgMgr = css::uno::Reference< css::ui::XUIConfigurationManager >(m_xSMGR->createInstance(SERVICE_UICONFIGMGR), css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::ui::XUIConfigurationStorage > xUICfgStore(xCfgMgr, css::uno::UNO_QUERY_THROW);
            xUICfgStore->setStorage(xUIConfig);
        }

        if (xCfgMgr.is())
        {
            // get the target configuration access and update with all shortcuts
            // which are set currently at the UI !
            // Dont copy the m_xAct content to it ... because m_xAct will be updated
            // from the UI on pressing the button "OK" only. And inbetween it's not up to date !
            css::uno::Reference< css::ui::XAcceleratorConfiguration > xTargetAccMgr(xCfgMgr->getShortCutManager(), css::uno::UNO_QUERY_THROW);
            Apply(xTargetAccMgr);

            // commit (order is important!)
            css::uno::Reference< css::ui::XUIConfigurationPersistence > xCommit1(xTargetAccMgr, css::uno::UNO_QUERY_THROW);
            css::uno::Reference< css::ui::XUIConfigurationPersistence > xCommit2(xCfgMgr      , css::uno::UNO_QUERY_THROW);
            xCommit1->store();
            xCommit2->store();

            if (xRootStorage.is())
            {
                // Commit root storage
                css::uno::Reference< css::embed::XTransactedObject > xCommit3(xRootStorage, css::uno::UNO_QUERY_THROW);
                xCommit3->commit();
            }
        }

        if (xRootStorage.is())
        {
            css::uno::Reference< css::lang::XComponent > xComponent;
            xComponent = css::uno::Reference< css::lang::XComponent >(xCfgMgr, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
            xComponent = css::uno::Reference< css::lang::XComponent >(xRootStorage, css::uno::UNO_QUERY);
            if (xComponent.is())
                xComponent->dispose();
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}

    GetTabDialog()->LeaveWait();

    return 0;
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::StartFileDialog( WinBits nBits, const String& rTitle )
{
    bool bSave = ( ( nBits & WB_SAVEAS ) == WB_SAVEAS );
    short nDialogType = bSave ? css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE
                              : css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE;
    if ( m_pFileDlg )
        delete m_pFileDlg;
    m_pFileDlg = new sfx2::FileDialogHelper( nDialogType, 0 );

    m_pFileDlg->SetTitle( rTitle );
    m_pFileDlg->AddFilter( aFilterAllStr, DEFINE_CONST_UNICODE( FILEDIALOG_FILTER_ALL ) );
    m_pFileDlg->AddFilter( aFilterCfgStr, DEFINE_CONST_UNICODE( "*.cfg" ) );

    Link aDlgClosedLink = bSave ? LINK( this, SfxAcceleratorConfigPage, SaveHdl )
                                : LINK( this, SfxAcceleratorConfigPage, LoadHdl );
    m_pFileDlg->StartExecuteModal( aDlgClosedLink );
}

//-----------------------------------------------
sal_Bool SfxAcceleratorConfigPage::FillItemSet( SfxItemSet& )
{
    Apply(m_xAct);
    try
    {
        m_xAct->store();
    }
    catch(const css::uno::RuntimeException&)
        { throw;  }
    catch(const css::uno::Exception&)
        { return sal_False; }

    return sal_True;
}

//-----------------------------------------------
void SfxAcceleratorConfigPage::Reset( const SfxItemSet& rSet )
{
    // open accelerator configs
    // Note: It initialize some other members too, which are needed here ...
    // e.g. m_sModuleUIName!
    InitAccCfg();

    // change te description of the radio button, which switch to the module
    // dependend accelerator configuration
    String sButtonText = aModuleButton.GetText();
    sButtonText.SearchAndReplace(rtl::OUString("$(MODULE)"), m_sModuleUIName);
    aModuleButton.SetText(sButtonText);

    if (m_xModule.is())
        aModuleButton.Check();
    else
    {
        aModuleButton.Hide();
        aOfficeButton.Check();
    }

    RadioHdl(0);

    const SfxPoolItem* pMacroItem=0;
    if( SFX_ITEM_SET == rSet.GetItemState( SID_MACROINFO, sal_True, &pMacroItem ) )
    {
        m_pMacroInfoItem = PTR_CAST( SfxMacroInfoItem, pMacroItem );
        pGroupLBox->SelectMacro( m_pMacroInfoItem );
    }
    else
    {
        const SfxPoolItem* pStringItem=0;
        if( SFX_ITEM_SET == rSet.GetItemState( SID_CHARMAP, sal_True, &pStringItem ) )
            m_pStringItem = PTR_CAST( SfxStringItem, pStringItem );

        const SfxPoolItem* pFontItem=0;
        if( SFX_ITEM_SET == rSet.GetItemState( SID_ATTR_SPECIALCHAR, sal_True, &pFontItem ) )
            m_pFontItem = PTR_CAST( SfxStringItem, pFontItem );
    }
}

//-----------------------------------------------
sal_uInt16 SfxAcceleratorConfigPage::MapKeyCodeToPos(const KeyCode& aKey) const
{
    sal_uInt16       nCode1 = aKey.GetCode()+aKey.GetModifier();
    SvLBoxEntry* pEntry = aEntriesBox.First();
    sal_uInt16       i      = 0;

    while (pEntry)
    {
        TAccInfo* pUserData = (TAccInfo*)pEntry->GetUserData();
        if (pUserData)
        {
            sal_uInt16 nCode2 = pUserData->m_aKey.GetCode()+pUserData->m_aKey.GetModifier();
            if (nCode1 == nCode2)
                return i;
        }
        pEntry = aEntriesBox.Next(pEntry);
        ++i;
    }

    return LISTBOX_ENTRY_NOTFOUND;
}

//-----------------------------------------------
String SfxAcceleratorConfigPage::GetLabel4Command(const String& sCommand)
{
    try
    {
        // check global command configuration first
        css::uno::Reference< css::container::XNameAccess > xModuleConf;
        m_xUICmdDescription->getByName(m_sModuleLongName) >>= xModuleConf;
        if (xModuleConf.is())
        {
            ::comphelper::SequenceAsHashMap lProps(xModuleConf->getByName(sCommand));
            String sLabel = String(lProps.getUnpackedValueOrDefault(CMDPROP_UINAME, ::rtl::OUString()));
            if (sLabel.Len())
                return sLabel;
        }
    }
    catch(const css::uno::RuntimeException&)
        { throw; }
    catch(const css::uno::Exception&)
        {}

    // may be it's a style URL .. they must be handled special
    SfxStyleInfo_Impl aStyle;
    aStyle.sCommand = sCommand;
    if (m_aStylesInfo.parseStyleCommand(aStyle))
    {
        m_aStylesInfo.getLabel4Style(aStyle);
        return aStyle.sLabel;
    }
    else
    {
        String aRet(rtl::OUString("Symbols: "));
        xub_StrLen nPos = sCommand.SearchAscii(".uno:InsertSymbol?Symbols:string=");
        if ( nPos == 0 )
        {
            aRet += String( sCommand, 34, sCommand.Len()-34 );
            return aRet;
        }
    }

    return sCommand;
}

SfxTabPage* SfxAcceleratorConfigPage::Create( Window* pParent, const SfxItemSet& rAttrSet )
{
    return new SfxAcceleratorConfigPage( pParent, rAttrSet );
}

//-----------------------------------------------
css::uno::Reference< css::frame::XModel > SfxAcceleratorConfigPage::SearchForAlreadyLoadedDoc(const String& /*sName*/)
{
    return css::uno::Reference< css::frame::XModel >();
}

SvxShortcutAssignDlg::SvxShortcutAssignDlg( Window* pParent, const uno::Reference< frame::XFrame >& rxDocumentFrame, const SfxItemSet& rSet )
    : SfxSingleTabDialog( pParent, rSet, 0 )
{
    SfxTabPage* pPage = SfxAcceleratorConfigPage::Create( this, rSet );
    pPage->SetFrame( rxDocumentFrame );
    SetTabPage( pPage );

}

SvxShortcutAssignDlg::~SvxShortcutAssignDlg()
{
}


// .uno:InsertSymbol?Symbols:string=bla

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
