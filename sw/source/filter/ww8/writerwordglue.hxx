/*************************************************************************
 *
 *  $RCSfile: writerwordglue.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2003-09-01 12:40:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): cmc@openoffice.org
 *
 *
 ************************************************************************/

/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil -*- */
/// @HTML

#ifndef SW_WRITERWORDGLUE
#define SW_WRITERWORDGLUE

#ifndef WW_NEEDED_CAST_HXX
#include "needed_cast.hxx"
#endif

namespace sw
{
    namespace types
    {
        /** A static_cast style cast for conversion of word types to writer's

            There are a number of places where the winword types are larger
            than the writer equivalents requiring a cast to silence warnings.
            To avoid throwing away this useful information writer_cast is used
            to identify where writer's types are smaller than word's.

            Based on needed_cast it will compile time assert if the cast
            becomes unnecessary at any time in the future.

            @tplparam
            Ret the desired return type

            @tplparam
            Param the type of the in param

            @param
            in the value to cast from Param to Ret

            @return in casted to type Ret
        */
        template<typename Ret, typename Param> Ret writer_cast(Param in)
        {
            return ww::needed_cast<Ret, Param>(in);
        }

        /** A static_cast style cast for conversion of writer types to word's

            There are a number of places where the writer types are larger than
            the winword equivalents requiring a cast to silence warnings.  To
            avoid throwing away this useful information writer_cast is used to
            identify where word's types are smaller than writers's.

            Based on needed_cast it will compile time assert if the cast
            becomes unnecessary at any time in the future.

            @tplparam
            Ret the desired return type

            @tplparam
            Param the type of the in param

            @param
            in the value to cast from Param to Ret

            @return in casted to type Ret
        */
        template<typename Ret, typename Param> Ret msword_cast(Param in)
        {
            return ww::needed_cast<Ret, Param>(in);
        }


        /** If a page dimension is close to a standard page size, snap to it.

            Commonly a page dimension is slightly different from a standard
            page size, so close that its likely a rounding error after
            creeping in. Use this to snap to standard sizes when within a
            trivial distance from a standard size.

            @param
            nSize the dimension to test against standard dimensions

            @return New dimension to use, equal to nSize unless within a
            trivial amount of a standard page dimension

            @author
            <a href="mailto:cmc@openoffice.org">Caol&aacute;n McNamara</a>
        */
        long SnapPageDimension(long nSize) throw();
    }
}

#endif
/* vi:set tabstop=4 shiftwidth=4 expandtab: */
