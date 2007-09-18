/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: parsefct.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2007-09-18 14:16:05 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


