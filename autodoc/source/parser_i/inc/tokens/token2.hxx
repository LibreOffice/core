/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: token2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:10:09 $
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

#ifndef ADC_TOKEN2_HXX
#define ADC_TOKEN2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETRS



/** This is the interface for parser classes, which get a sequence of Token s from
    a text.

    Start() starts to parse the text from the given i_rSource.
    GetNextToken() returns a Token on the heap as long as there are
    still characters in the text left. The last time GetNextToken()
    returns NULL.

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/
class TextToken
{
  public:
    // LIFECYCLE
    virtual             ~TextToken() {}


    // INQUIRY
    virtual const char* Text() const = 0;
};


#endif


