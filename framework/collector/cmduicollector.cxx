/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: cmduicollector.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_framework.hxx"

#include <sal/main.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>

#include <rsc/rscsfx.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/servicefactory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <tools/resmgr.hxx>
#include <tools/rc.h>
#include <tools/isolang.hxx>
//#include <vcl/rc.hxx>
#include <vcl/menu.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/keycodes.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/Locale.hpp>

#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <hash_map>
#include <vector>

using namespace rtl;
namespace css = ::com::sun::star;

// Menubars
const unsigned short MENUBAR_GLOBAL             = 261;
const unsigned short MENUBAR_BASIC              = 14851;
const unsigned short MENUBAR_WRITER             = 20016;
const unsigned short MENUBAR_WRITERWEB          = 20019;
const unsigned short MENUBAR_WRITERGLOBAL       = 20029;
const unsigned short MENUBAR_CALC               = 26007;
const unsigned short MENUBAR_DRAW               = 23009;
const unsigned short MENUBAR_CHART              = 20081;
const unsigned short MENUBAR_MATH               = 20814;
const unsigned short MENUBAR_IMPRESS            = 23006;
const unsigned short MENUBAR_BIBLIOGRAPHY       = 15901;
const unsigned short MENUBAR_BACKINGCOMP        = 261;
const unsigned short MENUBAR_DBACCESS_TABLE     = 19210;
const unsigned short MENUBAR_DBACCESS_QUERY     = 19211;
const unsigned short MENUBAR_DBACCESS_RELATION  = 19212;

// Accelerators (TODO_AS change numbers)
const unsigned short ACC_GLOBAL             = 262;
const unsigned short ACC_WRITER             = 20017;
const unsigned short ACC_WRITERWEB          = 20017;    // same as writer ?
const unsigned short ACC_WRITERGLOBAL       = 20017;    // same as writer ?
const unsigned short ACC_CALC               = 26006;
const unsigned short ACC_DRAW               = 23008;
const unsigned short ACC_CHART              = 20080;
const unsigned short ACC_MATH               = 20815;
const unsigned short ACC_IMPRESS            = 23008;    // same as draw ?
const unsigned short ACC_BASIC              = 0;
const unsigned short ACC_BIBLIOGRAPHY       = 0;
const unsigned short ACC_BACKINGCOMP        = 0;
const unsigned short ACC_DBACCESS_TABLE     = 0;
const unsigned short ACC_DBACCESS_QUERY     = 0;
const unsigned short ACC_DBACCESS_RELATION  = 0;

// Toolbars
const unsigned short TOOLBAR_functionbar                = 560;
const unsigned short TOOLBAR_fullscreenbar              = 558;
const unsigned short TOOLBAR_macrobar                   = 14850;
const unsigned short TOOLBAR_formsnavigationbar         = 18003;
const unsigned short TOOLBAR_formsfilterbar             = 18004;
const unsigned short TOOLBAR_mathtoolbar                = 20050;
//const unsigned short TOOLBAR_chartobjectbar             = 30001;
const unsigned short TOOLBAR_charttoolbar               = 30513;
const unsigned short TOOLBAR_calctextobjectbar          = 25005;
const unsigned short TOOLBAR_calcformsobjectbar         = 25047;
const unsigned short TOOLBAR_calcdrawobjectbar          = 25053;
const unsigned short TOOLBAR_calcgrapicobjectbar        = 25054;
const unsigned short TOOLBAR_calcformatobjectbar        = 25001;
const unsigned short TOOLBAR_calcpreviewbar             = 25006;
const unsigned short TOOLBAR_calctoolbar                = 25035;
const unsigned short TOOLBAR_drawbezierobjectbar        = 23015;
const unsigned short TOOLBAR_drawgluepointsobjectbar    = 23019;
const unsigned short TOOLBAR_impressgraphicobjectbar    = 23030;
const unsigned short TOOLBAR_drawdrawingobjectbar       = 23013;
const unsigned short TOOLBAR_impresstextobjectbar       = 23016;
const unsigned short TOOLBAR_drawtextobjectbar          = 23028;
const unsigned short TOOLBAR_impresstoolbar             = 23011;
const unsigned short TOOLBAR_impressoptionsbar          = 23020;
const unsigned short TOOLBAR_impresscommontaskbar       = 23021;
const unsigned short TOOLBAR_drawtoolbar                = 23025;
const unsigned short TOOLBAR_drawoptionsbar             = 23026;
const unsigned short TOOLBAR_drawgraphicobjectbar       = 23027;
const unsigned short TOOLBAR_impressoutlinetoolbar      = 23017;
const unsigned short TOOLBAR_impressslideviewtoolbar    = 23012;
const unsigned short TOOLBAR_impressslideviewobjectbar  = 23014;
const unsigned short TOOLBAR_writerbezierobjectbar      = 23283;
const unsigned short TOOLBAR_writerdrawingobjectbar     = 23269;
const unsigned short TOOLBAR_writerformsobjectbar       = 23299;
const unsigned short TOOLBAR_writerdrawtextobjectbar    = 23270;
const unsigned short TOOLBAR_writerframeobjectbar       = 23267;
const unsigned short TOOLBAR_writergraphicobjectbar     = 23268;
const unsigned short TOOLBAR_writernumobjectbar         = 23271;
const unsigned short TOOLBAR_writeroleobjectbar         = 23272;
const unsigned short TOOLBAR_writertableobjectbar       = 23266;
const unsigned short TOOLBAR_writertextobjectbar        = 23265;
const unsigned short TOOLBAR_writerpreviewobjectbar     = 20631;
//const unsigned short TOOLBAR_websourceviewobjectbar     = 23282;
//const unsigned short TOOLBAR_websourceviewtoolbar       = 20412;
const unsigned short TOOLBAR_webtextobjectbar           = 20403;
const unsigned short TOOLBAR_writertoolbar              = 23273;
const unsigned short TOOLBAR_webframeobjectbar          = 20408;
const unsigned short TOOLBAR_webgraphicobjectbar        = 20410;
const unsigned short TOOLBAR_weboleobjectbar            = 20411;
const unsigned short TOOLBAR_webtoolbar                 = 20402;
//const unsigned short TOOLBAR_BRW_TAB_TOOLBOX            = 19200;
const unsigned short TOOLBAR_BRW_QRY_TOOLBOX            = 19201;
//const unsigned short TOOLBAR_QRY_TOOLBOX_SQL            = 19202;
const unsigned short TOOLBAR_BRW_QUERYDESIGN_TOOLBOX    = 19203;
const unsigned short TOOLBAR_BRW_REALTIONDESIGN_TOOLBOX = 19204;
const unsigned short TOOLBAR_fontworkobjectbar          = 10987;
const unsigned short TOOLBAR_extrusionobjectbar         = 10986;
const unsigned short TOOLBAR_writerviewerbar            = 23310;
const unsigned short TOOLBAR_calcviewerbar              = 25000;
const unsigned short TOOLBAR_impressviewerbar           = 23023;
const unsigned short TOOLBAR_drawviewerbar              = 23024;

// Sub-Toolbars
const unsigned short RID_LIB_START                      = 10000;
const unsigned short RID_APP_START                      = 20000;

// Draw/Impress
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ALIGNMENT       = RID_APP_START+1;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ALIGNMENT_TBX   = RID_APP_START+5;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ZOOM            = RID_APP_START+2;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ZOOM_TBX        = RID_APP_START+6;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_CHOOSE_MODE         = RID_APP_START+7;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_CHOOSE_MODE_TBX     = RID_APP_START+8;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_TEXT            = RID_APP_START+10;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_TEXT_TBX        = RID_APP_START+11;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_RECTANGLES      = RID_APP_START+12;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_RECTANGLES_TBX  = RID_APP_START+13;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ELLIPSES        = RID_APP_START+14;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ELLIPSES_TBX    = RID_APP_START+15;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_LINES           = RID_APP_START+16;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_LINES_TBX       = RID_APP_START+17;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_3D_OBJECTS      = RID_APP_START+18;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_3D_OBJECTS_TBX  = RID_APP_START+19;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_INSERT          = RID_APP_START+20;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_INSERT_TBX      = RID_APP_START+21;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_POSITION        = RID_APP_START+22;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_POSITION_TBX    = RID_APP_START+23;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_CONNECTORS      = RID_APP_START+24;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_CONNECTORS_TBX  = RID_APP_START+25;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ARROWS          = RID_APP_START+26;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_ARROWS_TBX      = RID_APP_START+27;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_GRAFFILTERS     = RID_APP_START+29;
const unsigned short SUBTOOLBAR_DRAWIMPRESS_RID_GRAFFILTERS_TBX = RID_APP_START+30;

//Writer
const unsigned short RID_SW_START = 20000;
const unsigned short RC_BASE   = RID_SW_START;
const unsigned short RC_RIBBAR = (RC_BASE + 720);
const unsigned short RC_RIBBAR_BEGIN = RC_RIBBAR;
const unsigned short SUBTOOLBAR_WRITER_RID_INSERT_CTRL          = (RC_RIBBAR_BEGIN + 24);
const unsigned short SUBTOOLBAR_WRITER_TBX_INSERT               = 1;
const unsigned short SUBTOOLBAR_WRITER_RID_INSERT_OBJ_CTRL      = (RC_RIBBAR_BEGIN + 25);
const unsigned short SUBTOOLBAR_WRITER_TBX_OBJ_INSERT           = 2;
const unsigned short SUBTOOLBAR_WRITER_RID_TBL_OPT_CTRL         = (RC_RIBBAR_BEGIN + 27);
const unsigned short SUBTOOLBAR_WRITER_TBX_OPTIMIZE_TABLE       = 1;

//Calc
const unsigned short SC_RESOURCE_START                          = (RID_APP_START+5000);
const unsigned short SUBTOOLBAR_CALC_RID_TBXCTL_INSERT          = (SC_RESOURCE_START+38);
const unsigned short SUBTOOLBAR_CALC_RID_TOOLBOX_INSERT         = 1;
const unsigned short SUBTOOLBAR_CALC_RID_TBXCTL_INSCELLS        = (SC_RESOURCE_START+39);
const unsigned short SUBTOOLBAR_CALC_RID_TOOLBOX_INSCELLS       = 2;
const unsigned short SUBTOOLBAR_CALC_RID_TBXCTL_INSOBJ          = (SC_RESOURCE_START+40);
const unsigned short SUBTOOLBAR_CALC_RID_TOOLBOX_INSOBJ         = 3;

//Basic
const unsigned short RID_BASICIDE_START                         = (RID_LIB_START+4850);
const unsigned short SUBTOOLBAR_BASIC_RID_TBXCONTROLS           = ( RID_BASICIDE_START + 65 );
const unsigned short SUBTOOLBAR_BASIC_RID_TOOLBOX               = ( RID_BASICIDE_START + 66 );

//Global
const unsigned short RID_SVX_START                              = (RID_LIB_START);
const unsigned short SUBTOOLBAR_GLOBAL_RID_SVXTBX_DRAW          = (RID_SVX_START + 189);
const unsigned short SUBTOOLBAR_GLOBAL_TBX_DRAW                 = 1;
const unsigned short SUBTOOLBAR_GLOBAL_RID_SVXTBX_ALIGNMENT     = (RID_SVX_START + 190);
const unsigned short SUBTOOLBAR_GLOBAL_TBX_ALIGNMENT            = 10131; // SID_OBJECT_ALIGN_LEFT

struct KeyIdentifierInfo
{
    sal_Int16 Code      ;
    char*     Identifier;
};

KeyIdentifierInfo AWTKeyIdentifierMap[] =
{
    {css::awt::Key::NUM0          , "KEY_0"          },
    {css::awt::Key::NUM1          , "KEY_1"          },
    {css::awt::Key::NUM2          , "KEY_2"          },
    {css::awt::Key::NUM3          , "KEY_3"          },
    {css::awt::Key::NUM4          , "KEY_4"          },
    {css::awt::Key::NUM5          , "KEY_5"          },
    {css::awt::Key::NUM6          , "KEY_6"          },
    {css::awt::Key::NUM7          , "KEY_7"          },
    {css::awt::Key::NUM8          , "KEY_8"          },
    {css::awt::Key::NUM9          , "KEY_9"          },
    {css::awt::Key::A             , "KEY_A"          },
    {css::awt::Key::B             , "KEY_B"          },
    {css::awt::Key::C             , "KEY_C"          },
    {css::awt::Key::D             , "KEY_D"          },
    {css::awt::Key::E             , "KEY_E"          },
    {css::awt::Key::F             , "KEY_F"          },
    {css::awt::Key::G             , "KEY_G"          },
    {css::awt::Key::H             , "KEY_H"          },
    {css::awt::Key::I             , "KEY_I"          },
    {css::awt::Key::J             , "KEY_J"          },
    {css::awt::Key::K             , "KEY_K"          },
    {css::awt::Key::L             , "KEY_L"          },
    {css::awt::Key::M             , "KEY_M"          },
    {css::awt::Key::N             , "KEY_N"          },
    {css::awt::Key::O             , "KEY_O"          },
    {css::awt::Key::P             , "KEY_P"          },
    {css::awt::Key::Q             , "KEY_Q"          },
    {css::awt::Key::R             , "KEY_R"          },
    {css::awt::Key::S             , "KEY_S"          },
    {css::awt::Key::T             , "KEY_T"          },
    {css::awt::Key::U             , "KEY_U"          },
    {css::awt::Key::V             , "KEY_V"          },
    {css::awt::Key::W             , "KEY_W"          },
    {css::awt::Key::X             , "KEY_X"          },
    {css::awt::Key::Y             , "KEY_Y"          },
    {css::awt::Key::Z             , "KEY_Z"          },
    {css::awt::Key::F1            , "KEY_F1"         },
    {css::awt::Key::F2            , "KEY_F2"         },
    {css::awt::Key::F3            , "KEY_F3"         },
    {css::awt::Key::F4            , "KEY_F4"         },
    {css::awt::Key::F5            , "KEY_F5"         },
    {css::awt::Key::F6            , "KEY_F6"         },
    {css::awt::Key::F7            , "KEY_F7"         },
    {css::awt::Key::F8            , "KEY_F8"         },
    {css::awt::Key::F9            , "KEY_F9"         },
    {css::awt::Key::F10           , "KEY_F10"        },
    {css::awt::Key::F11           , "KEY_F11"        },
    {css::awt::Key::F12           , "KEY_F12"        },
    {css::awt::Key::F13           , "KEY_F13"        },
    {css::awt::Key::F14           , "KEY_F14"        },
    {css::awt::Key::F15           , "KEY_F15"        },
    {css::awt::Key::F16           , "KEY_F16"        },
    {css::awt::Key::F17           , "KEY_F17"        },
    {css::awt::Key::F18           , "KEY_F18"        },
    {css::awt::Key::F19           , "KEY_F19"        },
    {css::awt::Key::F20           , "KEY_F20"        },
    {css::awt::Key::F21           , "KEY_F21"        },
    {css::awt::Key::F22           , "KEY_F22"        },
    {css::awt::Key::F23           , "KEY_F23"        },
    {css::awt::Key::F24           , "KEY_F24"        },
    {css::awt::Key::F25           , "KEY_F25"        },
    {css::awt::Key::F26           , "KEY_F26"        },
    {css::awt::Key::DOWN          , "KEY_DOWN"       },
    {css::awt::Key::UP            , "KEY_UP"         },
    {css::awt::Key::LEFT          , "KEY_LEFT"       },
    {css::awt::Key::RIGHT         , "KEY_RIGHT"      },
    {css::awt::Key::HOME          , "KEY_HOME"       },
    {css::awt::Key::END           , "KEY_END"        },
    {css::awt::Key::PAGEUP        , "KEY_PAGEUP"     },
    {css::awt::Key::PAGEDOWN      , "KEY_PAGEDOWN"   },
    {css::awt::Key::RETURN        , "KEY_RETURN"     },
    {css::awt::Key::ESCAPE        , "KEY_ESCAPE"     },
    {css::awt::Key::TAB           , "KEY_TAB"        },
    {css::awt::Key::BACKSPACE     , "KEY_BACKSPACE"  },
    {css::awt::Key::SPACE         , "KEY_SPACE"      },
    {css::awt::Key::INSERT        , "KEY_INSERT"     },
    {css::awt::Key::DELETE        , "KEY_DELETE"     },
    {css::awt::Key::ADD           , "KEY_ADD"        },
    {css::awt::Key::SUBTRACT      , "KEY_SUBTRACT"   },
    {css::awt::Key::MULTIPLY      , "KEY_MULTIPLY"   },
    {css::awt::Key::DIVIDE        , "KEY_DIVIDE"     },
    {css::awt::Key::POINT         , "KEY_POINT"      },
    {css::awt::Key::COMMA         , "KEY_COMMA"      },
    {css::awt::Key::LESS          , "KEY_LESS"       },
    {css::awt::Key::GREATER       , "KEY_GREATER"    },
    {css::awt::Key::EQUAL         , "KEY_EQUAL"      },
    {css::awt::Key::OPEN          , "KEY_OPEN"       },
    {css::awt::Key::CUT           , "KEY_CUT"        },
    {css::awt::Key::COPY          , "KEY_COPY"       },
    {css::awt::Key::PASTE         , "KEY_PASTE"      },
    {css::awt::Key::UNDO          , "KEY_UNDO"       },
    {css::awt::Key::REPEAT        , "KEY_REPEAT"     },
    {css::awt::Key::FIND          , "KEY_FIND"       },
    {css::awt::Key::PROPERTIES    , "KEY_PROPERTIES" },
    {css::awt::Key::FRONT         , "KEY_FRONT"      },
    {css::awt::Key::CONTEXTMENU   , "KEY_CONTEXTMENU"},
    {css::awt::Key::HELP          , "KEY_HELP"       },
    {css::awt::Key::MENU          , "KEY_MENU"       },
    {css::awt::Key::HANGUL_HANJA  , "KEY_HANGUL_HANJA"},
    {css::awt::Key::DECIMAL       , "KEY_DECIMAL"    },
    {css::awt::Key::TILDE         , "KEY_TILDE"      },
    {css::awt::Key::QUOTELEFT     , "KEY_QUOTELEFT"  },
    {0                            , ""               } // mark the end of this array!
};

