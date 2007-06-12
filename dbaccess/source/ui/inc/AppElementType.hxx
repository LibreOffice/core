/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AppElementType.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2007-06-12 05:34:26 $
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
#ifndef DBAUI_APPELEMENTTYPE_HXX
#define DBAUI_APPELEMENTTYPE_HXX
//........................................................................
namespace dbaui
{
//........................................................................

    enum ElementType
    {
        E_FORM      = 0,
        E_REPORT    = 1,
        E_QUERY     = 2,
        E_TABLE     = 3,

        E_NONE      = 4,
        E_ELEMENT_TYPE_COUNT = E_NONE
    };

    enum PreviewMode
    {
        E_PREVIEWNONE   = 0,
        E_DOCUMENT      = 1,
        E_DOCUMENTINFO  = 2
    };
//........................................................................
} // namespace dbaui
//........................................................................
#endif // DBAUI_APPELEMENTTYPE_HXX

