/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#ifndef SIMPLETAG_HXX_INCLUDED
#define SIMPLETAG_HXX_INCLUDED

#include "itag.hxx"

/***************************   simple tag readers   ***************************/

/** Implements the ITag interface for
    building a general info that is not a compound tag.
*/
class CSimpleTag : public ITag
{
    public:
        CSimpleTag(){};
        CSimpleTag( const XmlTagAttributes_t& attributes ):m_SimpleAttributes(attributes){};

        virtual void startTag();
        virtual void endTag();
        virtual void addCharacters(const std::wstring& characters);
        virtual void addAttributes(const XmlTagAttributes_t& attributes);
        virtual std::wstring getTagContent();

        virtual ::std::wstring const getTagAttribute( ::std::wstring  const & attrname );

    private:
        std::wstring       m_SimpleContent;
        XmlTagAttributes_t m_SimpleAttributes;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