struct ISO_code
{
    const char* pLanguage;
    const char* pCountry;
};

const int NUM_LANGUAGES = 27;
//const int NUM_LANGUAGES = 1;

ISO_code Language_codes[] =
{
    { "de", ""       },
    { "en", "US"     },
    { "ar", ""       },
    { "no", ""       },
    { "ca", ""       },
    { "zh", "TW"     },
    { "cs", ""       },
    { "da", ""       },
    { "el", ""       },
    { "es", ""       },
    { "fi", ""       },
    { "fr", ""       },
    { "he", ""       },
    { "it", ""       },
    { "ja", ""       },
    { "ko", ""       },
    { "nl", ""       },
    { "pl", ""       },
    { "pt", "BR"     },
    { "ru", ""       },
    { "sk", ""       },
    { "sv", ""       },
    { "th", ""       },
    { "tr", ""       },
    { "hi", ""       },
    { "zh", "CN"     },
    { "pt", ""       },
    { 0,  0          }
};

enum MODULES
{
    MODULE_GLOBAL,
    MODULE_WRITER,
    MODULE_CALC,
    MODULE_DRAWIMPRESS,
    MODULE_CHART,
    MODULE_MATH,
    MODULE_BASIC,
    MODULE_BIBLIO,
    MODULE_BACKINGCOMP,
    MODULE_DBACCESS,
    MODULE_COUNT
};

enum SUBMODULES
{
    SUBMODULE_GLOBAL       = 1,
    SUBMODULE_WRITER       = 2,
    SUBMODULE_WRITERWEB    = 4,
    SUBMODULE_WRITERGLOBAL = 8,
    SUBMODULE_CALC         = 16,
    SUBMODULE_DRAW         = 32,
    SUBMODULE_IMPRESSS     = 64,
    SUBMODULE_CHART        = 128,
    SUBMODULE_MATH         = 256,
    SUBMODULE_BASIC        = 512,
    SUBMODULE_BIBLIO       = 1024,
    SUBMODULE_BACKINGCOMP  = 2048,
    SUBMODULE_DB_TABLE     = 4096,
    SUBMODULE_DB_QUERY     = 8192,
    SUBMODULE_DB_RELATION  = 16384
};

enum PARENT_RESTYPES
{
    PARENT_RESTYPE_FLOATINGWINDOW
};

struct Projects
{
    const char* pProjectFolder;
    const char* pResPrefix;
    const char* pCSVPrefix;
    bool        bSlotCommands;
    MODULES     eBelongsTo;
};

Projects ProjectModule_Mapping[] =
{
    { "sfx2"        , "sfx",    "sfx2",     true,   MODULE_GLOBAL         },
    { "svx"         , "svx",    "svx",      true,   MODULE_GLOBAL         },
    { "svx"         , "ofa",    "ofa",      true,   MODULE_GLOBAL         },
    { "sw"          , "sw",     "sw",       true,   MODULE_WRITER         },
    { "sd"          , "sd",     "sd",       true,   MODULE_DRAWIMPRESS    },
    { "sc"          , "sc",     "sc",       true,   MODULE_CALC           },
    { "sch"         , "sch",    "sch",      true,   MODULE_CHART          },
    { "starmath"    , "sm",     "starmath", true,   MODULE_MATH           },
    { "basctl"      , "basctl", "bastctl",  true,   MODULE_BASIC          },
    { "extensions"  , "bib",    "",         false,  MODULE_BIBLIO         },
    { "offmgr"      , "ofa",    "",         false,  MODULE_BACKINGCOMP    },
    { "dbaccess"    , "dbu",    "",         false,  MODULE_DBACCESS       },
    { 0             , 0,        "",         false,  MODULE_BASIC          }
};

const char XMLFileExtension[] = ".xcu";
const char* ModuleToXML_Mapping[] =
{
    "GenericCommands",
    "WriterCommands",
    "CalcCommands",
    "DrawImpressCommands",
    "ChartCommands",
    "MathCommands",
    "BasicIDECommands",
    "BibliographyCommands",
    "StartModuleCommands",
    "DbuCommands",
    0
};

struct ResourceToModule
{
    unsigned short nResId;
    const char*    pXMLPrefix;
    MODULES        eBelongsTo;
    bool           bWritten;
};

struct SubResourceToModule
{
    unsigned short  nParentResId;
    PARENT_RESTYPES eParentResType;
    unsigned short  nResId;
    const char*     pXMLPrefix;
    MODULES         eBelongsTo;
    bool            bWritten;
};

ResourceToModule ResourceModule_Mapping[] =
{
    { MENUBAR_GLOBAL,           "default",      MODULE_GLOBAL       },  // important: To prevent duplicate popup menu entries make sure this is the first entry!!
    { MENUBAR_BASIC,            "basic",        MODULE_BASIC        },
    { MENUBAR_WRITER,           "writer",       MODULE_WRITER       },
    { MENUBAR_WRITERWEB,        "web",          MODULE_WRITER       },
    { MENUBAR_WRITERGLOBAL,     "global",       MODULE_WRITER       },
    { MENUBAR_CALC,             "calc",         MODULE_CALC         },
    { MENUBAR_DRAW,             "draw",         MODULE_DRAWIMPRESS  },
    { MENUBAR_CHART,            "chart",        MODULE_CHART        },
    { MENUBAR_MATH,             "math",         MODULE_MATH         },
    { MENUBAR_IMPRESS,          "impress",      MODULE_DRAWIMPRESS  },
    { MENUBAR_BIBLIOGRAPHY,     "biblio",       MODULE_BIBLIO       },
    { MENUBAR_BACKINGCOMP,      "backing",      MODULE_BACKINGCOMP  },
    { MENUBAR_DBACCESS_TABLE,   "dbtable",      MODULE_DBACCESS     },
    { MENUBAR_DBACCESS_QUERY,   "dbquery",      MODULE_DBACCESS     },
    { MENUBAR_DBACCESS_RELATION,"dbrelation",   MODULE_DBACCESS     },
    { 0,                        "",             MODULE_DBACCESS     }
};


ResourceToModule AccResourceModule_Mapping[] =
{
    { ACC_GLOBAL,           "default",      MODULE_GLOBAL       },
    { ACC_WRITER,           "writer",       MODULE_WRITER       },
    { ACC_WRITERWEB,        "web",          MODULE_WRITER       },
    { ACC_WRITERGLOBAL,     "global",       MODULE_WRITER       },
    { ACC_CALC,             "calc",         MODULE_CALC         },
    { ACC_DRAW,             "draw",         MODULE_DRAWIMPRESS  },
    { ACC_CHART,            "chart",        MODULE_CHART        },
    { ACC_MATH,             "math",         MODULE_MATH         },
    { ACC_IMPRESS,          "impress",      MODULE_DRAWIMPRESS  },
    { ACC_BIBLIOGRAPHY,     "biblio",       MODULE_BIBLIO       }, // 0
    { ACC_BACKINGCOMP,      "backing",      MODULE_BACKINGCOMP  }, // 0
    { ACC_DBACCESS_TABLE,   "dbtable",      MODULE_DBACCESS     }, // 0
    { ACC_DBACCESS_QUERY,   "dbquery",      MODULE_DBACCESS     }, // 0
    { ACC_DBACCESS_RELATION,"dbrelation",   MODULE_DBACCESS     }, // 0
    { ACC_BASIC,            "basic",        MODULE_BASIC        }, // 0
    { 0,                    "",             MODULE_COUNT        }
};

ResourceToModule TBResourceModule_Mapping[] =
{
    { TOOLBAR_functionbar                   ,"standardbar",                 MODULE_GLOBAL        , false        },
    { TOOLBAR_fullscreenbar                 ,"fullscreenbar",               MODULE_GLOBAL        , false        },
    { TOOLBAR_macrobar                      ,"macrobar",                    MODULE_BASIC         , false        },
    { TOOLBAR_formsnavigationbar            ,"formsnavigationbar",          MODULE_GLOBAL        , false        },
    { TOOLBAR_formsfilterbar                ,"formsfilterbar",              MODULE_GLOBAL        , false        },
    { TOOLBAR_fontworkobjectbar             ,"fontworkobjectbar",           MODULE_GLOBAL        , false        },
    { TOOLBAR_extrusionobjectbar            ,"extrusionobjectbar",          MODULE_GLOBAL        , false        },
    { TOOLBAR_mathtoolbar                   ,"mathtoolbar",                 MODULE_MATH          , false        },
//    { TOOLBAR_chartobjectbar                ,"chartobjectbar",              MODULE_CHART         , false        },
    { TOOLBAR_charttoolbar                  ,"charttoolbar",                MODULE_CHART         , false        },
    { TOOLBAR_calctextobjectbar             ,"calctextobjectbar",           MODULE_CALC          , false        },
    { TOOLBAR_calcformsobjectbar            ,"calcformsobjectbar",          MODULE_CALC          , false        },
    { TOOLBAR_calcdrawobjectbar             ,"calcdrawobjectbar",           MODULE_CALC          , false        },
    { TOOLBAR_calcgrapicobjectbar           ,"calcgrapicobjectbar",         MODULE_CALC          , false        },
    { TOOLBAR_calcformatobjectbar           ,"calcformatobjectbar",         MODULE_CALC          , false        },
    { TOOLBAR_calcpreviewbar                ,"calcpreviewbar",              MODULE_CALC          , false        },
    { TOOLBAR_calctoolbar                   ,"calctoolbar",                 MODULE_CALC          , false        },
    { TOOLBAR_drawbezierobjectbar           ,"drawbezierobjectbar",         MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawgluepointsobjectbar       ,"drawgluepointsobjectbar",     MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impressgraphicobjectbar       ,"impressgraphicobjectbar",     MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawdrawingobjectbar          ,"drawdrawingobjectbar",        MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impresstextobjectbar          ,"impresstextobjectbar",        MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawtextobjectbar             ,"drawtextobjectbar",           MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impresstoolbar                ,"impresstoolbar",              MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impressoptionsbar             ,"impressoptionsbar",           MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impresscommontaskbar          ,"impresscommontaskbar",        MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawtoolbar                   ,"drawtoolbar",                 MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawoptionsbar                ,"drawoptionsbar",              MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawgraphicobjectbar          ,"drawgraphicobjectbar",        MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impressoutlinetoolbar         ,"impressoutlinetoolbar",       MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impressslideviewtoolbar       ,"impressslideviewtoolbar",     MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_impressslideviewobjectbar     ,"impressslideviewobjectbar",   MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_writerbezierobjectbar         ,"writerbezierobjectbar",       MODULE_WRITER        , false        },
    { TOOLBAR_writerdrawingobjectbar        ,"writerdrawingobjectbar",      MODULE_WRITER        , false        },
    { TOOLBAR_writerformsobjectbar          ,"writerformsobjectbar",        MODULE_WRITER        , false        },
    { TOOLBAR_writerdrawtextobjectbar       ,"writerdrawtextobjectbar",     MODULE_WRITER        , false        },
    { TOOLBAR_writerframeobjectbar          ,"writerframeobjectbar",        MODULE_WRITER        , false        },
    { TOOLBAR_writergraphicobjectbar        ,"writergraphicobjectbar",      MODULE_WRITER        , false        },
    { TOOLBAR_writernumobjectbar            ,"writernumobjectbar",          MODULE_WRITER        , false        },
    { TOOLBAR_writeroleobjectbar            ,"writeroleobjectbar",          MODULE_WRITER        , false        },
    { TOOLBAR_writertableobjectbar          ,"writertableobjectbar",        MODULE_WRITER        , false        },
    { TOOLBAR_writertextobjectbar           ,"writertextobjectbar",         MODULE_WRITER        , false        },
    { TOOLBAR_writerpreviewobjectbar        ,"writerpreviewobjectbar",      MODULE_WRITER        , false        },
//    { TOOLBAR_websourceviewobjectbar        ,"websourceviewobjectbar",      MODULE_WRITER        , false        },
//    { TOOLBAR_websourceviewtoolbar          ,"websourceviewtoolbar",        MODULE_WRITER        , false        },
    { TOOLBAR_webtextobjectbar              ,"webtextobjectbar",            MODULE_WRITER        , false        },
    { TOOLBAR_writertoolbar                 ,"writertoolbar",               MODULE_WRITER        , false        },
    { TOOLBAR_webframeobjectbar             ,"webframeobjectbar",           MODULE_WRITER        , false        },
    { TOOLBAR_webgraphicobjectbar           ,"webgraphicobjectbar",         MODULE_WRITER        , false        },
    { TOOLBAR_weboleobjectbar               ,"weboleobjectbar",             MODULE_WRITER        , false        },
    { TOOLBAR_webtoolbar                    ,"webtoolbar",                  MODULE_WRITER        , false        },
//    { TOOLBAR_BRW_TAB_TOOLBOX               ,"dbtablebrowsebar",            MODULE_DBACCESS      , false        },
    { TOOLBAR_BRW_QRY_TOOLBOX               ,"dbquerybrowsebar",            MODULE_DBACCESS      , false        },
//    { TOOLBAR_QRY_TOOLBOX_SQL               ,"dbquerysqlbar",               MODULE_DBACCESS      , false        },
    { TOOLBAR_BRW_QUERYDESIGN_TOOLBOX       ,"dbquerydesignbar",            MODULE_DBACCESS      , false        },
    { TOOLBAR_BRW_REALTIONDESIGN_TOOLBOX    ,"dbrelationbrowsebar",         MODULE_DBACCESS      , false        },
    { TOOLBAR_writerviewerbar               ,"writerviewerbar",             MODULE_WRITER        , false        },
    { TOOLBAR_calcviewerbar                 ,"calcviewerbar",               MODULE_CALC          , false        },
    { TOOLBAR_impressviewerbar              ,"impressviewerbar",            MODULE_DRAWIMPRESS   , false        },
    { TOOLBAR_drawviewerbar                 ,"drawviewerbar",               MODULE_DRAWIMPRESS   , false        },
    { 0                                     ,"",                            MODULE_GLOBAL        , false        }
};

