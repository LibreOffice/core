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

#include <precomp.h>
#include "displfct.hxx"


// NOT FULLY DECLARED SERVICES
#include <html/chd_udk2.hxx>
#include <idl/hi_display.hxx>
#include <cfrstd.hxx>


DYN DisplayToolsFactory * DisplayToolsFactory::dpTheInstance_ = 0;


namespace autodoc
{

DisplayToolsFactory_Ifc &
DisplayToolsFactory_Ifc::GetIt_()
{
    if ( DisplayToolsFactory::dpTheInstance_ == 0 )
        DisplayToolsFactory::dpTheInstance_ = new DisplayToolsFactory;
    return *DisplayToolsFactory::dpTheInstance_;
}

}   // namespace autodoc


DisplayToolsFactory::DisplayToolsFactory()
{
}

DisplayToolsFactory::~DisplayToolsFactory()
{
}

// DYN autodoc::TextDisplay_FunctionList_Ifc *
// DisplayToolsFactory::Create_TextDisplay_FunctionList() const
// {
//  	return new CppTextDisplay_FunctionList;
// }


DYN autodoc::HtmlDisplay_UdkStd *
DisplayToolsFactory::Create_HtmlDisplay_UdkStd() const
{
    return new CppHtmlDisplay_Udk2;
}

DYN autodoc::HtmlDisplay_Idl_Ifc *
DisplayToolsFactory::Create_HtmlDisplay_Idl() const
{
    return new HtmlDisplay_Idl;
}

const display::CorporateFrame &
DisplayToolsFactory::Create_StdFrame() const
{
    static StdFrame aFrame;
    return aFrame;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
