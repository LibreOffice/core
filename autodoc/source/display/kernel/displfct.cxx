/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: displfct.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:58:57 $
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
//      return new CppTextDisplay_FunctionList;
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