struct SubResourceToModule TBSubResourceModule_Mapping[] =
{
    { SUBTOOLBAR_DRAWIMPRESS_RID_ALIGNMENT,   PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_ALIGNMENT_TBX     , "drawimpressalignmentbar"   ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_ZOOM,        PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_ZOOM_TBX          , "drawimpresszoombar"        ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_CHOOSE_MODE,     PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_CHOOSE_MODE_TBX       , "drawimpresschoosemodebar"  ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_TEXT,        PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_TEXT_TBX          , "drawimpresstextbar"        ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_RECTANGLES,  PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_RECTANGLES_TBX    , "drawimpressrectanglebar"   ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_ELLIPSES,    PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_ELLIPSES_TBX      , "drawimpressellipsesbar"    ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_LINES,       PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_LINES_TBX         , "drawimpresslinesbar"       ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_3D_OBJECTS,  PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_3D_OBJECTS_TBX    , "drawimpress3dobjectsbar"   ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_INSERT,      PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_INSERT_TBX        , "drawimpressinsertbar"      ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_POSITION,    PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_POSITION_TBX      , "drawimpresspositionbar"    ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_CONNECTORS,  PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_CONNECTORS_TBX    , "drawimpressconnectorsbar"  ,   MODULE_DRAWIMPRESS  , false },
    { SUBTOOLBAR_DRAWIMPRESS_RID_ARROWS,      PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_ARROWS_TBX        , "drawimpressarrowsbar"      ,   MODULE_DRAWIMPRESS  , false },
//    { SUBTOOLBAR_DRAWIMPRESS_RID_GRAFFILTERS, PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_DRAWIMPRESS_RID_GRAFFILTERS_TBX   , "drawimpressgraffiltersbar" ,   MODULE_DRAWIMPRESS, false },
    { SUBTOOLBAR_WRITER_RID_INSERT_CTRL     , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_WRITER_TBX_INSERT                 , "writerinsertbar"           ,   MODULE_WRITER       , false },
    { SUBTOOLBAR_WRITER_RID_INSERT_OBJ_CTRL , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_WRITER_TBX_OBJ_INSERT             , "writerinsertobjectbar"     ,   MODULE_WRITER       , false },
    { SUBTOOLBAR_WRITER_RID_TBL_OPT_CTRL    , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_WRITER_TBX_OPTIMIZE_TABLE         , "writeroptimizetablebar"    ,   MODULE_WRITER       , false },
    { SUBTOOLBAR_GLOBAL_RID_SVXTBX_DRAW     , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_GLOBAL_TBX_DRAW                   , "genericdrawbar"            ,   MODULE_GLOBAL       , false },
    { SUBTOOLBAR_GLOBAL_RID_SVXTBX_ALIGNMENT, PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_GLOBAL_TBX_ALIGNMENT              , "genericalignmentbar"       ,   MODULE_GLOBAL       , false },
    { SUBTOOLBAR_CALC_RID_TBXCTL_INSERT     , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_CALC_RID_TOOLBOX_INSERT           , "calcinsertbar"             ,   MODULE_CALC         , false },
    { SUBTOOLBAR_CALC_RID_TBXCTL_INSCELLS   , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_CALC_RID_TOOLBOX_INSCELLS         , "calcinsertcellsbar"        ,   MODULE_CALC         , false },
    { SUBTOOLBAR_CALC_RID_TBXCTL_INSOBJ     , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_CALC_RID_TOOLBOX_INSOBJ           , "calcinsertobjectbar"       ,   MODULE_CALC         , false },
    { SUBTOOLBAR_BASIC_RID_TBXCONTROLS      , PARENT_RESTYPE_FLOATINGWINDOW, SUBTOOLBAR_BASIC_RID_TOOLBOX                 , "basicinsertcontrolsbar"    ,   MODULE_BASIC        , false },
    { 0,                                      PARENT_RESTYPE_FLOATINGWINDOW, 0                                            , ""                          ,   MODULE_GLOBAL       , false }
};

struct CommandLabels
{
    unsigned short  nID;
    bool            bWritten;
    bool            bPopupMenu;
    rtl::OUString   aCommand;
    unsigned long   nModules;
    rtl::OUString   aLabels[NUM_LANGUAGES];
    unsigned long   nProperties;

    CommandLabels() :
    nID( 0 ),
    bWritten( false ),
    bPopupMenu( false ),
    nModules( 0 ),
    nProperties( 0 )

    {}
};

struct KeyCommandInfo
{
    KeyCode aVCLKey;
    css::awt::KeyEvent aAWTKey;
    ::rtl::OUString sCommand;
    ::rtl::OUString sKey;
    USHORT nCommandId;

    bool operator==(const KeyCommandInfo& rI) const
    {
        return (
                   (rI.aVCLKey    == aVCLKey) &&
                   (rI.aAWTKey.KeyCode    == aAWTKey.KeyCode) &&
                   (rI.aAWTKey.Modifiers    == aAWTKey.Modifiers) &&
                   (rI.sCommand   == sCommand) &&
                   (rI.sKey       == sKey) &&
                   (rI.nCommandId == nCommandId)
               );
    }
};

struct ToolBarInfo
{
    unsigned short nID;
    bool           bWritten;
    unsigned long  nModules; // Which sub modules need this toolbar
    rtl::OUString  aUIName[NUM_LANGUAGES];

    ToolBarInfo() :
        nID( 0 ),
        bWritten( false ),
        nModules( 0 ) {}
};

struct OUStringHashCode
{
    size_t operator()( const ::rtl::OUString& sString ) const
    {
        return sString.hashCode();
    }
};

typedef std::hash_map< int, CommandLabels > CommandIDToLabelsMap;
typedef std::hash_map< OUString, CommandLabels, OUStringHashCode, ::std::equal_to< OUString > > CommandToLabelsMap;
typedef std::hash_map< int, ToolBarInfo > ResIdToToolBarInfoMap;
typedef std::vector< KeyCommandInfo > KeyCommandInfoList;
typedef std::hash_map< int, KeyCommandInfoList > Locale2KeyCommandInfoList;

static CommandIDToLabelsMap     moduleMapFiles[MODULE_COUNT];
static CommandToLabelsMap       modulePopupMenusCmd[MODULE_COUNT];
static Locale2KeyCommandInfoList moduleAccs[MODULE_COUNT];

bool ExtractVersionNumber( const OUString& rVersion, OUString& rVersionNumber )
{
    bool bCheckNumOnly( false );

    OUStringBuffer aBuf;

    rVersionNumber = OUString();
    for ( int i = 0; i < rVersion.getLength(); i++ )
    {
        if ( rVersion[i] >= sal_Unicode( '0' ) && rVersion[i] <= sal_Unicode( '9' ))
        {
            bCheckNumOnly = true;
            aBuf.append( rVersion[i] );
        }
        else if ( bCheckNumOnly )
            return false;
    }

    rVersionNumber = aBuf.makeStringAndClear();
    return true;
}

bool ReadCSVFile( const OUString& aCVSFileURL, MODULES eModule, const OUString& aProjectName )
{
    osl::File  aCSVFile( aCVSFileURL );
    fprintf(stdout, "############ read csv \"%s\" ... ", ::rtl::OUStringToOString(aCVSFileURL, RTL_TEXTENCODING_UTF8).getStr());
    if ( aCSVFile.open( OpenFlag_Read ) != osl::FileBase::E_None )
    {
        fprintf(stdout, "failed!\n");
        return false;
    }
    fprintf(stdout, "OK!\n");

    {
        sal_Bool            bEOF;
        ::rtl::ByteSequence aCSVLine;
        OUString            aUnoCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

        while ( aCSVFile.isEndOfFile( &bEOF ) == osl::FileBase::E_None && !bEOF )
        {
            aCSVFile.readLine( aCSVLine );

            OString     aLine( (const char *)aCSVLine.getConstArray(), aCSVLine.getLength() );
            OString     aID;
            OString     aAccelState;
            OString     aMenuState;
            OString     aStatusState;
            OString     aToolbarState;
            OString     aCmdName;
            OString     aSlotName;
            OString     aImageRotationState;
            OString     aImageReflectionState;

            sal_Int32 nIndex = 0;
            aID                     = aLine.getToken( 2, ',', nIndex );
            aAccelState             = aLine.getToken( 2, ',', nIndex );
            aMenuState              = aLine.getToken( 0, ',', nIndex );
            aStatusState            = aLine.getToken( 0, ',', nIndex );
            aToolbarState           = aLine.getToken( 0, ',', nIndex );
            aImageRotationState     = aLine.getToken( 8, ',', nIndex );
            aImageReflectionState   = aLine.getToken( 0, ',', nIndex );
            aCmdName                = aLine.getToken( 10, ',', nIndex );
            aSlotName               = aLine.getToken( 1, ',', nIndex );

            if ( aCmdName.getLength() == 0 )
                aCmdName = aSlotName;

            int nID = aID.toInt32();

            if ( nID > 5000 && ( aAccelState.equalsIgnoreAsciiCase( "TRUE" ) ||
                                 aMenuState.equalsIgnoreAsciiCase( "TRUE" ) ||
                                 aStatusState.equalsIgnoreAsciiCase( "TRUE" ) ||
                                 aToolbarState.equalsIgnoreAsciiCase( "TRUE" ) ))
            {
                CommandLabels aCmdLabel;

                aCmdLabel.nID = nID;
                aCmdLabel.aCommand = aUnoCmd;
                aCmdLabel.aCommand += OStringToOUString( aCmdName, RTL_TEXTENCODING_ASCII_US );
                aCmdLabel.nModules |= ( 1 << (unsigned long)( eModule ));

                // Set bitfield
                aCmdLabel.nProperties = (( aImageRotationState.equalsIgnoreAsciiCase( "TRUE" ) ? 1 : 0 ) << 1 );
                aCmdLabel.nProperties |= (( aImageReflectionState.equalsIgnoreAsciiCase( "TRUE" ) ? 1 : 0 ) << 2 );

                moduleMapFiles[int(eModule)].insert( CommandIDToLabelsMap::value_type( nID, aCmdLabel ));
                modulePopupMenusCmd[int(eModule)].insert( CommandToLabelsMap::value_type( aCmdLabel.aCommand, aCmdLabel ));
            }
        }

        aCSVFile.close();
    }

    return true;
}

CommandLabels* RetrieveCommandLabelsFromID( unsigned short nId, MODULES eModule )
{
    CommandIDToLabelsMap::iterator pIter = moduleMapFiles[MODULE_GLOBAL].find( nId );
    if ( pIter != moduleMapFiles[MODULE_GLOBAL].end() )
        return &(pIter->second);
    else if ( eModule != MODULE_GLOBAL )
    {
        CommandIDToLabelsMap::iterator pIter = moduleMapFiles[eModule].find( nId );
        if ( pIter != moduleMapFiles[eModule].end() )
            return &(pIter->second);
    }

    return NULL;
}

CommandLabels* RetrieveCommandLabelsFromCommand( const OUString& aCommand, MODULES eModule )
{
    CommandToLabelsMap::iterator pIter = modulePopupMenusCmd[MODULE_GLOBAL].find( aCommand );
    if ( pIter != modulePopupMenusCmd[MODULE_GLOBAL].end() )
        return &(pIter->second);
    else if ( eModule != MODULE_GLOBAL )
    {
        CommandToLabelsMap::iterator pIter = modulePopupMenusCmd[eModule].find( aCommand );
        if ( pIter != modulePopupMenusCmd[eModule].end() )
            return &(pIter->second);
    }

    return NULL;
}

void AddCommandLabelsToIDHashMap( MODULES eModule, const CommandLabels& rNewCmdLabels )
{
    moduleMapFiles[int(eModule)].insert( CommandIDToLabelsMap::value_type( rNewCmdLabels.nID, rNewCmdLabels ));
}

void AddCommandLabelsToCommandHashMap( MODULES eModule, const CommandLabels& rNewCmdLabels )
{
    modulePopupMenusCmd[int(eModule)].insert( CommandToLabelsMap::value_type( rNewCmdLabels.aCommand, rNewCmdLabels ));
}

