/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tokenmap.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 12:51:06 $
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

#ifndef _SAX_TOKENMAP_HXX_
#define _SAX_TOKENMAP_HXX_

#ifndef _RTL_REF_HXX_
#include <rtl/ref.hxx>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef INCLUDED_SAX_DLLAPI_H
#include "sax/dllapi.h"
#endif

#ifndef _SAX_OBJECT_HXX_
#include "sax/tools/saxobject.hxx"
#endif

namespace sax
{
    /** type for a token identifier */
    typedef sal_uInt32 SaxToken;

    /** this class maps a set of ascii/utf-8 strings to token identifier */
    class SAX_DLLPUBLIC SaxTokenMap : public SaxObject
    {
    public:
        /** constant do indicate an unknown token */
        const static SaxToken InvalidToken = (SaxToken)-1;

        /** returns the token identifier for the given ascii string or SaxTokenMap::InvalidToken */
        virtual SaxToken GetToken( const sal_Char* pChar, sal_uInt32 nLength  = 0 ) const = 0;

        /** returns the token identifier for the given unicode string or SaxTokenMap::InvalidToken */
        virtual SaxToken GetToken( const ::rtl::OUString& rToken ) const = 0;

        /** returns the unicode string for the given token identifier */
        virtual const ::rtl::OUString& GetToken( SaxToken nToken ) const = 0;

        /** returns if the given unicode string equals the given token identifier */
        bool IsToken( const ::rtl::OUString& rToken, SaxToken nToken ) const { return GetToken( rToken ) == nToken; }
    };

    /** reference type to a SaxTokenMap */
    typedef rtl::Reference< SaxTokenMap > SaxTokenMapRef;
}

#endif // _SAX_TOKENMAP_HXX_
