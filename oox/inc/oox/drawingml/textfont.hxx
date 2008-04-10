/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: textfont.hxx,v $
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

#ifndef OOX_DRAWINGNML__TEXTFONT_HXX
#define OOX_DRAWINGNML__TEXTFONT_HXX

#include <rtl/ustring.hxx>

namespace oox { namespace drawingml {


    /** carries a CT_TextFont*/
    class TextFont
    {
    public:
        TextFont()
            : mnPitch( 0 ), mnCharset( 0 )
            {
            }
        /** return true if valid */
        bool is() const
            { return msTypeface.getLength() != 0; }
        ::rtl::OUString  msTypeface;
        ::rtl::OUString  msPanose;
        sal_Int32        mnPitch;
        sal_Int32        mnCharset;
    };

} }

#endif