KeyCode impl_KeyCodeAWT2VCL(const css::awt::KeyEvent& aAWTKey)
{
    BOOL   bShift = ((aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
    BOOL   bMod1  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
    BOOL   bMod2  = ((aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );
    USHORT nKey   = (USHORT)aAWTKey.KeyCode;
    // unfortunately MENU and CONTEXTMENU are twisted between vcl and awt
    if( aAWTKey.KeyCode == css::awt::Key::MENU )
        nKey = KEY_CONTEXTMENU;
    else if( aAWTKey.KeyCode == css::awt::Key::CONTEXTMENU )
        nKey = KEY_MENU;
    return KeyCode(nKey, bShift, bMod1, bMod2);
}

css::awt::KeyEvent impl_KeyCodeVCL2AWT(const KeyCode& aVCLKey)
{
    css::awt::KeyEvent aAWTKey;
    aAWTKey.Modifiers = 0;
    aAWTKey.KeyCode   = (sal_Int16)aVCLKey.GetCode();
    // unfortunately MENU and CONTEXTMENU are twisted between vcl and awt
    if( aAWTKey.KeyCode == css::awt::Key::MENU )
        aAWTKey.KeyCode = css::awt::Key::CONTEXTMENU;
    else if( aAWTKey.KeyCode == css::awt::Key::CONTEXTMENU )
        aAWTKey.KeyCode = css::awt::Key::MENU;

    if (aVCLKey.IsShift())
        aAWTKey.Modifiers |= css::awt::KeyModifier::SHIFT;
    if (aVCLKey.IsMod1())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD1;
    if (aVCLKey.IsMod2())
        aAWTKey.Modifiers |= css::awt::KeyModifier::MOD2;

    return aAWTKey;
}

bool impl_isDuplicateKey(const KeyCommandInfoList& rList,
                         const KeyCommandInfo&     aInfo)
{
    KeyCommandInfoList::const_iterator pInfo;
    for (  pInfo  = rList.begin();
           pInfo != rList.end()  ;
         ++pInfo                 )
    {
        if (
            (pInfo->aVCLKey    == aInfo.aVCLKey   ) &&
            (pInfo->nCommandId == aInfo.nCommandId)
           )
        {
            return true;
        }
    }
    return false;
}

bool ReadAccelerators( int nLangIndex, ResMgr* pResMgr, MODULES eModule, const ::rtl::OUString& aProjectName, Accelerator* pAcc )
{
    Locale2KeyCommandInfoList& rLocale = moduleAccs[eModule];
    KeyCommandInfoList&        rAccs   = rLocale[nLangIndex];

    USHORT c = pAcc->GetItemCount();
    USHORT i = 0;
    for (i=0; i<c; ++i)
    {
        USHORT nItem = pAcc->GetItemId(i);

        KeyCommandInfo aInfo;
        aInfo.nCommandId = nItem;
        aInfo.aVCLKey    = pAcc->GetItemKeyCode(i); // dont use GetKeyCode(nItem) ... You cant handle logic keys then. Because they have the same ID - but another keycode!
        aInfo.aAWTKey    = impl_KeyCodeVCL2AWT(aInfo.aVCLKey);

        fprintf(stdout, "     acc key=%d\tid=%d", aInfo.aVCLKey.GetFullCode(), aInfo.nCommandId);
        CommandLabels* pCmdInfo = RetrieveCommandLabelsFromID(aInfo.nCommandId, eModule);
        if (!pCmdInfo || !pCmdInfo->aCommand.getLength())
        {
            fprintf(stdout, "\t\t\t=>\tERROR\t: no command [mod=%d]\n", eModule);
            continue;
        }
        aInfo.sCommand = pCmdInfo->aCommand;
        fprintf(stdout, "\tcmd=%s\t=>\t", ::rtl::OUStringToOString(aInfo.sCommand, RTL_TEXTENCODING_UTF8).getStr());

        sal_Int32 i = 0;
        while(AWTKeyIdentifierMap[i].Code != 0)
        {
            const KeyIdentifierInfo& rKeyInfo = AWTKeyIdentifierMap[i];
            if (aInfo.aAWTKey.KeyCode == rKeyInfo.Code)
            {
                aInfo.sKey = ::rtl::OUString::createFromAscii(rKeyInfo.Identifier);
                break;
            }
            ++i;
        }

        if (!aInfo.sKey.getLength())
        {
            fprintf(stdout, "ERROR\t: no key identifier\n");
            continue;
        }

        if (impl_isDuplicateKey(rAccs, aInfo))
        {
            fprintf(stdout, "WARNING\t: key is duplicate\n");
            continue;
        }

        fprintf(stdout, "\n");
        rAccs.push_back(aInfo);
    }

    return true;
}

bool ReadMenuLabels( int nLangIndex, Menu* pMenu, MODULES eModule, bool bHasSlotInfos, OUString aResourceFilePrefix )
{
    OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

    for ( unsigned short n = 0; n < pMenu->GetItemCount(); n++ )
    {
        unsigned short nId = pMenu->GetItemId( n );
        if ( pMenu->GetItemType( n ) != MENUITEM_SEPARATOR )
        {
            OUString aLabel( pMenu->GetItemText( nId ));
            OUString aCommand( pMenu->GetItemCommand( nId ));
            bool     bHasUnoCommand( aCommand.getLength() > 0 && ( aCommand.indexOf( aCmd ) == 0 ));

            if ( pMenu->GetPopupMenu( nId ) != 0 )
            {
                CommandLabels* pCmdLabels = 0;
                // Special popup menus have ID's between 5000-10000. Apps uses various ID's from their number area =>
                // don't use ID if outside this area, you can accidently map popups to functions!!
                if (( nId > 5000 ) && ( nId < 10000 ))
                    pCmdLabels = RetrieveCommandLabelsFromID( nId, eModule );
                if ( !pCmdLabels )
                    pCmdLabels = RetrieveCommandLabelsFromCommand( aCommand, eModule );

                if ( pCmdLabels )
                {
                    pCmdLabels->aLabels[nLangIndex] = aLabel;
                    pCmdLabels->bPopupMenu = true;
                }
                else
                {
                    if ( bHasUnoCommand )
                    {
                        CommandLabels aCmdLabels;

                        aCmdLabels.nID = 0;
                        aCmdLabels.aCommand = aCommand;
                        aCmdLabels.aLabels[nLangIndex] = aLabel;
                        aCmdLabels.bPopupMenu = true;
                        AddCommandLabelsToCommandHashMap( eModule, aCmdLabels );
                    }
                    else
                    {
                        fprintf( stderr, "Popup menu (resource = %s ) with unknown uno-command and id '%s', ID = %d, aCmd = %s\n",
                            OUStringToOString( aResourceFilePrefix, RTL_TEXTENCODING_UTF8 ).getStr(),
                            OUStringToOString( aLabel, RTL_TEXTENCODING_UTF8 ).getStr(),
                            nId,
                            OUStringToOString( aCommand, RTL_TEXTENCODING_ASCII_US ).getStr() );
                    }
                }

                ReadMenuLabels( nLangIndex, pMenu->GetPopupMenu( nId ), eModule, bHasSlotInfos, aResourceFilePrefix );
            }
            else
            {
                if ( bHasSlotInfos )
                {
                    CommandLabels* pCmdLabels = RetrieveCommandLabelsFromID( nId, eModule );
                    if ( !pCmdLabels )
                        pCmdLabels = RetrieveCommandLabelsFromCommand( aCommand, eModule );

                    if ( pCmdLabels )
                        pCmdLabels->aLabels[nLangIndex] = aLabel;
                    else if ( bHasUnoCommand )
                    {
                        CommandLabels aCmdLabels;

                        aCmdLabels.nID = ( nId > 5000 ) ? nId : 0;
                        aCmdLabels.aCommand = aCommand;
                        aCmdLabels.aLabels[nLangIndex] = aLabel;
                        AddCommandLabelsToCommandHashMap( eModule, aCmdLabels );
                        if ( nId > 5000 )
                            AddCommandLabelsToIDHashMap( eModule, aCmdLabels );
                   }
                   else
                   {
                        fprintf( stderr, "Menu item with unknown uno-command and id '%s', ID = %d, aCmd = %s\n",
                            OUStringToOString( aLabel, RTL_TEXTENCODING_UTF8 ).getStr(),
                            nId,
                            OUStringToOString( aCommand, RTL_TEXTENCODING_ASCII_US ).getStr() );
                   }
                }
                else if ( bHasUnoCommand )
                {
                    CommandLabels* pCmdLabels = RetrieveCommandLabelsFromCommand( aCommand, eModule );
                    if ( pCmdLabels )
                        pCmdLabels->aLabels[nLangIndex] = aLabel;
                    else
                    {
                        CommandLabels aCmdLabels;

                        aCmdLabels.nID = 0;
                        aCmdLabels.aCommand = aCommand;
                        aCmdLabels.aLabels[nLangIndex] = aLabel;
                        AddCommandLabelsToCommandHashMap( eModule, aCmdLabels );
                    }
                }
            }
        }
    }

    return true;
}

class SfxSlotInfo : public Resource
{
    OUString aName;
    OUString aHelpText;
public:
    SfxSlotInfo(const ResId &rResId);

    const OUString &GetName() const { return aName; }
    const OUString &GetHelpText() const { return aHelpText; }
};

SfxSlotInfo::SfxSlotInfo( const ResId &rResId ) :
    Resource( rResId.SetRT( RSC_SFX_SLOT_INFO ) )
{
    unsigned short nMask = (unsigned short) ReadShortRes();
    if(nMask & RSC_SFX_SLOT_INFO_SLOTNAME)
    {
        aName = ReadStringRes();
    }
    if(nMask & RSC_SFX_SLOT_INFO_HELPTEXT)
    {
        aHelpText = ReadStringRes();
    }
}

bool ReadSlotInfos( int nLangIndex, ResMgr* pResMgr, MODULES eModule, const OUString& aProjectName )
{
    CommandIDToLabelsMap::iterator pIter = moduleMapFiles[MODULE_GLOBAL].begin();;

    if ( eModule != MODULE_GLOBAL )
    {
        while ( pIter != moduleMapFiles[MODULE_GLOBAL].end() )
        {
            int nId = pIter->first;
            CommandLabels& rCmdLabels = pIter->second;

            ResId aResId( nId, pResMgr );
            aResId.SetRT( RSC_SFX_SLOT_INFO );

            if ( pResMgr->IsAvailable( aResId ))
            {
                SfxSlotInfo aSlotInfo( aResId );
                OUString    aText( aSlotInfo.GetName() );
                if ( rCmdLabels.aLabels[nLangIndex].getLength() == 0 )
                    rCmdLabels.aLabels[nLangIndex] = aText;
            }
            ++pIter;
        }
    }

    pIter = moduleMapFiles[eModule].begin();
    while ( pIter != moduleMapFiles[eModule].end() )
    {
        int nId = pIter->first;
        CommandLabels& rCmdLabels = pIter->second;

        ResId aResId( nId, pResMgr );
        aResId.SetRT( RSC_SFX_SLOT_INFO );

        if ( pResMgr->IsAvailable( aResId ))
        {
            SfxSlotInfo aSlotInfo( aResId );
            OUString    aText( aSlotInfo.GetName() );
            if ( rCmdLabels.aLabels[nLangIndex].getLength() == 0 )
                rCmdLabels.aLabels[nLangIndex] = aText;
        }
        ++pIter;
    }

    return true;
}

bool ReadResourceFile( int nLangIndex, const OUString& aResourceDirURL, const OUString& aResourceFilePrefix, MODULES eModule, const OUString& aProjectName, bool bHasSlotInfos )
{
    OUString aSysDirPath;
    OString  aResFilePrefix = OUStringToOString( aResourceFilePrefix, RTL_TEXTENCODING_ASCII_US );

    osl::FileBase::getSystemPathFromFileURL( aResourceDirURL, aSysDirPath );

    String aSysDirPathStr( aSysDirPath );

//    String aLangString( String::CreateFromAscii( Language_Mapping[nLangIndex].pISO ));
//    LanguageType aLangtype = ConvertIsoStringToLanguage( aLangString );
    ::com::sun::star::lang::Locale aLocale;
    aLocale.Language = OUString::createFromAscii( Language_codes[nLangIndex].pLanguage );
    aLocale.Country = OUString::createFromAscii( Language_codes[nLangIndex].pCountry );
    ResMgr* pResMgr = ResMgr::CreateResMgr( aResFilePrefix.getStr(),
                                            aLocale,
                                            NULL,
                                            &aSysDirPathStr );
    if ( pResMgr )
    {
        int i = 0;
        while ( ResourceModule_Mapping[i].nResId > 0 )
        {
            if ( ResourceModule_Mapping[i].eBelongsTo == eModule )
            {
                ResId   aResId( ResourceModule_Mapping[i].nResId, pResMgr );
                aResId.SetRT( RSC_MENU );

                if ( pResMgr->IsAvailable( aResId ))
                {
                    MenuBar* pMenuBar = new MenuBar( aResId );
                    ReadMenuLabels( nLangIndex, (Menu *)pMenuBar, eModule, bHasSlotInfos, aResourceFilePrefix );
                    delete pMenuBar;
                }

                if ( bHasSlotInfos )
                    ReadSlotInfos( nLangIndex, pResMgr, eModule, aProjectName );
            }
            ++i;
        }

        i = 0;
        while ( AccResourceModule_Mapping[i].nResId > 0 )
        {
            if ( AccResourceModule_Mapping[i].eBelongsTo != eModule )
            {
                ++i;
                continue;
            }

            ResId   aAccResId( AccResourceModule_Mapping[i].nResId, pResMgr );
            aAccResId.SetRT( RSC_ACCEL );

            if (!pResMgr->IsAvailable( aAccResId ))
            {
                fprintf(stdout, ".. acc %d not found\n", AccResourceModule_Mapping[i].nResId);
                ++i;
                continue;
            }

            Accelerator* pAcc = new Accelerator( aAccResId );
            ReadAccelerators( nLangIndex, pResMgr, eModule, aProjectName, pAcc );
            delete pAcc;
            ++i;
        }

        delete pResMgr;
    }

    return true;
}

static const char ENCODED_AMPERSAND[]  = "&amp;";
static const char ENCODED_LESS[]       = "&lt;";
static const char ENCODED_GREATER[]    = "&gt;";
static const char ENCODED_QUOTE[]      = "&quot;";
static const char ENCODED_APOS[]       = "&apos;";

struct EncodeChars
{
    char  cChar;
    const char* pEncodedChars;
};

EncodeChars EncodeChar_Map[] =
{
    { '&', ENCODED_AMPERSAND    },
    { '<', ENCODED_LESS         },
    { '>', ENCODED_GREATER      },
    { '"', ENCODED_QUOTE        },
    { '\'', ENCODED_APOS        },
    { ' ', 0                    }
};

// Encodes a string to UTF-8 and uses "Built-in entity reference" to
// to escape character data that is not markup!
OString EncodeString( const OUString& aToEncodeStr )
{
    OString aString = OUStringToOString( aToEncodeStr, RTL_TEXTENCODING_UTF8 );

    int     i = 0;
    bool    bMustCopy( sal_False );
    while ( EncodeChar_Map[i].pEncodedChars != 0 )
    {
        OStringBuffer aBuf;
        bool bEncoded( false );

        sal_Int32 nCurIndex = 0;
        sal_Int32 nIndex    = 0;
        while ( nIndex < aString.getLength() )
        {
            nIndex = aString.indexOf( EncodeChar_Map[i].cChar, nIndex );
            if ( nIndex > 0 )
            {
                bEncoded = true;
                if ( nIndex > nCurIndex )
                    aBuf.append( aString.copy( nCurIndex, nIndex-nCurIndex ));
                aBuf.append( EncodeChar_Map[i].pEncodedChars );
                nCurIndex = nIndex+1;
            }
            else if ( nIndex == 0 )
            {
                bEncoded = true;
                aBuf.append( EncodeChar_Map[i].pEncodedChars );
                nCurIndex = nIndex+1;
            }
            else
            {
                if ( bEncoded && nCurIndex < aString.getLength() )
                    aBuf.append( aString.copy( nCurIndex ));
                break;
            }
            ++nIndex;
        }

        if ( bEncoded )
            aString = aBuf.makeStringAndClear();
        ++i;
    }

    return aString;
}

// Mark all commands which are popups
void FindAndMarkModulePopupMenuCmds()
{
    for ( int i = 0; i < int( MODULE_COUNT ); i++ )
    {
        CommandToLabelsMap::iterator pCmdIter = modulePopupMenusCmd[i].begin();
        while ( pCmdIter != modulePopupMenusCmd[i].end() )
        {
            CommandLabels& rCmdLabel = pCmdIter->second;
            if ( rCmdLabel.bPopupMenu )
            {
                CommandIDToLabelsMap::iterator pIDIter = moduleMapFiles[i].begin();
                while ( pIDIter != moduleMapFiles[i].end() )
                {
                    CommandLabels& rIDLabel = pIDIter->second;
                    if ( rIDLabel.aCommand.equals( rCmdLabel.aCommand ))
                    {
                        rIDLabel.bPopupMenu = true;
                        break;
                    }
                    ++pIDIter;
                }
            }

            ++pCmdIter;
        }
    }
}


bool FindAndMarkPopupMenuCmds( const OUString& aCommand, MODULES eExcludeModule )
{
    bool bFound( sal_False );

    for ( int i = 1; i < int( MODULE_COUNT ); i++ )
    {
        if ( eExcludeModule != MODULES( i ))
        {
            CommandToLabelsMap::iterator pCmdIter = modulePopupMenusCmd[i].find( aCommand );
            if ( pCmdIter != modulePopupMenusCmd[i].end() )
            {
                CommandLabels& rCmdLabel = pCmdIter->second;
                rCmdLabel.bWritten = true; // mark as written, this is now a global popup menu command
                bFound = true;
            }
        }
    }

    return bFound;
}

bool FindAndMoveGlobalPopupMenus()
{
    for ( int i = 1; i < int( MODULE_COUNT ); i++ )
    {
        CommandToLabelsMap::iterator pCmdIter = modulePopupMenusCmd[i].begin();
        while ( pCmdIter != modulePopupMenusCmd[i].end() )
        {
            CommandLabels& rCmdLabel = pCmdIter->second;
            if ( rCmdLabel.bPopupMenu )
            {
                bool bDuplicate = FindAndMarkPopupMenuCmds( rCmdLabel.aCommand, MODULES( i ));
                if ( bDuplicate )
                {
                    CommandToLabelsMap::const_iterator pIter = modulePopupMenusCmd[MODULE_GLOBAL].find( rCmdLabel.aCommand );
                    if ( pIter == modulePopupMenusCmd[MODULE_GLOBAL].end() )
                    {
                        // Put command into global ui command list
                        CommandLabels aCmdLabels;

                        aCmdLabels.nID = 0;
                        aCmdLabels.aCommand = rCmdLabel.aCommand;
                        aCmdLabels.bPopupMenu = true;
                        for ( int j = 0; j < NUM_LANGUAGES; j++ )
                            aCmdLabels.aLabels[j] = rCmdLabel.aLabels[j];
                        AddCommandLabelsToCommandHashMap( MODULE_GLOBAL, aCmdLabels );
                        rCmdLabel.bWritten = true;
                    }
                    else
                    {
                        rCmdLabel.bWritten = true;
                    }
                }
            }
            ++pCmdIter;
        }
    }

    return true;
}

static const char XMLStart[]        = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

bool WriteXMLFiles( const OUString& aOutputDirURL)
{
    static const char DocType[]                 = "<!DOCTYPE oor:component-data SYSTEM \"../../../../../component-update.dtd\">\n";
    static const char ComponentDataStart[]      = "<oor:component-data oor:name=\"";
    static const char ComponentDataStartEnd[]   = "\" oor:package=\"org.openoffice.Office.UI\" xmlns:oor=\"http://openoffice.org/2001/registry\" xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">\n";
    static const char GroupText[]               = "\t<node oor:name=\"UserInterface\">\n";
    static const char SetTextCmds[]             = "\t\t<node oor:name=\"Commands\">\n";
    static const char SetTextPopups[]           = "\t\t<node oor:name=\"Popups\">\n";
    static const char NodeStart[]               = "\t\t\t<node oor:name=\"";
    static const char ReplaceOp[]               = "\" oor:op=\"replace\">\n";
    static const char NodeEnd[]                 = "\t\t\t</node>\n";
    static const char PropLabelNodeStart[]      = "\t\t\t\t<prop oor:name=\"Label\" oor:type=\"xs:string\">\n";
    static const char PropNodeEnd[]             = "\t\t\t\t</prop>\n";
    static const char PropPropNodeStart[]       = "\t\t\t\t<prop oor:name=\"Properties\" oor:type=\"xs:int\">\n";
    static const char PropHelpIdStart[]         = "\t\t\t\t<prop oor:name=\"HelpId\" oor:type=\"xs:int\">\n";
    static const char ValueSimpleNodeStart[]    = "\t\t\t\t\t<value>";
    static const char ValueNodeStart[]          = "\t\t\t\t\t<value xml:lang=\"";
    static const char ValueNodeMid[]            = "\">";
    static const char ValueNodeEnd[]            = "</value>\n";
    static const char ValueNodeEmpty[]          = "\t\t\t\t\t<value/>\n";
    static const char SetTextEnd[]              = "\t\t</node>\n";
    static const char GroupTextEnd[]            = "\t</node>\n";
    static const char ComponentDataEnd[]        = "</oor:component-data>\n";

    // Search popup menu commands that can be moved to the global list as they are used in more than one project
    FindAndMoveGlobalPopupMenus();
    // Search and mark all commands which are popups
    FindAndMarkModulePopupMenuCmds();

    OUString aOutputDirectoryURL( aOutputDirURL );
    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
        aOutputDirectoryURL += OUString::createFromAscii( "/" );

    int i = 0;
    while ( ModuleToXML_Mapping[i] != 0 )
    {
        OUString aOutputFileURL( aOutputDirectoryURL );
        aOutputFileURL += OUString::createFromAscii( ModuleToXML_Mapping[i] );
        aOutputFileURL += OUString::createFromAscii( XMLFileExtension );

        osl::File aXMLFile( aOutputFileURL );

        osl::File::RC nRet = aXMLFile.open( OpenFlag_Create|OpenFlag_Write );
        if ( nRet == osl::File::E_EXIST )
        {
            nRet = aXMLFile.open( OpenFlag_Write );
            if ( nRet == osl::File::E_None )
                nRet = aXMLFile.setSize( 0 );
        }

        if ( nRet == osl::FileBase::E_None )
        {
            sal_uInt64 nWritten;

            aXMLFile.write( XMLStart, strlen( XMLStart ), nWritten );
            aXMLFile.write( DocType, strlen( DocType ), nWritten );
            aXMLFile.write( ComponentDataStart, strlen( ComponentDataStart ), nWritten );
            aXMLFile.write( ModuleToXML_Mapping[i], strlen( ModuleToXML_Mapping[i] ), nWritten );
            aXMLFile.write( ComponentDataStartEnd, strlen( ComponentDataStartEnd ), nWritten );
            aXMLFile.write( GroupText, strlen( GroupText ), nWritten );

            if (( moduleMapFiles[i].size() > 0      ) ||
                ( modulePopupMenusCmd[i].size() > 0 )    )
            {
                for ( int nSets = 0; nSets < 2; nSets++ )
                {
                    if ( nSets == 0 )
                        aXMLFile.write( SetTextCmds, strlen( SetTextCmds ), nWritten );
                    else
                        aXMLFile.write( SetTextPopups, strlen( SetTextPopups ), nWritten );

                    CommandIDToLabelsMap::iterator pIter = moduleMapFiles[i].begin();
                    while ( pIter != moduleMapFiles[i].end() )
                    {
                        CommandLabels& rCmdLabels = pIter->second;

                        if ( rCmdLabels.bWritten )
                        {
                            ++pIter;
                            continue;
                        }

                        CommandToLabelsMap::iterator pCmdIter = modulePopupMenusCmd[i].find( rCmdLabels.aCommand );
                        if ( pCmdIter != modulePopupMenusCmd[i].end() )
                        {
                            // Mark as written within the popup menus hash table
                            CommandLabels& rCmdLabel = pCmdIter->second;
                            rCmdLabel.bWritten = true;
                        }

                        // We have to save popups on the next iteration
                        if (( nSets == 0 ) && rCmdLabels.bPopupMenu )
                        {
                            ++pIter;
                            continue;
                        }

                        // Mark as written for next iteration
                        rCmdLabels.bWritten = true;

                        // we have to use UTF-8 as encoding
                        OString aCmd = OUStringToOString( rCmdLabels.aCommand, RTL_TEXTENCODING_UTF8 );

                        aXMLFile.write( NodeStart, strlen( NodeStart ), nWritten );
                        aXMLFile.write( aCmd.getStr(), aCmd.getLength(), nWritten );
                        aXMLFile.write( ReplaceOp, strlen( ReplaceOp ), nWritten );
                        aXMLFile.write( PropLabelNodeStart, strlen( PropLabelNodeStart ), nWritten );

                        sal_Bool bLabels( sal_False );
                        for ( int j = 0; j < NUM_LANGUAGES; j++ )
                        {
                            OString aLabel;
                            if ( rCmdLabels.aLabels[j].getLength() > 0 )
                            {
                                bLabels = sal_True;

                                // Encode label to the XML rules.
                                aLabel = EncodeString( rCmdLabels.aLabels[j] );
                            }
                            else
                            {
                                CommandToLabelsMap::const_iterator pIter = modulePopupMenusCmd[i].find( rCmdLabels.aCommand );
                                if ( pIter != modulePopupMenusCmd[i].end() )
                                {
                                    const CommandLabels& rCmdLabels = pIter->second;
                                    if ( rCmdLabels.aLabels[j].getLength() > 0 )
                                    {
                                        bLabels = sal_True;
                                        aLabel = EncodeString( rCmdLabels.aLabels[j] );
                                    }
                                }
                            }

                            if ( aLabel.getLength() > 0 )
                            {
                                OString mystr;
                                mystr += Language_codes[j].pLanguage;
                                if ( strlen(Language_codes[j].pCountry))
                                {
                                    mystr += "-";
                                    mystr += Language_codes[j].pCountry;
                                }

                                /*
                                char mystr[255];
                                strncat( mystr, Language_codes[j].pLanguage, 2);
                                if ( strlen(Language_codes[j].pCountry))
                                {
                                    strncat( mystr, "-", 1);
                                    strncat( mystr, Language_codes[j].pCountry, 2);
                                }
                                */
                                aXMLFile.write( ValueNodeStart, strlen( ValueNodeStart ), nWritten );
                                aXMLFile.write( mystr, strlen( mystr ), nWritten );
                                aXMLFile.write( ValueNodeMid, strlen( ValueNodeMid ), nWritten );
                                aXMLFile.write( aLabel.getStr(), aLabel.getLength(), nWritten );
                                aXMLFile.write( ValueNodeEnd, strlen( ValueNodeEnd ), nWritten );
                            }
                        }

                        if ( !bLabels )
                            aXMLFile.write( ValueNodeEmpty, strlen( ValueNodeEmpty ), nWritten );
                        aXMLFile.write( PropNodeEnd, strlen( PropNodeEnd ), nWritten );

                        char aBuf[16] = {0};

                        // write properties node which stores additional information about a command
                        if ( rCmdLabels.nProperties != 0 )
                        {
                            sprintf( aBuf, "%d", rCmdLabels.nProperties );
                            aXMLFile.write( PropPropNodeStart, strlen( PropPropNodeStart ), nWritten );
                            aXMLFile.write( ValueSimpleNodeStart, strlen( ValueSimpleNodeStart ), nWritten );
                            aXMLFile.write( aBuf, strlen( aBuf ), nWritten );
                            aXMLFile.write( ValueNodeEnd, strlen( ValueNodeEnd ), nWritten );
                            aXMLFile.write( PropNodeEnd, strlen( PropNodeEnd ), nWritten );
                        }

/*
                        // write help ID property node
                        sprintf( aBuf, "%d", rCmdLabels.nID );
                        aXMLFile.write( PropHelpIdStart, strlen( PropHelpIdStart ), nWritten );
                        aXMLFile.write( ValueSimpleNodeStart, strlen( ValueSimpleNodeStart ), nWritten );
                        aXMLFile.write( aBuf, strlen( aBuf ), nWritten );
                        aXMLFile.write( ValueNodeEnd, strlen( ValueNodeEnd ), nWritten );
                        aXMLFile.write( PropNodeEnd, strlen( PropNodeEnd ), nWritten );
*/
                        aXMLFile.write( NodeEnd, strlen( NodeEnd ), nWritten );
                        ++pIter;
                    }

                    sal_uInt32 nSize = modulePopupMenusCmd[i].size();
                    if ( nSize > 0 )
                    {
                        CommandToLabelsMap::iterator pIter = modulePopupMenusCmd[i].begin();
                        while ( pIter != modulePopupMenusCmd[i].end() )
                        {
                            CommandLabels& rCmdLabels = pIter->second;

                            if (( nSets == 0 ) && rCmdLabels.bPopupMenu )
                            {
                                ++pIter;
                                continue;
                            }

                            if ( !rCmdLabels.bWritten )
                            {
                                rCmdLabels.bWritten = true;
                                OString aCmd( OUStringToOString( rCmdLabels.aCommand, RTL_TEXTENCODING_UTF8 ));
                                aXMLFile.write( NodeStart, strlen( NodeStart ), nWritten );
                                aXMLFile.write( aCmd.getStr(), aCmd.getLength(), nWritten );
                                aXMLFile.write( ReplaceOp, strlen( ReplaceOp ), nWritten );
                                aXMLFile.write( PropLabelNodeStart, strlen( PropLabelNodeStart ), nWritten );

                                sal_Bool bLabels( sal_False );
                                for ( int k = 0; k < NUM_LANGUAGES; k++ )
                                {
                                    if ( rCmdLabels.aLabels[k].getLength() > 0 )
                                    {
                                        bLabels = sal_True;
                                        OString aLabel( OUStringToOString( rCmdLabels.aLabels[k], RTL_TEXTENCODING_UTF8 ));

                                        char mystr[255];
                                        strncat( mystr, Language_codes[k].pLanguage, 2);
                                        if ( strlen(Language_codes[k].pCountry))
                                        {
                                            strncat( mystr, "-", 1);
                                            strncat( mystr, Language_codes[k].pCountry, 2);
                                        }

                                        aXMLFile.write( ValueNodeStart, strlen( ValueNodeStart ), nWritten );
                                        aXMLFile.write( mystr, strlen( mystr ), nWritten );
                                        aXMLFile.write( ValueNodeMid, strlen( ValueNodeMid ), nWritten );
                                        aXMLFile.write( aLabel.getStr(), aLabel.getLength(), nWritten );
                                        aXMLFile.write( ValueNodeEnd, strlen( ValueNodeEnd ), nWritten );
                                    }
                                }

                                if ( !bLabels )
                                    aXMLFile.write( ValueNodeEmpty, strlen( ValueNodeEmpty ), nWritten );

                                aXMLFile.write( PropNodeEnd, strlen( PropNodeEnd ), nWritten );
                                aXMLFile.write( NodeEnd, strlen( NodeEnd ), nWritten );
                            }

                            ++pIter;
                        }
                    }

                    aXMLFile.write( SetTextEnd, strlen( SetTextEnd ), nWritten );
                }
            }
            else
            {
                // Write empty sets
                aXMLFile.write( SetTextCmds, strlen( SetTextCmds ), nWritten );
                aXMLFile.write( SetTextEnd, strlen( SetTextEnd ), nWritten );
                aXMLFile.write( SetTextPopups, strlen( SetTextPopups ), nWritten );
                aXMLFile.write( SetTextEnd, strlen( SetTextEnd ), nWritten );
            }

            aXMLFile.write( GroupTextEnd, strlen( GroupTextEnd ), nWritten );
            aXMLFile.write( ComponentDataEnd, strlen( ComponentDataEnd ), nWritten );

            aXMLFile.close();
            ++i;
        }
    }

    return true;
}

bool WriteLevel( osl::File& rFile, int nLevel )
{
    const char cTab[] = "\t";

    sal_uInt64 nWritten;
    for ( int i = 0; i < nLevel; i++ )
        rFile.write( cTab, strlen( cTab ), nWritten );

    return true;
}

bool WriteSeparator( osl::File& rFile, int nLevel )
{
    static const char MenuSeparator[] = "<menu:menuseparator/>\n";

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuSeparator, strlen( MenuSeparator ), nWritten );

    return true;
}

bool WriteMenuItem( osl::File& rFile, const OUString& aCmd, const OUString& aHelpID, int nLevel )
{
    static const char MenuItemStart[]   = "<menu:menuitem menu:id=\"";
    static const char MenuItemHelp[]    = "menu:helpid=\"";
    static const char MenuItemLabel[]   = "menu:label=\"";
    static const char MenuItemAttrEnd[] = "\" ";
    static const char MenuItemEnd[]     = "/>\n";

    OString aCmdStr = OUStringToOString( aCmd, RTL_TEXTENCODING_UTF8 );
    OString aHelpId = OUStringToOString( aHelpID, RTL_TEXTENCODING_UTF8 );

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuItemStart, strlen( MenuItemStart ), nWritten );
    rFile.write( aCmdStr.getStr(), aCmdStr.getLength(), nWritten );
    rFile.write( MenuItemAttrEnd, strlen( MenuItemAttrEnd ), nWritten );
    rFile.write( MenuItemHelp, strlen( MenuItemHelp ), nWritten );
    rFile.write( aHelpId.getStr(), aHelpId.getLength(), nWritten );
    rFile.write( MenuItemAttrEnd, strlen( MenuItemAttrEnd ), nWritten );
    rFile.write( MenuItemLabel, strlen( MenuItemLabel ), nWritten );
    rFile.write( MenuItemAttrEnd, strlen( MenuItemAttrEnd ), nWritten );
    rFile.write( MenuItemEnd, strlen( MenuItemEnd ), nWritten );

    return true;
}

bool WritePopupMenuEmpty( osl::File& rFile, int nLevel )
{
    static const char MenuPopupStart[]  = "<menu:menupopup/>\n";

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuPopupStart, strlen( MenuPopupStart ), nWritten );

    return true;
}

