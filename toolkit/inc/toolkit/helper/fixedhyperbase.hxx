/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: fixedhyperbase.hxx,v $
 * $Revision: 1.3 $
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

#ifndef TOOLKIT_INC_TOOLKIT_HELPER_FIXEDHYPERBASE_HXX
#define TOOLKIT_INC_TOOLKIT_HELPER_FIXEDHYPERBASE_HXX

#include <toolkit/dllapi.h>

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

