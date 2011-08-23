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

#ifndef AUTODOC_DSP_TXT_FLIST_HXX
#define AUTODOC_DSP_TXT_FLIST_HXX

#include <iostream>


namespace ary
{
    namespace cpp
    {
        class DisplayGate;
    }
}


namespace autodoc
{

class TextDisplay_FunctionList_Ifc
{
  public:
    virtual			    ~TextDisplay_FunctionList_Ifc() {}

    /** Displays the names of all C++ functions and methods within the
        given namespace (or the global namespace as default). All
        subnamespaces are included.
    */
    virtual void        Run(
                            ostream &           o_rStream,
                            const ary::cpp::DisplayGate &
                                                i_rAryGate ) = 0; /// If i_pNamespace == 0, the global namespace is displayed.
};


}   // namespace autodoc

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