bool WritePopupMenuStart( osl::File& rFile, int nLevel )
{
    static const char MenuPopupStart[]  = "<menu:menupopup>\n";

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuPopupStart, strlen( MenuPopupStart ), nWritten );

    return true;
}

bool WritePopupMenuEnd( osl::File& rFile, int nLevel )
{
    static const char MenuPopupEnd[]    = "</menu:menupopup>\n";

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuPopupEnd, strlen( MenuPopupEnd ), nWritten );

    return true;
}

bool WriteMenuStart( osl::File& rFile, const OUString& aCmd, int nLevel )
{
    static const char MenuStart[]       = "<menu:menu menu:id=\"";
    static const char MenuEnd[]         = ">\n";
    static const char MenuAttrEnd[]     = "\" ";
    static const char MenuLabel[]       = "menu:label=\"";

    OString aCmdStr = OUStringToOString( aCmd, RTL_TEXTENCODING_UTF8 );

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuStart, strlen( MenuStart ), nWritten );
    rFile.write( aCmdStr.getStr(), aCmdStr.getLength(), nWritten );
    rFile.write( MenuAttrEnd, strlen( MenuAttrEnd ), nWritten );
    rFile.write( MenuLabel, strlen( MenuLabel ), nWritten );
    rFile.write( MenuAttrEnd, strlen( MenuAttrEnd ), nWritten );
    rFile.write( MenuEnd, strlen( MenuEnd ), nWritten );

    return true;
}

