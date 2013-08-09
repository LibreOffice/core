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

#ifndef _XMLOFF_FORMS_CONTROLELEMENT_HXX_
#define _XMLOFF_FORMS_CONTROLELEMENT_HXX_

#include <sal/types.h>

namespace xmloff
{

    //= OControlElement
    /** helper for translating between control types and XML tags
    */
    class OControlElement
    {
    public:
        enum ElementType
        {
            TEXT = 0,
            TEXT_AREA,
            PASSWORD,
            FILE,
            FORMATTED_TEXT,
            FIXED_TEXT,
            COMBOBOX,
            LISTBOX,
            BUTTON,
            IMAGE,
            CHECKBOX,
            RADIO,
            FRAME,
            IMAGE_FRAME,
            HIDDEN,
            GRID,
            VALUERANGE,
            GENERIC_CONTROL,
            TIME,
            DATE,

            UNKNOWN // must be the last element
        };

    protected:
        /** ctor.
            <p>This default constructor is protected, 'cause this class is not intended to be instantiated
            directly. Instead, the derived classes should be used.</p>
        */
        OControlElement() { }

    public:
        /** retrieves the tag name to be used to describe a control of the given type

            <p>The retuned string is the pure element name, without any namespace.</p>

            @param  _eType
                the element type
        */
        static const sal_Char* getElementName(ElementType _eType);
    };

}   // namespace xmloff

#endif // _XMLOFF_FORMS_CONTROLELEMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
