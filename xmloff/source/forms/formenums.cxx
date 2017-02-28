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
#include <tools/gen.hxx>
#include <xmloff/xmltoken.hxx>

namespace xmloff
{

    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::sdb;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star;
    using namespace ::xmloff::token;

                // FormSubmitEncoding
                    const SvXMLEnumMapEntry aSubmitEncodingMap[] =
                    {
                        { XML_APPLICATION_X_WWW_FORM_URLENCODED, FormSubmitEncoding_URL },
                        { XML_MULTIPART_FORMDATA, FormSubmitEncoding_MULTIPART },
                        { XML_APPLICATION_TEXT, FormSubmitEncoding_TEXT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    // FormSubmitMethod
                    const SvXMLEnumMapEntry aSubmitMethodMap[] =
                    {
                        { XML_GET, FormSubmitMethod_GET },
                        { XML_POST, FormSubmitMethod_POST },
                        { XML_TOKEN_INVALID, 0 }
                    };

                // CommandType
                    const SvXMLEnumMapEntry aCommandTypeMap[] =
                    {
                        { XML_TABLE, CommandType::TABLE },
                        { XML_QUERY, CommandType::QUERY },
                        { XML_COMMAND, CommandType::COMMAND },
                        { XML_TOKEN_INVALID, 0 }
                    };
                // NavigationBarMode
                    const SvXMLEnumMapEntry aNavigationTypeMap[] =
                    {
                        { XML_NONE, NavigationBarMode_NONE },
                        { XML_CURRENT, NavigationBarMode_CURRENT },
                        { XML_PARENT, NavigationBarMode_PARENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                // TabulatorCycle
                    const SvXMLEnumMapEntry aTabulatorCycleMap[] =
                    {
                        { XML_RECORDS, TabulatorCycle_RECORDS },
                        { XML_CURRENT, TabulatorCycle_CURRENT },
                        { XML_PAGE, TabulatorCycle_PAGE },
                        { XML_TOKEN_INVALID, 0 }
                    };
                // FormButtonType
                    const SvXMLEnumMapEntry aFormButtonTypeMap[] =
                    {
                        { XML_PUSH, FormButtonType_PUSH },
                        { XML_SUBMIT, FormButtonType_SUBMIT },
                        { XML_RESET, FormButtonType_RESET },
                        { XML_URL, FormButtonType_URL },
                        { XML_TOKEN_INVALID, 0 }
                    };
                // ListSourceType
                    const SvXMLEnumMapEntry aListSourceTypeMap[] =
                    {
                        { XML_VALUE_LIST, ListSourceType_VALUELIST },
                        { XML_TABLE, ListSourceType_TABLE },
                        { XML_QUERY, ListSourceType_QUERY },
                        { XML_SQL, ListSourceType_SQL },
                        { XML_SQL_PASS_THROUGH, ListSourceType_SQLPASSTHROUGH },
                        { XML_TABLE_FIELDS, ListSourceType_TABLEFIELDS },
                        { XML_TOKEN_INVALID, 0 }
                    };
                // check state of a checkbox
                    const SvXMLEnumMapEntry aCheckStateMap[] =
                    {
                        { XML_UNCHECKED, TRISTATE_FALSE },
                        { XML_CHECKED, TRISTATE_TRUE },
                        { XML_UNKNOWN, TRISTATE_INDET },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aTextAlignMap[] =
                    {
                        { XML_START,        awt::TextAlign::LEFT },
                        { XML_CENTER,       awt::TextAlign::CENTER },
                        { XML_END,          awt::TextAlign::RIGHT },
                        { XML_JUSTIFY,      (sal_uInt16)-1 },
                        { XML_JUSTIFIED,    (sal_uInt16)-1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aBorderTypeMap[] =
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
                    const SvXMLEnumMapEntry aFontEmphasisMap[] =
                    {
                        { XML_NONE,     awt::FontEmphasisMark::NONE },
                        { XML_DOT,      awt::FontEmphasisMark::DOT },
                        { XML_CIRCLE,   awt::FontEmphasisMark::CIRCLE },
                        { XML_DISC,     awt::FontEmphasisMark::DISC },
                        { XML_ACCENT,   awt::FontEmphasisMark::ACCENT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aFontReliefMap[] =
                    {
                        { XML_NONE,     FontRelief::NONE },
                        { XML_ENGRAVED, FontRelief::ENGRAVED },
                        { XML_EMBOSSED, FontRelief::EMBOSSED },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aListLinkageMap[] =
                    {
                        { XML_SELECTION,            0 },
                        { XML_SELECTION_INDEXES,    1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aOrientationMap[] =
                    {
                        { XML_HORIZONTAL,   ScrollBarOrientation::HORIZONTAL },
                        { XML_VERTICAL,     ScrollBarOrientation::VERTICAL },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aVisualEffectMap[] =
                    {
                        { XML_NONE, VisualEffect::NONE },
                        { XML_3D,   VisualEffect::LOOK3D },
                        { XML_FLAT, VisualEffect::FLAT },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aImagePositionMap[] =
                    {
                        { XML_START,  0 },
                        { XML_END,    1 },
                        { XML_TOP,    2 },
                        { XML_BOTTOM, 3 },
                        { XML_CENTER, (sal_uInt16)-1 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aImageAlignMap[] =
                    {
                        { XML_START,  0 },
                        { XML_CENTER, 1 },
                        { XML_END, 2 },
                        { XML_TOKEN_INVALID, 0 }
                    };
                    const SvXMLEnumMapEntry aScaleModeMap[] =
                    {
                        { XML_BACKGROUND_NO_REPEAT, ImageScaleMode::NONE },
                        { XML_REPEAT,               ImageScaleMode::NONE },  // repeating the image is not supported
                        { XML_STRETCH,              ImageScaleMode::ANISOTROPIC },
                        { XML_SCALE,                ImageScaleMode::ISOTROPIC },
                        { XML_TOKEN_INVALID,        ImageScaleMode::NONE }
                    };

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