bool WriteMenuEnd( osl::File& rFile, int nLevel )
{
    static const char MenuEnd[]         = "</menu:menu>\n";

    sal_uInt64 nWritten;
    WriteLevel( rFile, nLevel );
    rFile.write( MenuEnd, strlen( MenuEnd ), nWritten );

    return true;
}

bool WriteAccXML( osl::File& rFile, Accelerator* pAcc, MODULES eModule, int nLevel )
{
   ::rtl::OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

    for ( unsigned short n = 0; n < pAcc->GetItemCount(); n++ )
    {
        unsigned short nId = pAcc->GetItemId( n );
    }

    return true;
}

bool WriteMenuBarXML( osl::File& rFile, Menu* pMenu, MODULES eModule, int nLevel )
{
   OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

    for ( unsigned short n = 0; n < pMenu->GetItemCount(); n++ )
    {
        unsigned short nId = pMenu->GetItemId( n );
        if ( pMenu->GetItemType( n ) != MENUITEM_SEPARATOR )
        {
            OUString aLabel( pMenu->GetItemText( nId ));
            OUString aCommand( pMenu->GetItemCommand( nId ));
            bool     bHasUnoCommand( aCommand.getLength() > 0 && ( aCommand.indexOf( aCmd ) == 0 ));

            if ( pMenu->GetPopupMenu( nId ) != 0 )
            {
                if ( !bHasUnoCommand )
                {
                    CommandLabels* pCmdLabels = 0;
                    pCmdLabels = RetrieveCommandLabelsFromID( nId, eModule );
                    if ( !pCmdLabels )
                        pCmdLabels = RetrieveCommandLabelsFromCommand( aCommand, eModule );

                    if ( pCmdLabels )
                        aCommand = pCmdLabels->aCommand;
                    else if ( aCommand.getLength() == 0 )
                        aCommand = OUString( RTL_CONSTASCII_USTRINGPARAM( "???" ));
                }

                WriteMenuStart( rFile, aCommand, nLevel );
                WritePopupMenuStart( rFile, nLevel+1 );
                WriteMenuBarXML( rFile, pMenu->GetPopupMenu( nId ), eModule, nLevel+2 );
                WritePopupMenuEnd( rFile, nLevel+1 );
                WriteMenuEnd( rFile, nLevel );
            }
            else if ( pMenu->IsMenuBar() )
            {
                // write empty popup menu
                WriteMenuStart( rFile, aCommand, nLevel );
                WritePopupMenuEmpty( rFile, nLevel+1 );
                WriteMenuEnd( rFile, nLevel );
            }
            else
            {
                CommandLabels* pCmdLabels = RetrieveCommandLabelsFromID( nId, eModule );
                if ( !pCmdLabels )
                    pCmdLabels = RetrieveCommandLabelsFromCommand( aCommand, eModule );

                if ( pCmdLabels )
                    aCommand = pCmdLabels->aCommand;
                else
                    aCommand = OUString( RTL_CONSTASCII_USTRINGPARAM( "???" ));

                OUString aHelpId = OUString::valueOf( sal_Int32( pMenu->GetHelpId( nId )));
                WriteMenuItem( rFile, aCommand, aHelpId, nLevel );
            }
        }
        else
        {
            WriteSeparator( rFile, nLevel );
        }
    }

    return true;
}

bool impl_isAccListDuplicate(const KeyCommandInfoList& rAccsReference,
                             const KeyCommandInfoList& rAccs         )
{
    KeyCommandInfoList::const_iterator pIt1;
    for (  pIt1  = rAccsReference.begin();
           pIt1 != rAccsReference.end()  ;
         ++pIt1                          )
    {
        KeyCommandInfoList::const_iterator pIt2 = ::std::find(rAccs.begin(), rAccs.end(), *pIt1);
        if (pIt2 == rAccs.end())
            return false;
    }
    return true;
}

