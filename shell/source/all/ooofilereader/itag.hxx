/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: itag.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:43:58 $
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


#ifndef ITAG_HXX_INCLUDED
#define ITAG_HXX_INCLUDED

#ifndef CONFIG_HXX_INCLUDED
#include "internal/config.hxx"
#endif

#ifndef TYPES_HXX_INCLUDED
#include "internal/types.hxx"
#endif

/***************************   interface of tag readers   ***************************/

/** Interface for a xml tag character builder
*/
class ITag
{
    public:
        virtual ~ITag() {};

        virtual void startTag() = 0;
        virtual void endTag() = 0;
        virtual void addCharacters(const std::wstring& characters) = 0;
        virtual void addAttributes(const XmlTagAttributes_t& attributes) = 0;
        virtual ::std::wstring getTagContent( void ) = 0;
        virtual ::std::wstring const getTagAttribute( ::std::wstring  const & attrname ) = 0;
};

#endif