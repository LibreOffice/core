/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: quotedstring.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:02:28 $
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

#ifndef CONNECTIVITY_QUOTED_STRING_HXX
#define CONNECTIVITY_QUOTED_STRING_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

namespace connectivity
{
    //==================================================================
    // Ableitung von String mit ueberladenen GetToken/GetTokenCount-Methoden
    // Speziell fuer FLAT FILE-Format: Strings koennen gequotet sein
    //==================================================================
    class QuotedTokenizedString : public String
    {
    public:
        QuotedTokenizedString(){}

        xub_StrLen  GetTokenCount( sal_Unicode cTok = ';', sal_Unicode cStrDel = '\0' ) const;
        void        GetTokenSpecial( String& _rStr,xub_StrLen& nStartPos, sal_Unicode cTok = ';', sal_Unicode cStrDel = '\0' ) const;
    };
}

#endif // CONNECTIVITY_QUOTED_STRING_HXX