bool ReadResourceWriteAcceleratorXMLLang( const ::rtl::OUString& aOutDirURL,
                                      const ::rtl::OUString& aResourceDirURL,
                                      const ::rtl::OUString& aResourceFilePrefix,
                                            MODULES          eModule,
                                      const ::rtl::OUString& aProjectName,
                                      int nLanguage)
{
    static const char AccStart[]    = "<accel:acceleratorlist xmlns:accel=\"http://openoffice.org/2001/accel\" xmlns:xlink=\"http://www.w3.org/1999/xlink\">\n";
    static const char AccEnd[]      = "</accel:acceleratorlist>\n";

    Locale2KeyCommandInfoList& rLocale        = moduleAccs[eModule];
    KeyCommandInfoList&        rAccs          = rLocale[nLanguage]; // current language
    KeyCommandInfoList&        rAccsReference = rLocale[1];         // en-US!

    // dont write empty files :-)
    if (rAccs.size()<1)
        return true;

    ::rtl::OUString sLanguage = ::rtl::OUString::createFromAscii(Language_codes[nLanguage].pLanguage);
    ::rtl::OUString sCountry  = ::rtl::OUString::createFromAscii(Language_codes[nLanguage].pCountry);

    // dont generate duplicate xml files, if current language isnt different from en-US!
    // Of course we have to write en-US everytimes :-)
    if (nLanguage != 1 && impl_isAccListDuplicate(rAccsReference, rAccs))
    {
        fprintf(stdout, "ignore acc list [mod=%d, lang=%s-%s] ... seems to be duplicate against en-US=1\n", eModule, ::rtl::OUStringToOString(sLanguage, RTL_TEXTENCODING_UTF8).getStr(), ::rtl::OUStringToOString(sCountry, RTL_TEXTENCODING_UTF8).getStr());
        return true;
    }

    fprintf(stdout, "write acc list [mod=%d, lang=%s-%s] ... seems to be duplicate against en-US=1\n", eModule, ::rtl::OUStringToOString(sLanguage, RTL_TEXTENCODING_UTF8).getStr(), ::rtl::OUStringToOString(sCountry, RTL_TEXTENCODING_UTF8).getStr());

    ::rtl::OUString aSysDirPath;
    ::rtl::OString  aResFilePrefix = ::rtl::OUStringToOString( aResourceFilePrefix, RTL_TEXTENCODING_ASCII_US );

    osl::FileBase::getSystemPathFromFileURL( aResourceDirURL, aSysDirPath );

    String aSysDirPathStr( aSysDirPath );

    ::rtl::OUString aOutputDirectoryURL( aOutDirURL );
    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
        aOutputDirectoryURL += ::rtl::OUString::createFromAscii( "/" );

    int i = 0;
    while ( AccResourceModule_Mapping[i].nResId > 0 )
    {
        if ( AccResourceModule_Mapping[i].eBelongsTo == eModule )
            break;
        ++i;
    }

    ::rtl::OUString aOutputFileURL( aOutputDirectoryURL );
    aOutputFileURL += ::rtl::OUString::createFromAscii( AccResourceModule_Mapping[i].pXMLPrefix );
    aOutputFileURL += ::rtl::OUString::createFromAscii( "_(" );
    aOutputFileURL += sLanguage;
    aOutputFileURL += ::rtl::OUString::createFromAscii( "-" );
    aOutputFileURL += sCountry;
    aOutputFileURL += ::rtl::OUString::createFromAscii( ")_accelerator.xml" );

    osl::File aXMLFile( aOutputFileURL );
    osl::File::RC nRet = aXMLFile.open( OpenFlag_Write );
    if ( nRet != osl::File::E_None )
    {
        nRet = aXMLFile.open( OpenFlag_Write | OpenFlag_Create );
        if ( nRet != osl::File::E_None )
        {
            fprintf(stdout, "ERROR:\tcould not create acc.xml \"%s\" [ret=%d]\n", ::rtl::OUStringToOString( aOutputFileURL, RTL_TEXTENCODING_ASCII_US ).getStr(), nRet);
            return false;
        }
    }

    sal_uInt64 nWritten;

    aXMLFile.write( XMLStart, strlen( XMLStart ), nWritten );
    aXMLFile.write( AccStart, strlen( AccStart ), nWritten );

    ::rtl::OUStringBuffer sAccBuf(10000);
    KeyCommandInfoList::const_iterator pIt;
    for (  pIt  = rAccs.begin();
           pIt != rAccs.end()  ;
         ++pIt                 )
    {
        const KeyCommandInfo& aInfo = *pIt;

        sAccBuf.appendAscii("\t<accel:item accel:code=\"");
        sAccBuf.append(aInfo.sKey);
        sAccBuf.appendAscii("\"");

        BOOL   bShift = ((aInfo.aAWTKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT );
        BOOL   bMod1  = ((aInfo.aAWTKey.Modifiers & css::awt::KeyModifier::MOD1 ) == css::awt::KeyModifier::MOD1  );
        BOOL   bMod2  = ((aInfo.aAWTKey.Modifiers & css::awt::KeyModifier::MOD2 ) == css::awt::KeyModifier::MOD2  );

        if (bShift)
            sAccBuf.appendAscii(" accel:shift=\"true\"");
        if (bMod1)
            sAccBuf.appendAscii(" accel:mod1=\"true\"");
        if (bMod2)
            sAccBuf.appendAscii(" accel:mod2=\"true\"");

        sAccBuf.appendAscii(" xlink:href=\"");
        sAccBuf.append(aInfo.sCommand);
        sAccBuf.appendAscii("\"/>\n");
    }

    ::rtl::OString sAcc = ::rtl::OUStringToOString(sAccBuf.makeStringAndClear(), RTL_TEXTENCODING_UTF8);

    aXMLFile.write( sAcc.getStr(), sAcc.getLength(), nWritten );
    aXMLFile.write( AccEnd, strlen( AccEnd ), nWritten );
    aXMLFile.close();

    return true;
}

bool ReadResourceWriteAcceleratorXML( const ::rtl::OUString& aOutDirURL,
                                      const ::rtl::OUString& aResourceDirURL,
                                      const ::rtl::OUString& aResourceFilePrefix,
                                            MODULES          eModule,
                                      const ::rtl::OUString& aProjectName)
{
    for ( int k = 0; k < NUM_LANGUAGES; k++ )
    {
        if (!ReadResourceWriteAcceleratorXMLLang(aOutDirURL, aResourceDirURL, aResourceFilePrefix, eModule, aProjectName, k))
            return false;
    }
    return true;
}

bool ReadResourceWriteMenuBarXML( const OUString& aOutDirURL,
                                  const OUString& aResourceDirURL,
                                  const OUString& aResourceFilePrefix,
                                  MODULES eModule,
                                  const OUString& aProjectName )
{
    static const char MenuBarStart[]    = "<menu:menubar xmlns:menu=\"http://openoffice.org/2001/menu\" menu:id=\"menubar\">\n";
    static const char MenuBarEnd[]      = "</menu:menubar>";

    OUString aSysDirPath;
    OString  aResFilePrefix = OUStringToOString( aResourceFilePrefix, RTL_TEXTENCODING_ASCII_US );

    osl::FileBase::getSystemPathFromFileURL( aResourceDirURL, aSysDirPath );

    String aSysDirPathStr( aSysDirPath );

//    String aLangString( String::CreateFromAscii( Language_Mapping[0].pISO ));
//    LanguageType aLangtype = ConvertIsoStringToLanguage( aLangString );
    ::com::sun::star::lang::Locale aLocale;
    aLocale.Language = OUString::createFromAscii( Language_codes[1].pLanguage );
    aLocale.Country = OUString::createFromAscii( Language_codes[1].pCountry );
    ResMgr* pResMgr = ResMgr::CreateResMgr( aResFilePrefix.getStr(),
                                            aLocale,
                                            NULL,
                                            &aSysDirPathStr );
    if ( pResMgr )
    {
        OUString aMenuBar( RTL_CONSTASCII_USTRINGPARAM( "menubar" ));
        int i = 0;
        while ( ResourceModule_Mapping[i].nResId > 0 )
        {
            if ( ResourceModule_Mapping[i].eBelongsTo == eModule )
            {
                ResId   aResId( ResourceModule_Mapping[i].nResId, pResMgr );
                aResId.SetRT( RSC_MENU );

                if ( pResMgr->IsAvailable( aResId ))
                {
                    OUString aOutputDirectoryURL( aOutDirURL );
                    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
                        aOutputDirectoryURL += OUString::createFromAscii( "/" );

                    OUString aOutputFileURL( aOutputDirectoryURL );
                    aOutputFileURL += OUString::createFromAscii( ResourceModule_Mapping[i].pXMLPrefix );
                    aOutputFileURL += aMenuBar;
                    aOutputFileURL += OUString::createFromAscii( ".xml" );

                    osl::File aXMLFile( aOutputFileURL );
                    osl::File::RC nRet = aXMLFile.open( OpenFlag_Create|OpenFlag_Write );
                    if ( nRet == osl::File::E_EXIST )
                    {
                        nRet = aXMLFile.open( OpenFlag_Write );
                        if ( nRet == osl::File::E_None )
                            nRet = aXMLFile.setSize( 0 );
                    }

                    if ( nRet == osl::FileBase::E_None )
                    {
                        sal_uInt64 nWritten;

                        aXMLFile.write( XMLStart, strlen( XMLStart ), nWritten );
                        aXMLFile.write( MenuBarStart, strlen( MenuBarStart ), nWritten );

                        MenuBar* pMenuBar = new MenuBar( aResId );
                        WriteMenuBarXML( aXMLFile, (Menu *)pMenuBar, eModule, 1 );
                        delete pMenuBar;

                        aXMLFile.write( MenuBarEnd, strlen( MenuBarEnd ), nWritten );
                        aXMLFile.close();
                    }
                }
            }
            ++i;
        }

        delete pResMgr;
    }

    return true;
}

bool WriteToolBarSeparator( osl::File& rFile )
{
    static const char ToolBarSeparator[] = " <toolbar:toolbarseparator/>\n";

    sal_uInt64 nWritten;
    rFile.write( ToolBarSeparator, strlen( ToolBarSeparator ), nWritten );

    return true;
}

bool WriteToolBarSpace( osl::File& rFile )
{
    static const char ToolBarSpace[] = " <toolbar:toolbarspace/>\n";

    sal_uInt64 nWritten;
    rFile.write( ToolBarSpace, strlen( ToolBarSpace ), nWritten );

    return true;
}

bool WriteToolBarBreak( osl::File& rFile )
{
    static const char ToolBarBlank[] = " <toolbar:toolbarbreak/>\n";

    sal_uInt64 nWritten;
    rFile.write( ToolBarBlank, strlen( ToolBarBlank ), nWritten );

    return true;
}

bool WriteToolBarItem( osl::File& rFile, const OUString& aCmd, const OUString& aHelpID, ToolBoxItemBits nStyle, bool bVisible )
{
    static const char ToolBarItemStart[]                = " <toolbar:toolbaritem xlink:href=\"";
    static const char ToolBarItemLabel[]                = "toolbar:text=\"";
    static const char ToolBarItemHelp[]                 = "toolbar:helpid=\"helpid:";
    static const char ToolBarItemStyle[]                = "toolbar:style=\"";
    static const char ToolBarItemVisible[]              = "toolbar:visible=\"false";
    static const char ToolBarItemAttrEnd[]              = "\" ";
    static const char ToolBarItemEnd[]                  = "/>\n";
    static const char ATTRIBUTE_ITEMSTYLE_RADIO[]       = "radio";
    static const char ATTRIBUTE_ITEMSTYLE_AUTO[]        = "auto";
    static const char ATTRIBUTE_ITEMSTYLE_LEFT[]        = "left";
    static const char ATTRIBUTE_ITEMSTYLE_AUTOSIZE[]    = "autosize";
    static const char ATTRIBUTE_ITEMSTYLE_DROPDOWN[]    = "dropdown";
    static const char ATTRIBUTE_ITEMSTYLE_REPEAT[]      = "repeat";

    OString aCmdStr = OUStringToOString( aCmd, RTL_TEXTENCODING_UTF8 );
    OString aHelpId = OUStringToOString( aHelpID, RTL_TEXTENCODING_UTF8 );

    sal_uInt64 nWritten;
    rFile.write( ToolBarItemStart, strlen( ToolBarItemStart ), nWritten );
    rFile.write( aCmdStr.getStr(), aCmdStr.getLength(), nWritten );
    rFile.write( ToolBarItemAttrEnd, strlen( ToolBarItemAttrEnd ), nWritten );
    rFile.write( ToolBarItemHelp, strlen( ToolBarItemHelp ), nWritten );
    rFile.write( aHelpId.getStr(), aHelpId.getLength(), nWritten );
    rFile.write( ToolBarItemAttrEnd, strlen( ToolBarItemAttrEnd ), nWritten );
    rFile.write( ToolBarItemLabel, strlen( ToolBarItemLabel ), nWritten );
    rFile.write( ToolBarItemAttrEnd, strlen( ToolBarItemAttrEnd ), nWritten );
    if ( nStyle != 0 )
    {
        rFile.write( ToolBarItemStyle, strlen( ToolBarItemStyle ), nWritten );
        sal_Bool bBitSet( sal_False );
        if ( nStyle & TIB_RADIOCHECK )
        {
            rFile.write( ATTRIBUTE_ITEMSTYLE_RADIO, strlen( ATTRIBUTE_ITEMSTYLE_RADIO ), nWritten );
            bBitSet = sal_True;
        }
        if ( nStyle & TIB_AUTOCHECK )
        {
            if ( bBitSet )
                rFile.write( " ", 1, nWritten );
            rFile.write( ATTRIBUTE_ITEMSTYLE_AUTO, strlen( ATTRIBUTE_ITEMSTYLE_AUTO ), nWritten );
            bBitSet = sal_True;
        }
        if ( nStyle & TIB_LEFT )
        {
            if ( bBitSet )
                rFile.write( " ", 1, nWritten );
            rFile.write( ATTRIBUTE_ITEMSTYLE_LEFT, strlen( ATTRIBUTE_ITEMSTYLE_LEFT ), nWritten );
            bBitSet = sal_True;
        }
        if ( nStyle & TIB_AUTOSIZE )
        {
            if ( bBitSet )
                rFile.write( " ", 1, nWritten );
            rFile.write( ATTRIBUTE_ITEMSTYLE_AUTOSIZE, strlen( ATTRIBUTE_ITEMSTYLE_AUTOSIZE ), nWritten );
            bBitSet = sal_True;
        }
        if ( nStyle & TIB_DROPDOWN )
        {
            if ( bBitSet )
                rFile.write( " ", 1, nWritten );
            rFile.write( ATTRIBUTE_ITEMSTYLE_DROPDOWN, strlen( ATTRIBUTE_ITEMSTYLE_DROPDOWN ), nWritten );
            bBitSet = sal_True;
        }
        if ( nStyle & TIB_REPEAT )
        {
            if ( bBitSet )
                rFile.write( " ", 1, nWritten );
            rFile.write( ATTRIBUTE_ITEMSTYLE_REPEAT, strlen( ATTRIBUTE_ITEMSTYLE_REPEAT ), nWritten );
        }
        rFile.write( ToolBarItemAttrEnd, strlen( ToolBarItemAttrEnd ), nWritten );
    }
    if ( !bVisible )
    {
        rFile.write( ToolBarItemVisible, strlen( ToolBarItemVisible ), nWritten );
        rFile.write( ToolBarItemAttrEnd, strlen( ToolBarItemAttrEnd ), nWritten );
    }

    rFile.write( ToolBarItemEnd, strlen( ToolBarItemEnd ), nWritten );

    return true;
}

bool WriteToolBarXML( osl::File& rFile, ToolBox* pToolBar, MODULES eModule )
{
   OUString aCmd( RTL_CONSTASCII_USTRINGPARAM( ".uno:" ));

    for ( unsigned short n = 0; n < pToolBar->GetItemCount(); n++ )
    {
        unsigned short  nId = pToolBar->GetItemId( n );
        ToolBoxItemType eType = pToolBar->GetItemType( n );

        if ( eType == TOOLBOXITEM_BUTTON )
        {
            OUString        aLabel( pToolBar->GetItemText( nId ));
            OUString        aCommand( pToolBar->GetItemCommand( nId ));
            bool            bHasUnoCommand( aCommand.getLength() > 0 && ( aCommand.indexOf( aCmd ) == 0 ));
            bool            bVisible( pToolBar->IsItemVisible( nId ));
            ToolBoxItemBits nItemBits( pToolBar->GetItemBits( nId ));

            CommandLabels* pCmdLabels = RetrieveCommandLabelsFromID( nId, eModule );
            if ( !pCmdLabels )
                pCmdLabels = RetrieveCommandLabelsFromCommand( aCommand, eModule );

            if ( pCmdLabels )
                aCommand = pCmdLabels->aCommand;
            else
                aCommand = OUString( RTL_CONSTASCII_USTRINGPARAM( "???" ));

            sal_uInt32 nHelpId = pToolBar->GetHelpId( nId );
            if ( nHelpId == 0 )
                nHelpId = nId;

            OUString aHelpId = OUString::valueOf( sal_Int32( nHelpId ));
            WriteToolBarItem( rFile, aCommand, aHelpId, nItemBits, bVisible );
        }
        else if ( eType == TOOLBOXITEM_SEPARATOR )
        {
            WriteToolBarSeparator( rFile );
        }
        else if ( eType == TOOLBOXITEM_SPACE )
        {
            WriteToolBarSpace( rFile );
        }
        else if ( eType == TOOLBOXITEM_BREAK )
        {
            WriteToolBarBreak( rFile );
        }
    }

    return true;
}

bool ReadResourceWriteToolBarXML( const OUString& aOutDirURL,
                                  const OUString& aResourceDirURL,
                                  const OUString& aResourceFilePrefix,
                                  MODULES eModule,
                                  const OUString& aProjectName )
{
    static const char ToolBarDocType[]  = "<!DOCTYPE toolbar:toolbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"toolbar.dtd\">\n";
    static const char ToolBarStart[]    = "<toolbar:toolbar xmlns:toolbar=\"http://openoffice.org/2001/toolbar\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" toolbar:id=\"toolbar\">\n";
    static const char ToolBarEnd[]      = "</toolbar:toolbar>";

    OUString aSysDirPath;
    OString  aResFilePrefix = OUStringToOString( aResourceFilePrefix, RTL_TEXTENCODING_ASCII_US );

    osl::FileBase::getSystemPathFromFileURL( aResourceDirURL, aSysDirPath );

    String aSysDirPathStr( aSysDirPath );

    ::com::sun::star::lang::Locale aLocale;
    aLocale.Language = OUString::createFromAscii( Language_codes[0].pLanguage );
    aLocale.Country = OUString::createFromAscii( Language_codes[0].pCountry );
    ResMgr* pResMgr = ResMgr::CreateResMgr( aResFilePrefix.getStr(),
                                            aLocale,
                                            NULL,
                                            &aSysDirPathStr );

    WorkWindow* pWindow = new WorkWindow( NULL, WB_APP | WB_STDWORK | WB_HIDE );
    if ( pResMgr )
    {
        int i = 0;
        while ( TBResourceModule_Mapping[i].nResId > 0 )
        {
            if ( TBResourceModule_Mapping[i].eBelongsTo == eModule )
            {
                ResId   aResId( TBResourceModule_Mapping[i].nResId, pResMgr );
                aResId.SetRT( RSC_TOOLBOX );

                if ( pResMgr->IsAvailable( aResId ))
                {
                    OUString aOutputDirectoryURL( aOutDirURL );
                    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
                        aOutputDirectoryURL += OUString::createFromAscii( "/" );

                    OUString aOutputFileURL( aOutputDirectoryURL );
                    aOutputFileURL += OUString::createFromAscii( TBResourceModule_Mapping[i].pXMLPrefix );
                    aOutputFileURL += OUString::createFromAscii( ".xml" );

                    osl::File aXMLFile( aOutputFileURL );
                    osl::File::RC nRet = aXMLFile.open( OpenFlag_Create|OpenFlag_Write );
                    if ( nRet == osl::File::E_EXIST )
                    {
                        nRet = aXMLFile.open( OpenFlag_Write );
                        if ( nRet == osl::File::E_None )
                            nRet = aXMLFile.setSize( 0 );
                    }

                    if ( nRet == osl::FileBase::E_None )
                    {
                        sal_uInt64 nWritten;

                        aXMLFile.write( XMLStart, strlen( XMLStart ), nWritten );
                        aXMLFile.write( ToolBarDocType, strlen( ToolBarDocType ), nWritten );
                        aXMLFile.write( ToolBarStart, strlen( ToolBarStart ), nWritten );

                        ToolBox* pToolBox = new ToolBox( pWindow, aResId );
                        WriteToolBarXML( aXMLFile, (ToolBox *)pToolBox, eModule );
                        delete pToolBox;

                        aXMLFile.write( ToolBarEnd, strlen( ToolBarEnd ), nWritten );
                        aXMLFile.close();

                        TBResourceModule_Mapping[i].bWritten = true;
                    }
                }
            }
            ++i;
        }

        delete pResMgr;
    }

    delete pWindow;

    return true;
}

class MyFloatingWindow : public FloatingWindow
{
    public:
        MyFloatingWindow( Window* pParent, const ResId& rResId, const ResId& rSubTbxResId );
        ~MyFloatingWindow()
        {
            delete pMyToolBox;
        }

        ToolBox* GetSubToolBox() { return pMyToolBox; }

    private:
        ToolBox* pMyToolBox;
};

MyFloatingWindow::MyFloatingWindow( Window* pParent, const ResId& rResId, const ResId& rSubTbxResId ) :
    FloatingWindow( pParent, rResId )
{
    pMyToolBox = new ToolBox( this, rSubTbxResId );

    FreeResource();
}


bool ReadResourceWriteSubToolBarXML( const OUString& aOutDirURL,
                                     const OUString& aResourceDirURL,
                                     const OUString& aResourceFilePrefix,
                                     MODULES eModule,
                                     const OUString& aProjectName )
{
    static const char ToolBarDocType[]  = "<!DOCTYPE toolbar:toolbar PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"toolbar.dtd\">\n";
    static const char ToolBarStart[]    = "<toolbar:toolbar xmlns:toolbar=\"http://openoffice.org/2001/toolbar\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" toolbar:id=\"toolbar\">\n";
    static const char ToolBarEnd[]      = "</toolbar:toolbar>";

    OUString aSysDirPath;
    OString  aResFilePrefix = OUStringToOString( aResourceFilePrefix, RTL_TEXTENCODING_ASCII_US );

    osl::FileBase::getSystemPathFromFileURL( aResourceDirURL, aSysDirPath );

    String aSysDirPathStr( aSysDirPath );

    ::com::sun::star::lang::Locale aLocale;
    aLocale.Language = OUString::createFromAscii( Language_codes[0].pLanguage );
    aLocale.Country = OUString::createFromAscii( Language_codes[0].pCountry );
    ResMgr* pResMgr = ResMgr::CreateResMgr( aResFilePrefix.getStr(),
                                            aLocale,
                                            NULL,
                                            &aSysDirPathStr );

    WorkWindow* pWindow = new WorkWindow( NULL, WB_APP | WB_STDWORK | WB_HIDE );
    if ( pResMgr )
    {
        int i = 0;
        while ( TBSubResourceModule_Mapping[i].nParentResId > 0 )
        {
            if ( TBSubResourceModule_Mapping[i].eBelongsTo == eModule )
            {
                ResId   aParentResId( TBSubResourceModule_Mapping[i].nParentResId, pResMgr );
                ResId   aResId( TBSubResourceModule_Mapping[i].nResId, pResMgr );
                MyFloatingWindow* pFloatWin( 0 );

                aResId.SetRT( RSC_TOOLBOX );
                aParentResId.SetRT( RSC_FLOATINGWINDOW );

                if ( pResMgr->IsAvailable( aParentResId ))
                {
                    pFloatWin = new MyFloatingWindow( pWindow, aParentResId, aResId );

                    OUString aOutputDirectoryURL( aOutDirURL );
                    if ( aOutputDirectoryURL.getLength() > 0 && aOutputDirectoryURL[aOutputDirectoryURL.getLength()-1] != '/' )
                        aOutputDirectoryURL += OUString::createFromAscii( "/" );

                    OUString aOutputFileURL( aOutputDirectoryURL );
                    aOutputFileURL += OUString::createFromAscii( TBSubResourceModule_Mapping[i].pXMLPrefix );
                    aOutputFileURL += OUString::createFromAscii( ".xml" );

                    osl::File aXMLFile( aOutputFileURL );
                    osl::File::RC nRet = aXMLFile.open( OpenFlag_Create|OpenFlag_Write );
                    if ( nRet == osl::File::E_EXIST )
                    {
                        nRet = aXMLFile.open( OpenFlag_Write );
                        if ( nRet == osl::File::E_None )
                            nRet = aXMLFile.setSize( 0 );
                    }

                    if ( nRet == osl::FileBase::E_None )
                    {
                        sal_uInt64 nWritten;

                        aXMLFile.write( XMLStart, strlen( XMLStart ), nWritten );
                        aXMLFile.write( ToolBarDocType, strlen( ToolBarDocType ), nWritten );
                        aXMLFile.write( ToolBarStart, strlen( ToolBarStart ), nWritten );

                        ToolBox* pToolBox = pFloatWin->GetSubToolBox();
                        if ( pToolBox )
                            WriteToolBarXML( aXMLFile, (ToolBox *)pToolBox, eModule );

                        aXMLFile.write( ToolBarEnd, strlen( ToolBarEnd ), nWritten );
                        aXMLFile.close();

                        TBSubResourceModule_Mapping[i].bWritten = true;
                    }

                    if ( pFloatWin )
                    {
                        delete pFloatWin;
                        pFloatWin = 0;
                    }
                }
            }
            ++i;
        }

        delete pResMgr;
    }

    delete pWindow;

    return true;
}

bool Convert( sal_Bool        bUseProduct,
              const OUString& aUseRes,
              const OUString& rVersion,
              const OUString& rOutputDirName,
              const OUString& rPlatformName,
              const std::vector< OUString >& rInDirVector,
              const OUString& rErrOutputFileName )
{
    OUString aWorkDir;

    osl_getProcessWorkingDir( &aWorkDir.pData );

    // Try to find xx*.csv file and put all commands into hash table
    for ( int i = 0; i < (int)rInDirVector.size(); i++ )
    {
        OUString aAbsInDirURL;
        OUString aInDirURL;
        OUString aInDir( rInDirVector[i] );

        osl::FileBase::getFileURLFromSystemPath( aInDir, aInDirURL );
        osl::FileBase::getAbsoluteFileURL( aWorkDir, aInDirURL, aAbsInDirURL );
        osl::Directory aDir( aAbsInDirURL );
        if ( aDir.open() == osl::FileBase::E_None )
        {
            osl::DirectoryItem aItem;
            while ( aDir.getNextItem( aItem ) == osl::FileBase::E_None )
            {
                osl::FileStatus aFileStatus( FileStatusMask_FileName );

                aItem.getFileStatus( aFileStatus );

                int j=0;
                OUString aFileName = aFileStatus.getFileName();

                while ( ProjectModule_Mapping[j].pProjectFolder != 0 &&
                        ProjectModule_Mapping[j].bSlotCommands == true )
                {
                    if ( aFileName.equalsAscii( ProjectModule_Mapping[j].pProjectFolder ))
                    {
                        OUStringBuffer aBuf( aAbsInDirURL );

                        aBuf.appendAscii( "/" );
                        aBuf.append( aFileStatus.getFileName() );
                        aBuf.appendAscii( "/" );
                        aBuf.append( rPlatformName );
                        if ( bUseProduct )
                            aBuf.appendAscii( ".pro" );
                        aBuf.appendAscii( "/misc/xx" );
                        aBuf.appendAscii( ProjectModule_Mapping[j].pCSVPrefix );
                        aBuf.appendAscii( ".csv" );

                        OUString aCSVFileURL( aBuf.makeStringAndClear() );
                        ReadCSVFile( aCSVFileURL, ProjectModule_Mapping[j].eBelongsTo, OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ));
                        break;
                    }
                    j++;
                }
            }

            aDir.close();
        }
    }

    OUString aVersionNumber;
    bool bOk = ExtractVersionNumber( rVersion, aVersionNumber );
    if ( bOk )
    {
        // Try to find the resource files and read in menus and slot infos
        for ( int i = 0; i < (int)rInDirVector.size(); i++ )
        {
            OUString aAbsInDirURL;
            OUString aInDirURL;
            OUString aInDir( rInDirVector[i] );

            osl::FileBase::getFileURLFromSystemPath( aInDir, aInDirURL );
            osl::FileBase::getAbsoluteFileURL( aWorkDir, aInDirURL, aAbsInDirURL );
            osl::Directory aDir( aAbsInDirURL );
            if ( aDir.open() == osl::FileBase::E_None )
            {
                osl::DirectoryItem aItem;
                while ( aDir.getNextItem( aItem ) == osl::FileBase::E_None )
                {
                    osl::FileStatus aFileStatus( FileStatusMask_FileName );

                    aItem.getFileStatus( aFileStatus );

                    int j=0;
                    OUString aFileName = aFileStatus.getFileName();

                    while ( ProjectModule_Mapping[j].pProjectFolder != 0 )
                    {
                        if ( aFileName.equalsAscii( ProjectModule_Mapping[j].pProjectFolder ))
                        {
                            OUStringBuffer aBuf( aAbsInDirURL );

                            aBuf.appendAscii( "/" );
                            aBuf.append( aFileStatus.getFileName() );
                            aBuf.appendAscii( "/common" );
                            if ( bUseProduct )
                                aBuf.appendAscii( ".pro" );
                            aBuf.appendAscii( "/bin/" );
                            if ( aUseRes.equalsAscii( "so" ))
                                aBuf.appendAscii( "so/" );

                            OUString aResDirURL( aBuf.makeStringAndClear() );

                            OUStringBuffer aLangResBuf;
                            aLangResBuf.appendAscii( ProjectModule_Mapping[j].pResPrefix );
                            aLangResBuf.append( aVersionNumber );
                            OUString aLangResPrefix( aLangResBuf.makeStringAndClear() );

                            for ( int k = 0; k < NUM_LANGUAGES; k++ )
                            {
                                char aBuf[8] = {0};

                                ReadResourceFile( k,
                                                  aResDirURL,
                                                  aLangResPrefix,
                                                  ProjectModule_Mapping[j].eBelongsTo,
                                                  OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ),
                                                  ProjectModule_Mapping[j].bSlotCommands );
                            }

                            OUString aOutDirURL;
                            osl::FileBase::getFileURLFromSystemPath( rOutputDirName, aOutDirURL );
                            osl::FileBase::getAbsoluteFileURL( aWorkDir, aOutDirURL, aOutDirURL );

                            ReadResourceWriteMenuBarXML( aOutDirURL, aResDirURL, aLangResPrefix, ProjectModule_Mapping[j].eBelongsTo, OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ));
                            ReadResourceWriteToolBarXML( aOutDirURL, aResDirURL, aLangResPrefix, ProjectModule_Mapping[j].eBelongsTo, OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ));
                            ReadResourceWriteSubToolBarXML( aOutDirURL, aResDirURL, aLangResPrefix, ProjectModule_Mapping[j].eBelongsTo, OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ));
                            ReadResourceWriteAcceleratorXML( aOutDirURL, aResDirURL, aLangResPrefix, ProjectModule_Mapping[j].eBelongsTo, ::rtl::OUString::createFromAscii( ProjectModule_Mapping[j].pProjectFolder ));
                        }
                        j++;
                    }
                }

                aDir.close();
            }
        }
    }

    OUString aOutDirURL;
    osl::FileBase::getFileURLFromSystemPath( rOutputDirName, aOutDirURL );
    osl::FileBase::getAbsoluteFileURL( aWorkDir, aOutDirURL, aOutDirURL );

    WriteXMLFiles( aOutDirURL );

    sal_Int32 n = 0;
    while ( TBResourceModule_Mapping[n].nResId > 0 )
    {
        if ( !TBResourceModule_Mapping[n].bWritten )
            fprintf( stderr, "Warning: Couldn't convert toolbar %s\n", TBResourceModule_Mapping[n].pXMLPrefix );
        n++;
    }

    n = 0;
    while ( TBSubResourceModule_Mapping[n].nResId > 0 )
    {
        if ( !TBSubResourceModule_Mapping[n].bWritten )
            fprintf( stderr, "Warning: Couldn't convert sub toolbar %s\n", TBSubResourceModule_Mapping[n].pXMLPrefix );
        n++;
    }

    return true;
}

