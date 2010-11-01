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
