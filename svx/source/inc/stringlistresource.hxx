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

#ifndef SVX_STRINGLISTRESOURCE_HXX
#define SVX_STRINGLISTRESOURCE_HXX

#include <tools/rc.hxx>

#include <svx/svxdllapi.h>

#include <memory>

//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
    //= StringListResource
    //====================================================================
    /** loads a list of strings from a resource, where the resource is of type RSC_RESOURCE,
        and has sub resources of type string, numbered from 1 to n
    */
    class StringListResource : public Resource
    {
    public:
        SVX_DLLPUBLIC StringListResource( const ResId& _rResId );
        SVX_DLLPUBLIC ~StringListResource();

        inline void get( ::std::vector< String >& _rStrings )
        {
            _rStrings = m_aStrings;
        }


        /** returns the String with a given local resource id

            @param  _nResId
                The resource id. It will not be checked if this id exists.

            @return String
                The string.
        */
        String getString( sal_uInt16 _nResId )
        {
            return String( ResId( _nResId, *m_pResMgr ) );
        }

        size_t  size() const    { return m_aStrings.size(); }
        bool    empty() const   { return m_aStrings.empty(); }

        const String& operator[]( size_t _index ) const { return m_aStrings[ _index ]; }

    private:
        ::std::vector< String > m_aStrings;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_STRINGLISTRESOURCE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
