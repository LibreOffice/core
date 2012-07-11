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
#include "parsefct.hxx"


// NOT FULLY DECLARED SERVICES
#include <tools/filecoll.hxx>


DYN ParseToolsFactory * ParseToolsFactory::dpTheInstance_ = 0;


namespace autodoc
{

ParseToolsFactory_Ifc &
ParseToolsFactory_Ifc::GetIt_()
{
    if ( ParseToolsFactory::dpTheInstance_ == 0 )
        ParseToolsFactory::dpTheInstance_ = new ParseToolsFactory;
    return *ParseToolsFactory::dpTheInstance_;
}

}   // namespace autodoc


ParseToolsFactory::ParseToolsFactory()
{
}

ParseToolsFactory::~ParseToolsFactory()
{
}

DYN autodoc::FileCollector_Ifc *
ParseToolsFactory::Create_FileCollector( uintt i_nEstimatedNrOfFiles ) const
{
    return new FileCollector(i_nEstimatedNrOfFiles);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
