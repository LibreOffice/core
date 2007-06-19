/***************************************************************************
 *   Copyright (C) 2006 by Jocelyn Merand                                  *
 *   joc.mer@gmail.com                                                     *
 *                                                                         *
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: altstrfunc.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-06-19 16:00:38 $
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

#ifndef _ALT_STRFUNC_HXX_
#define _ALT_STRFUNC_HXX_

#include <string>
#include <guess.hxx>

inline bool isSeparator(const char c){
    return c == GUESS_SEPARATOR_OPEN || c == GUESS_SEPARATOR_SEP || c == GUESS_SEPARATOR_CLOSE || c == '\0';
}

std::string     upperCase(const std::string &s);
int             start(const std::string &s1, const std::string &s2);

#endif

