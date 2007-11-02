/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x_parsing.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:22:09 $
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

#ifndef AUTODOC_X_PARSING_HXX
#define AUTODOC_X_PARSING_HXX

// USED SERVICES
#include <iostream>




namespace autodoc
{

class X_Parser_Ifc
{
  public:
    // TYPES
    enum E_Event
    {
        x_Any                       = 0,
        x_InvalidChar,
        x_UnexpectedToken,
        x_UnexpectedEOF,
        x_UnspecifiedSyntaxError
    };

    // LIFECYCLE
    virtual             ~X_Parser_Ifc() {}

    // INQUIRY
    virtual E_Event     GetEvent() const = 0;
    virtual void        GetInfo(
                            std::ostream &      o_rOutputMedium ) const = 0;
};


}   // namespace autodoc

std::ostream &      operator<<(
                        std::ostream &      o_rOut,
                        const autodoc::X_Parser_Ifc &
                                            i_rException );




#endif
