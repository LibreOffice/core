/*************************************************************************
 *
 *  $RCSfile: displfct.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: np $ $Date: 2002-05-14 09:02:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <precomp.h>
#include "displfct.hxx"


// NOT FULLY DECLARED SERVICES
#include <html/chd_udk2.hxx>
#include <html/hd_udk2.hxx>
#include <html/cfrstd.hxx>


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
    return new IdlHtmlDisplay_Udk2;
}

const display::CorporateFrame &
DisplayToolsFactory::Create_StdFrame() const
{
    static StdFrame aFrame;
    return aFrame;
}


