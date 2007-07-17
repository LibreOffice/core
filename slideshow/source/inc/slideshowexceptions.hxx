/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: slideshowexceptions.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:15:59 $
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

#ifndef INCLUDED_SLIDESHOW_SLIDESHOWEXCEPTIONS_HXX
#define INCLUDED_SLIDESHOW_SLIDESHOWEXCEPTIONS_HXX


namespace slideshow
{
    class SlideShowException {};

    /** This exception is thrown, when the Shape class was not
        able to convert an API object into our internal
        representation.
     */
    struct ShapeLoadFailedException : public SlideShowException {};

    /** This exception is thrown, when the SMIL arithmetic expression
        parser failed to parse a string.
     */
    struct ParseError : public SlideShowException
    {
        ParseError() {}
        explicit ParseError( const char* ) {}
    };

}

#endif /* INCLUDED_SLIDESHOW_SLIDESHOWEXCEPTIONS_HXX */
