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

#include <accelerators/keymapping.hxx>

#include <macros/generic.hxx>

#include <com/sun/star/awt/Key.hpp>


namespace framework
{

//_______________________________________________
// helper

KeyMapping::KeyIdentifierInfo KeyMapping::KeyIdentifierMap[] =
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
    {css::awt::Key::BRACKETLEFT   , "KEY_BRACKETLEFT" },
    {css::awt::Key::BRACKETRIGHT  , "KEY_BRACKETRIGHT" },
    {css::awt::Key::SEMICOLON     , "KEY_SEMICOLON" },
    {0                            , ""               } // mark the end of this array!
};

//-----------------------------------------------
KeyMapping::KeyMapping()
{
    sal_Int32 i = 0;
    while(KeyIdentifierMap[i].Code != 0)
    {
        OUString sIdentifier = OUString::createFromAscii(KeyIdentifierMap[i].Identifier);
        sal_Int16       nCode       = KeyIdentifierMap[i].Code;

        m_lIdentifierHash[sIdentifier] = nCode      ;
        m_lCodeHash      [nCode]       = sIdentifier;

        ++i;
    }
}

//-----------------------------------------------
KeyMapping::~KeyMapping()
{
}

//-----------------------------------------------
sal_uInt16 KeyMapping::mapIdentifierToCode(const OUString& sIdentifier)
    throw(css::lang::IllegalArgumentException)
{
    Identifier2CodeHash::const_iterator pIt = m_lIdentifierHash.find(sIdentifier);
    if (pIt != m_lIdentifierHash.end())
        return pIt->second;

    // Its not well known identifier - but may be a pure key code formated as string ...
    // Check and convert it!
    sal_uInt16 nCode = 0;
    if (!KeyMapping::impl_st_interpretIdentifierAsPureKeyCode(sIdentifier, nCode))
        throw css::lang::IllegalArgumentException(
                DECLARE_ASCII("Cant map given identifier to a valid key code value."),
                css::uno::Reference< css::uno::XInterface >(),
                0);

    return (sal_uInt16)nCode;
}

//-----------------------------------------------
OUString KeyMapping::mapCodeToIdentifier(sal_uInt16 nCode)
{
    Code2IdentifierHash::const_iterator pIt = m_lCodeHash.find(nCode);
    if (pIt != m_lCodeHash.end())
        return pIt->second;

    // If we have no well known identifier - use the pure code value!
    return OUString::number(nCode);
}

//-----------------------------------------------
sal_Bool KeyMapping::impl_st_interpretIdentifierAsPureKeyCode(const OUString& sIdentifier,
                                                                    sal_uInt16&      rCode      )
{
    sal_Int32 nCode = sIdentifier.toInt32();
    if (nCode > 0)
    {
        rCode = (sal_uInt16)nCode;
        return sal_True;
    }

    // 0 is normaly an error of the called method toInt32() ...
    // But we must be aware, that the identifier is "0"!
    rCode = 0;
    return sIdentifier == "0";
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
