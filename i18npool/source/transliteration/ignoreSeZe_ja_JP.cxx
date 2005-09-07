/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ignoreSeZe_ja_JP.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:29:53 $
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

// prevent internal compiler error with MSVC6SP3
#include <utility>

#define TRANSLITERATION_SeZe_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

static Mapping SeZe[] = {
        // SI + E --> SE
        { 0x30B7, 0x30A7, 0x30BB, sal_True },
        // SI + E --> SE
        { 0x3057, 0x3047, 0x305B, sal_True },
        // ZI + E --> ZE
        { 0x30B8, 0x30A7, 0x30BC, sal_True },
        // ZI + E --> ZE
        { 0x3058, 0x3047, 0x305C, sal_True },

        { 0, 0, 0, sal_True }
};

ignoreSeZe_ja_JP::ignoreSeZe_ja_JP()
{
        func = (TransFunc) 0;
        table = 0;
        map = SeZe;
        transliterationName = "ignoreSeZe_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreSeZe_ja_JP";
}

} } } }
