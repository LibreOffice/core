/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: x_parse2.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 17:19:52 $
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
#include <x_parse2.hxx>

// NOT FULLY DECLARED SERVICES

    enum E_Type
    {
        x_Any                       = 0,
        x_InvalidChar,
        x_UnexpectedEOF
    };
void
X_AutodocParser::GetInfo( std::ostream &        o_rOutputMedium ) const
{
    switch (eType)
    {
        case x_Any:
            o_rOutputMedium << "Unspecified parsing exception ." << Endl();
            break;
        case x_InvalidChar:
            o_rOutputMedium << "Unknown character during parsing." << Endl();
            break;
        case x_UnexpectedToken:
            o_rOutputMedium << "Unexpected token " << sName << " found." << Endl();
            break;
        case x_UnexpectedEOF:
            o_rOutputMedium << "Unexpected end of file found." << Endl();
            break;
        default:
            o_rOutputMedium << "Unknown exception during parsing." << Endl();
    }
}


