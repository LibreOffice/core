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

#include "formenums.hxx"
#include <osl/diagnose.h>
#include <com/sun/star/form/FormSubmitEncoding.hpp>
#include <com/sun/star/form/FormSubmitMethod.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/form/NavigationBarMode.hpp>
#include <com/sun/star/form/TabulatorCycle.hpp>
#include <com/sun/star/form/FormButtonType.hpp>
#include <com/sun/star/form/ListSourceType.hpp>
#include <com/sun/star/awt/TextAlign.hpp>
#include <com/sun/star/awt/FontWidth.hpp>
#include <com/sun/star/awt/FontEmphasisMark.hpp>
#include <com/sun/star/awt/FontRelief.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/VisualEffect.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <tools/wintypes.hxx>       // for check states
#include <xmloff/xmltoken.hxx>

namespace xmloff
{

    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star;
    using namespace ::xmloff::token;

    const SvXMLEnumMapEntry*    OEnumMapper::s_pEnumMap[OEnumMapper::KNOWN_ENUM_PROPERTIES] =
    {
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

    const SvXMLEnumMapEntry*    OEnumMapper::getEnumMap(EnumProperties _eProperty)
    {
        OSL_ENSURE(_eProperty < KNOWN_ENUM_PROPERTIES, "OEnumMapper::getEnumMap: invalid index (this will crash)!");

        const SvXMLEnumMapEntry*& rReturn = s_pEnumMap[_eProperty];
        if (!rReturn)
        {
            // the map for this property is not initialized yet
             switch (_eProperty)
            {
                // FormSubmitEncoding
                case epSubmitEncoding:
                {
                    static const SvXMLEnumMapEntry aSubmitEncodingMap[] =
                    {
                        { XML_APPLICATION_X_WWW_FORM_URLENCODED, FormSubmitEncoding_URL },
                        { XML_MULTIPART_FORMDATA, FormSubmitEncoding_MULTIPART },
                        { XML_APPLICATION_TEXT, FormSubmitEncoding_TEXT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aSubmitEncodingMap;
                }
                break;
                // FormSubmitMethod
                case epSubmitMethod:
                {
                    static const SvXMLEnumMapEntry aSubmitMethodMap[] =
                    {
                        { XML_GET, FormSubmitMethod_GET },
                        { XML_POST, FormSubmitMethod_POST },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aSubmitMethodMap;
                }
                break;
                // CommandType
                case epCommandType:
                {
                    static const SvXMLEnumMapEntry aCommandTypeMap[] =
                    {
                        { XML_TABLE, CommandType::TABLE },
                        { XML_QUERY, CommandType::QUERY },
                        { XML_COMMAND, CommandType::COMMAND },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aCommandTypeMap;
                }
                break;
                // NavigationBarMode
                case epNavigationType:
                {
                    static const SvXMLEnumMapEntry aNavigationTypeMap[] =
                    {
                        { XML_NONE, NavigationBarMode_NONE },
                        { XML_CURRENT, NavigationBarMode_CURRENT },
                        { XML_PARENT, NavigationBarMode_PARENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aNavigationTypeMap;
                };
                break;
                // TabulatorCycle
                case epTabCyle:
                {
                    static const SvXMLEnumMapEntry aTabulytorCycleMap[] =
                    {
                        { XML_RECORDS, TabulatorCycle_RECORDS },
                        { XML_CURRENT, TabulatorCycle_CURRENT },
                        { XML_PAGE, TabulatorCycle_PAGE },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aTabulytorCycleMap;
                };
                break;
                // FormButtonType
                case epButtonType:
                {
                    static const SvXMLEnumMapEntry aFormButtonTypeMap[] =
                    {
                        { XML_PUSH, FormButtonType_PUSH },
                        { XML_SUBMIT, FormButtonType_SUBMIT },
                        { XML_RESET, FormButtonType_RESET },
                        { XML_URL, FormButtonType_URL },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFormButtonTypeMap;
                };
                break;
                // ListSourceType
                case epListSourceType:
                {
                    static const SvXMLEnumMapEntry aListSourceTypeMap[] =
                    {
                        { XML_VALUE_LIST, ListSourceType_VALUELIST },
                        { XML_TABLE, ListSourceType_TABLE },
                        { XML_QUERY, ListSourceType_QUERY },
                        { XML_SQL, ListSourceType_SQL },
                        { XML_SQL_PASS_THROUGH, ListSourceType_SQLPASSTHROUGH },
                        { XML_TABLE_FIELDS, ListSourceType_TABLEFIELDS },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aListSourceTypeMap;
                };
                break;
                // check state of a checkbox
                case epCheckState:
                {
                    static const SvXMLEnumMapEntry aCheckStateMap[] =
                    {
                        { XML_UNCHECKED, STATE_NOCHECK },
                        { XML_CHECKED, STATE_CHECK },
                        { XML_UNKNOWN, STATE_DONTKNOW },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aCheckStateMap;
                };
                break;
                case epTextAlign:
                {
                    static const SvXMLEnumMapEntry aTextAlignMap[] =
                    {
                        { XML_START,        awt::TextAlign::LEFT },
                        { XML_CENTER,       awt::TextAlign::CENTER },
                        { XML_END,          awt::TextAlign::RIGHT },
                        { XML_JUSTIFY,      (sal_uInt16)-1 },
                        { XML_JUSTIFIED,    (sal_uInt16)-1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aTextAlignMap;
                };
                break;
                case epBorderWidth:
                {
                    static const SvXMLEnumMapEntry aBorderTypeMap[] =
                    {
                        { XML_NONE,     0 },
                        { XML_HIDDEN,   0 },
                        { XML_SOLID,    2 },
                        { XML_DOUBLE,   2 },
                        { XML_DOTTED,   2 },
                        { XML_DASHED,   2 },
                        { XML_GROOVE,   1 },
                        { XML_RIDGE,    1 },
                        { XML_INSET,    1 },
                        { XML_OUTSET,   1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aBorderTypeMap;
                };
                break;

                case epFontEmphasis:
                {
                    static const SvXMLEnumMapEntry aFontEmphasisMap[] =
                    {
                        { XML_NONE,     awt::FontEmphasisMark::NONE },
                        { XML_DOT,      awt::FontEmphasisMark::DOT },
                        { XML_CIRCLE,   awt::FontEmphasisMark::CIRCLE },
                        { XML_DISC,     awt::FontEmphasisMark::DISC },
                        { XML_ACCENT,   awt::FontEmphasisMark::ACCENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFontEmphasisMap;
                }
                break;

                case epFontRelief:
                {
                    static const SvXMLEnumMapEntry aFontReliefMap[] =
                    {
                        { XML_NONE,     FontRelief::NONE },
                        { XML_ENGRAVED, FontRelief::ENGRAVED },
                        { XML_EMBOSSED, FontRelief::EMBOSSED },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aFontReliefMap;
                }
                break;

                case epListLinkageType:
                {
                    static const SvXMLEnumMapEntry aListLinkageMap[] =
                    {
                        { XML_SELECTION,            0 },
                        { XML_SELECTION_INDEXES,    1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aListLinkageMap;
                }
                break;

                case epOrientation:
                {
                    static const SvXMLEnumMapEntry aOrientationMap[] =
                    {
                        { XML_HORIZONTAL,   ScrollBarOrientation::HORIZONTAL },
                        { XML_VERTICAL,     ScrollBarOrientation::VERTICAL },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aOrientationMap;
                }
                break;

                case epVisualEffect:
                {
                    static const SvXMLEnumMapEntry aVisualEffectMap[] =
                    {
                        { XML_NONE, VisualEffect::NONE },
                        { XML_3D,   VisualEffect::LOOK3D },
                        { XML_FLAT, VisualEffect::FLAT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aVisualEffectMap;
                }
                break;

                case epImagePosition:
                {
                    static const SvXMLEnumMapEntry aImagePositionMap[] =
                    {
                        { XML_START,  0 },
                        { XML_END,    1 },
                        { XML_TOP,    2 },
                        { XML_BOTTOM, 3 },
                        { XML_CENTER, (sal_uInt16)-1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aImagePositionMap;
                }
                break;

                case epImageAlign:
                {
                    static const SvXMLEnumMapEntry aImageAlignMap[] =
                    {
                        { XML_START,  0 },
                        { XML_CENTER, 1 },
                        { XML_END, 2 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    rReturn = aImageAlignMap;
                }
                break;

                case epImageScaleMode:
                {
                    static const SvXMLEnumMapEntry aScaleModeMap[] =
                    {
                        { XML_BACKGROUND_NO_REPEAT, ImageScaleMode::NONE },
                        { XML_REPEAT,               ImageScaleMode::NONE },  // repeating the image is not supported
                        { XML_STRETCH,              ImageScaleMode::ANISOTROPIC },
                        { XML_SCALE,                ImageScaleMode::ISOTROPIC },
                        { XML_TOKEN_INVALID,        ImageScaleMode::NONE }
                    };
                    rReturn = aScaleModeMap;
                }
                break;

                case KNOWN_ENUM_PROPERTIES:
                    break;
            }
        }

        return rReturn;
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
