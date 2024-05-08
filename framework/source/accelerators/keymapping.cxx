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

#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <o3tl/string_view.hxx>

namespace framework
{

// helper

KeyMapping::KeyIdentifierInfo constexpr KeyMapping::KeyIdentifierMap[] =
{
    {css::awt::Key::NUM0          , u"KEY_0"_ustr          },
    {css::awt::Key::NUM1          , u"KEY_1"_ustr          },
    {css::awt::Key::NUM2          , u"KEY_2"_ustr          },
    {css::awt::Key::NUM3          , u"KEY_3"_ustr          },
    {css::awt::Key::NUM4          , u"KEY_4"_ustr          },
    {css::awt::Key::NUM5          , u"KEY_5"_ustr          },
    {css::awt::Key::NUM6          , u"KEY_6"_ustr          },
    {css::awt::Key::NUM7          , u"KEY_7"_ustr          },
    {css::awt::Key::NUM8          , u"KEY_8"_ustr          },
    {css::awt::Key::NUM9          , u"KEY_9"_ustr          },
    {css::awt::Key::A             , u"KEY_A"_ustr          },
    {css::awt::Key::B             , u"KEY_B"_ustr          },
    {css::awt::Key::C             , u"KEY_C"_ustr          },
    {css::awt::Key::D             , u"KEY_D"_ustr          },
    {css::awt::Key::E             , u"KEY_E"_ustr          },
    {css::awt::Key::F             , u"KEY_F"_ustr          },
    {css::awt::Key::G             , u"KEY_G"_ustr          },
    {css::awt::Key::H             , u"KEY_H"_ustr          },
    {css::awt::Key::I             , u"KEY_I"_ustr          },
    {css::awt::Key::J             , u"KEY_J"_ustr          },
    {css::awt::Key::K             , u"KEY_K"_ustr          },
    {css::awt::Key::L             , u"KEY_L"_ustr          },
    {css::awt::Key::M             , u"KEY_M"_ustr          },
    {css::awt::Key::N             , u"KEY_N"_ustr          },
    {css::awt::Key::O             , u"KEY_O"_ustr          },
    {css::awt::Key::P             , u"KEY_P"_ustr          },
    {css::awt::Key::Q             , u"KEY_Q"_ustr          },
    {css::awt::Key::R             , u"KEY_R"_ustr          },
    {css::awt::Key::S             , u"KEY_S"_ustr          },
    {css::awt::Key::T             , u"KEY_T"_ustr          },
    {css::awt::Key::U             , u"KEY_U"_ustr          },
    {css::awt::Key::V             , u"KEY_V"_ustr          },
    {css::awt::Key::W             , u"KEY_W"_ustr          },
    {css::awt::Key::X             , u"KEY_X"_ustr          },
    {css::awt::Key::Y             , u"KEY_Y"_ustr          },
    {css::awt::Key::Z             , u"KEY_Z"_ustr          },
    {css::awt::Key::F1            , u"KEY_F1"_ustr         },
    {css::awt::Key::F2            , u"KEY_F2"_ustr         },
    {css::awt::Key::F3            , u"KEY_F3"_ustr         },
    {css::awt::Key::F4            , u"KEY_F4"_ustr         },
    {css::awt::Key::F5            , u"KEY_F5"_ustr         },
    {css::awt::Key::F6            , u"KEY_F6"_ustr         },
    {css::awt::Key::F7            , u"KEY_F7"_ustr         },
    {css::awt::Key::F8            , u"KEY_F8"_ustr         },
    {css::awt::Key::F9            , u"KEY_F9"_ustr         },
    {css::awt::Key::F10           , u"KEY_F10"_ustr        },
    {css::awt::Key::F11           , u"KEY_F11"_ustr        },
    {css::awt::Key::F12           , u"KEY_F12"_ustr        },
    {css::awt::Key::F13           , u"KEY_F13"_ustr        },
    {css::awt::Key::F14           , u"KEY_F14"_ustr        },
    {css::awt::Key::F15           , u"KEY_F15"_ustr        },
    {css::awt::Key::F16           , u"KEY_F16"_ustr        },
    {css::awt::Key::F17           , u"KEY_F17"_ustr        },
    {css::awt::Key::F18           , u"KEY_F18"_ustr        },
    {css::awt::Key::F19           , u"KEY_F19"_ustr        },
    {css::awt::Key::F20           , u"KEY_F20"_ustr        },
    {css::awt::Key::F21           , u"KEY_F21"_ustr        },
    {css::awt::Key::F22           , u"KEY_F22"_ustr        },
    {css::awt::Key::F23           , u"KEY_F23"_ustr        },
    {css::awt::Key::F24           , u"KEY_F24"_ustr        },
    {css::awt::Key::F25           , u"KEY_F25"_ustr        },
    {css::awt::Key::F26           , u"KEY_F26"_ustr        },
    {css::awt::Key::DOWN          , u"KEY_DOWN"_ustr       },
    {css::awt::Key::UP            , u"KEY_UP"_ustr         },
    {css::awt::Key::LEFT          , u"KEY_LEFT"_ustr       },
    {css::awt::Key::RIGHT         , u"KEY_RIGHT"_ustr      },
    {css::awt::Key::HOME          , u"KEY_HOME"_ustr       },
    {css::awt::Key::END           , u"KEY_END"_ustr        },
    {css::awt::Key::PAGEUP        , u"KEY_PAGEUP"_ustr     },
    {css::awt::Key::PAGEDOWN      , u"KEY_PAGEDOWN"_ustr   },
    {css::awt::Key::RETURN        , u"KEY_RETURN"_ustr     },
    {css::awt::Key::ESCAPE        , u"KEY_ESCAPE"_ustr     },
    {css::awt::Key::TAB           , u"KEY_TAB"_ustr        },
    {css::awt::Key::BACKSPACE     , u"KEY_BACKSPACE"_ustr  },
    {css::awt::Key::SPACE         , u"KEY_SPACE"_ustr      },
    {css::awt::Key::INSERT        , u"KEY_INSERT"_ustr     },
    {css::awt::Key::DELETE        , u"KEY_DELETE"_ustr     },
    {css::awt::Key::ADD           , u"KEY_ADD"_ustr        },
    {css::awt::Key::SUBTRACT      , u"KEY_SUBTRACT"_ustr   },
    {css::awt::Key::MULTIPLY      , u"KEY_MULTIPLY"_ustr   },
    {css::awt::Key::DIVIDE        , u"KEY_DIVIDE"_ustr     },
    {css::awt::Key::POINT         , u"KEY_POINT"_ustr      },
    {css::awt::Key::COMMA         , u"KEY_COMMA"_ustr      },
    {css::awt::Key::LESS          , u"KEY_LESS"_ustr       },
    {css::awt::Key::GREATER       , u"KEY_GREATER"_ustr    },
    {css::awt::Key::EQUAL         , u"KEY_EQUAL"_ustr      },
    {css::awt::Key::OPEN          , u"KEY_OPEN"_ustr       },
    {css::awt::Key::CUT           , u"KEY_CUT"_ustr        },
    {css::awt::Key::COPY          , u"KEY_COPY"_ustr       },
    {css::awt::Key::PASTE         , u"KEY_PASTE"_ustr      },
    {css::awt::Key::UNDO          , u"KEY_UNDO"_ustr       },
    {css::awt::Key::REPEAT        , u"KEY_REPEAT"_ustr     },
    {css::awt::Key::FIND          , u"KEY_FIND"_ustr       },
    {css::awt::Key::PROPERTIES    , u"KEY_PROPERTIES"_ustr },
    {css::awt::Key::FRONT         , u"KEY_FRONT"_ustr      },
    {css::awt::Key::CONTEXTMENU   , u"KEY_CONTEXTMENU"_ustr},
    {css::awt::Key::HELP          , u"KEY_HELP"_ustr       },
    {css::awt::Key::MENU          , u"KEY_MENU"_ustr       },
    {css::awt::Key::HANGUL_HANJA  , u"KEY_HANGUL_HANJA"_ustr},
    {css::awt::Key::DECIMAL       , u"KEY_DECIMAL"_ustr    },
    {css::awt::Key::TILDE         , u"KEY_TILDE"_ustr      },
    {css::awt::Key::QUOTELEFT     , u"KEY_QUOTELEFT"_ustr  },
    {css::awt::Key::BRACKETLEFT   , u"KEY_BRACKETLEFT"_ustr },
    {css::awt::Key::BRACKETRIGHT  , u"KEY_BRACKETRIGHT"_ustr },
    {css::awt::Key::SEMICOLON     , u"KEY_SEMICOLON"_ustr },
    {css::awt::Key::QUOTERIGHT    , u"KEY_QUOTERIGHT"_ustr },
    {css::awt::Key::RIGHTCURLYBRACKET, u"KEY_RIGHTCURLYBRACKET"_ustr },
    {css::awt::Key::NUMBERSIGN, u"KEY_NUMBERSIGN"_ustr },
    {css::awt::Key::COLON         , u"KEY_COLON"_ustr },
    {0                            , u""_ustr               } // mark the end of this array!
};

KeyMapping::KeyMapping()
{
    sal_Int32 i = 0;
    while(KeyIdentifierMap[i].Code != 0)
    {
        m_lIdentifierHash[KeyIdentifierMap[i].Identifier] = KeyIdentifierMap[i].Code;
        m_lCodeHash      [KeyIdentifierMap[i].Code]       = KeyIdentifierMap[i].Identifier;

        ++i;
    }
}

KeyMapping & KeyMapping::get() {
    static KeyMapping KEYS;
    return KEYS;
}

sal_uInt16 KeyMapping::mapIdentifierToCode(const OUString& sIdentifier)
{
    Identifier2CodeHash::const_iterator pIt = m_lIdentifierHash.find(sIdentifier);
    if (pIt != m_lIdentifierHash.end())
        return pIt->second;

    // It's not well known identifier - but may be a pure key code formatted as string...
    // Check and convert it!
    sal_uInt16 nCode = 0;
    if (!KeyMapping::impl_st_interpretIdentifierAsPureKeyCode(sIdentifier, nCode))
        throw css::lang::IllegalArgumentException(
                u"Can not map given identifier to a valid key code value."_ustr,
                css::uno::Reference< css::uno::XInterface >(),
                0);

    return nCode;
}

OUString KeyMapping::mapCodeToIdentifier(sal_uInt16 nCode)
{
    Code2IdentifierHash::const_iterator pIt = m_lCodeHash.find(nCode);
    if (pIt != m_lCodeHash.end())
        return pIt->second;

    // If we have no well known identifier - use the pure code value!
    return OUString::number(nCode);
}

bool KeyMapping::impl_st_interpretIdentifierAsPureKeyCode(std::u16string_view sIdentifier,
                                                                sal_uInt16&      rCode      )
{
    sal_Int32 nCode = o3tl::toInt32(sIdentifier);
    if (nCode > 0)
    {
        rCode = static_cast<sal_uInt16>(nCode);
        return true;
    }

    // 0 is normally an error of the called method toInt32() ...
    // But we must be aware, that the identifier is "0"!
    rCode = 0;
    return sIdentifier == u"0";
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
