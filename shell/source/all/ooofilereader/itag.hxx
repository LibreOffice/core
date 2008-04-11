/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: itag.hxx,v $
 * $Revision: 1.4 $
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


#ifndef ITAG_HXX_INCLUDED
#define ITAG_HXX_INCLUDED

#include "internal/config.hxx"
#include "internal/types.hxx"

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