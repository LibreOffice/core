/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: displaying.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:25:44 $
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
    virtual             ~DisplayToolsFactory_Ifc() {}
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

