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

#include <precomp.h>
#include "displfct.hxx"


// NOT FULLY DECLARED SERVICES
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
//      return new CppTextDisplay_FunctionList;
// }


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
