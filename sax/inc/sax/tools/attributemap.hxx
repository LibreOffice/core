/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: attributemap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:50:34 $
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

#ifndef _SAX_ATTRIBUTEMAP_HXX_
#define _SAX_ATTRIBUTEMAP_HXX_

#ifndef INCLUDED_SAX_DLLAPI_H
#include "sax/dllapi.h"
#endif

#include <map>

#ifndef _SAX_TOKENMAP_HXX_
#include "sax/tools/tokenmap.hxx"
#endif

namespace sax
{
    class SaxTokenMap;

    typedef std::map< sal_uInt32, rtl::OString > AttributeMapBase;

    /** a map for a set of xml attributes, identified with integer tokens.
        Attribute values are stored in utf-8 encoding. */
    class SAX_DLLPUBLIC AttributeMap : public AttributeMapBase
    {
    public:
        AttributeMap( const SaxTokenMap& rTokenMap );
        ~AttributeMap();

        /** returns a unicode string, if the token does not exists the string is empty */
        ::rtl::OUString getString( SaxToken nToken ) const;

        /** returns true if the attribute with the token nToken is part of this map */
        bool has( SaxToken nToken ) const;

        /** converts the attribute with the token nToken to sal_Int32 or returns
            nDefault if this attribute does not exists */
        sal_Int32 getInt32( SaxToken nToken, sal_Int32 nDefault = 0 ) const;

        /** converts the attribute with the token nToken to a token or returns
            nDefault if this attribute does not exists */
        sal_uInt32 getToken( SaxToken nToken, SaxToken nDefault ) const;

    private:
        const SaxTokenMap& mrTokenMap;
    };

}

#endif // _SAX_ATTRIBUTEMAP_HXX_
