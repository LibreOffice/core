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
#include "parsefct.hxx"


// NOT FULLY DECLARED SERVICES
#include <cpp/prs_cpp.hxx>
#include <adoc/prs_adoc.hxx>
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

DYN autodoc::CodeParser_Ifc *
ParseToolsFactory::Create_Parser_Cplusplus() const
{
    return new cpp::Cpluplus_Parser;
}

DYN autodoc::DocumentationParser_Ifc *
ParseToolsFactory::Create_DocuParser_AutodocStyle() const
{
    return new adoc::DocuParser_AutodocStyle;
}

DYN autodoc::FileCollector_Ifc *
ParseToolsFactory::Create_FileCollector( uintt i_nEstimatedNrOfFiles ) const
{
    return new FileCollector(i_nEstimatedNrOfFiles);
}