bool GetCommandOption( const ::std::vector< OUString >& rArgs, const OUString& rSwitch, OUString& rParam )
{
    bool        bRet = false;
    OUString    aSwitch( OUString::createFromAscii( "-" ));

    aSwitch += rSwitch;
    for( int i = 0, nCount = rArgs.size(); ( i < nCount ) && !bRet; i++ )
    {
        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            if ( aSwitch.equalsIgnoreAsciiCase( rArgs[ i ] ))
            {
                bRet = true;

                if( i < ( nCount - 1 ) )
                    rParam = rArgs[ i + 1 ];
                else
                    rParam = OUString();
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------

bool GetCommandOptions( const ::std::vector< OUString >& rArgs, const OUString& rSwitch, ::std::vector< OUString >& rParams )
{
    bool bRet = false;

    OUString    aSwitch( OUString::createFromAscii( "-" ));

    aSwitch += rSwitch;
    for( int i = 0, nCount = rArgs.size(); ( i < nCount ); i++ )
    {
        for( int n = 0; ( n < 2 ) && !bRet; n++ )
        {
            if ( aSwitch.equalsIgnoreAsciiCase( rArgs[ i ] ))
            {
                if( i < ( nCount - 1 ) )
                    rParams.push_back( rArgs[ i + 1 ] );
                else
                    rParams.push_back( OUString() );

                break;
            }
        }
    }

    return( rParams.size() > 0 );
}

void ShowUsage()
{
    fprintf( stderr, "Usage: uicmdxml output_dir -r res -v version [-p] -s platform -i input_dir [-i input_dir] [-f errfile]\n" );
    fprintf( stderr, "Options:" );
    fprintf( stderr, "   -r [oo|so]    use resources from ooo, so\n" );
    fprintf( stderr, "   -v            name of the version used, f.e. SRX645, SRC680\n" );
    fprintf( stderr, "   -p            use product version\n" );
    fprintf( stderr, "   -s            name of the system to use, f.e. wntmsci8, unxsols4, unxlngi5\n" );
    fprintf( stderr, "   -i ...        name of directory to be searched for input files [multiple occurence is possible]\n" );
    fprintf( stderr, "   -f            name of file, error output should be written to\n" );
    fprintf( stderr, "Examples:\n" );
    fprintf( stderr, "    uicmdxml /home/out -r so -v SRC680 -p unxlngi5 -i /home/res -f /home/out/log.err\n" );
}

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
// -----------------------------------------------------------------------

// Forward declaration
void Main();

// -----------------------------------------------------------------------

SAL_IMPLEMENT_MAIN()
{
    Reference< XMultiServiceFactory > xMS;

    // for this to work make sure an <appname>.ini file is available, you can just copy soffice.ini
    Reference< XComponentContext > xComponentContext = ::cppu::defaultBootstrap_InitialComponentContext();
    xMS = Reference< XMultiServiceFactory >( xComponentContext->getServiceManager() , UNO_QUERY );

    InitVCL( xMS );
    ::Main();
    DeInitVCL();

    return 0;
}

void Main()
{
    ::std::vector< OUString > aArgs;

    sal_uInt32 nCmds = osl_getCommandArgCount();
    if ( nCmds >= 8 )
    {
        for ( sal_uInt32 i=0; i < nCmds; i++ )
        {
            OUString aArg;
            osl_getCommandArg( i, &aArg.pData );
            aArgs.push_back( aArg );
        }

        ::std::vector< OUString > aInDirVector;
        OUString                  aErrOutputFileName;
        OUString                  aOutputDirName( aArgs[0] );
        OUString                  aPlatformName;
        OUString                  aVersion;
        OUString                  aUseRes;
        bool                      bUseProduct;
        OUString                  aDummy;

        GetCommandOption( aArgs, OUString::createFromAscii( "v" ), aVersion );
        bUseProduct = GetCommandOption( aArgs, OUString::createFromAscii( "p" ), aDummy );
        GetCommandOption( aArgs, OUString::createFromAscii( "s" ), aPlatformName );
        GetCommandOptions( aArgs, OUString::createFromAscii( "i" ), aInDirVector );
        GetCommandOption( aArgs, OUString::createFromAscii( "f" ), aErrOutputFileName );
        GetCommandOption( aArgs, OUString::createFromAscii( "r" ), aUseRes );

        if ( aVersion.getLength() > 0 &&
             aPlatformName.getLength() > 0 &&
             aUseRes.getLength() > 0 &&
             aInDirVector.size() > 0 )
        {
            Convert( bUseProduct, aUseRes, aVersion, aOutputDirName, aPlatformName, aInDirVector, aErrOutputFileName );
        }
        else
        {
            ShowUsage();
            exit( -1 );
        }
    }
    else
    {
        ShowUsage();
        exit( -1 );
    }
}
