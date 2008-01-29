/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fixedhyperbase.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 15:04:30 $
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

#ifndef TOOLKIT_INC_TOOLKIT_HELPER_FIXEDHYPERBASE_HXX
#define TOOLKIT_INC_TOOLKIT_HELPER_FIXEDHYPERBASE_HXX

#ifndef TOOLKIT_DLLAPI_H
#include <toolkit/dllapi.h>
#endif

#include <vcl/fixed.hxx>

//........................................................................
namespace toolkit
{
//........................................................................

    class TOOLKIT_DLLPUBLIC FixedHyperlinkBase : public FixedText
    {
    public:
        FixedHyperlinkBase( Window* pParent, const ResId& rId );
        FixedHyperlinkBase( Window* pParent, WinBits nWinStyle );
        virtual ~FixedHyperlinkBase();

        virtual void    SetURL( const String& rNewURL );
        virtual String  GetURL() const;
        virtual void    SetDescription( const String& rNewDescription );
    };

//........................................................................
} // namespace toolkit
//........................................................................

#endif // TOOLKIT_INC_TOOLKIT_HELPER_FIXEDHYPERBASE_HXX

