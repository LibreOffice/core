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

#ifndef AUTODOC_X_PARSING_HXX
#define AUTODOC_X_PARSING_HXX

// USED SERVICES
#include <iostream>




namespace autodoc
{

class X_Parser_Ifc
{
  public:
    // TYPES
    enum E_Event
    {
        x_Any                       = 0,
        x_InvalidChar,
        x_UnexpectedToken,
        x_UnexpectedEOF,
        x_UnspecifiedSyntaxError
    };

    // LIFECYCLE
    virtual             ~X_Parser_Ifc() {}

    // INQUIRY
    virtual E_Event     GetEvent() const = 0;
    virtual void        GetInfo(
                            std::ostream &      o_rOutputMedium ) const = 0;
};


}   // namespace autodoc

std::ostream &      operator<<(
                        std::ostream &      o_rOut,
                        const autodoc::X_Parser_Ifc &
                                            i_rException );




#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
