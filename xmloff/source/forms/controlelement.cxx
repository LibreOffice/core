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
    const char* OControlElement::getElementName(ElementType _eType)
    {
        switch (_eType)
        {
            case TEXT: return "text";
            case TEXT_AREA: return "textarea";
            case PASSWORD: return "password";
            case FILE: return "file";
            case FORMATTED_TEXT: return "formatted-text";
            case FIXED_TEXT: return "fixed-text";
            case COMBOBOX: return "combobox";
            case LISTBOX: return "listbox";
            case BUTTON: return "button";
            case IMAGE: return "image";
            case CHECKBOX: return "checkbox";
            case RADIO: return "radio";
            case FRAME: return "frame";
            case IMAGE_FRAME: return "image-frame";
            case HIDDEN: return "hidden";
            case GRID: return "grid";
            case VALUERANGE: return "value-range";
            case TIME: return "time";
            case DATE: return "date";

            default: return "generic-control";
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
