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

#ifndef AUTODOC_DISPLAYING_HXX
#define AUTODOC_DISPLAYING_HXX


namespace display
{
    class CorporateFrame;
}



namespace autodoc
{

class HtmlDisplay_UdkStd;
class HtmlDisplay_Idl_Ifc;

// class TextDisplay_FunctionList_Ifc;


/** Interface for parsing code of a programming language and
    delivering the information into an Autodoc Repository.
**/
class DisplayToolsFactory_Ifc
{
  public:
    virtual				~DisplayToolsFactory_Ifc() {}
    static DisplayToolsFactory_Ifc &
                        GetIt_();

//    virtual DYN autodoc::TextDisplay_FunctionList_Ifc *
//                        Create_TextDisplay_FunctionList() const = 0;

    virtual DYN autodoc::HtmlDisplay_UdkStd *
                        Create_HtmlDisplay_UdkStd() const = 0;
    virtual DYN autodoc::HtmlDisplay_Idl_Ifc *
                        Create_HtmlDisplay_Idl() const = 0;

    virtual const display::CorporateFrame &
                        Create_StdFrame() const = 0;
};


} // namespace autodoc



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
