/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textfont.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-17 08:05:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
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

