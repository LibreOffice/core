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

#include "controlelement.hxx"
#include <xmloff/xmltoken.hxx>

using namespace ::xmloff::token;

namespace xmloff
{

    //= OControlElement
    OUString OControlElement::getElementName(ElementType _eType)
    {
        switch (_eType)
        {
            case TEXT: return u"text"_ustr;
            case TEXT_AREA: return u"textarea"_ustr;
            case PASSWORD: return u"password"_ustr;
            case FILE: return u"file"_ustr;
            case FORMATTED_TEXT: return u"formatted-text"_ustr;
            case FIXED_TEXT: return u"fixed-text"_ustr;
            case COMBOBOX: return u"combobox"_ustr;
            case LISTBOX: return u"listbox"_ustr;
            case BUTTON: return u"button"_ustr;
            case IMAGE: return u"image"_ustr;
            case CHECKBOX: return u"checkbox"_ustr;
            case RADIO: return u"radio"_ustr;
            case FRAME: return u"frame"_ustr;
            case IMAGE_FRAME: return u"image-frame"_ustr;
            case HIDDEN: return u"hidden"_ustr;
            case GRID: return u"grid"_ustr;
            case VALUERANGE: return u"value-range"_ustr;
            case TIME: return u"time"_ustr;
            case DATE: return u"date"_ustr;

            default: return u"generic-control"_ustr;
        }
    }

    sal_Int32 OControlElement::getElementToken(ElementType _eType)
    {
        switch (_eType)
        {
            case TEXT: return XML_TEXT;
            case TEXT_AREA: return XML_TEXTAREA;
            case PASSWORD: return XML_PASSWORD;
            case FILE: return XML_FILE;
            case FORMATTED_TEXT: return XML_FORMATTED_TEXT;
            case FIXED_TEXT: return XML_FIXED_TEXT;
            case COMBOBOX: return XML_COMBOBOX;
            case LISTBOX: return XML_LISTBOX;
            case BUTTON: return XML_BUTTON;
            case IMAGE: return XML_IMAGE;
            case CHECKBOX: return XML_CHECKBOX;
            case RADIO: return XML_RADIO;
            case FRAME: return XML_FRAME;
            case IMAGE_FRAME: return XML_IMAGE_FRAME;
            case HIDDEN: return XML_HIDDEN;
            case GRID: return XML_GRID;
            case VALUERANGE: return XML_VALUE_RANGE;
            case TIME: return XML_TIME;
            case DATE: return XML_DATE;

            default: return XML_GENERIC_CONTROL;
        }
    }

}   // namespace xmloff

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
