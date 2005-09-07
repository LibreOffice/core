/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ignoreBaFa_ja_JP.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:27:07 $
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

#define TRANSLITERATION_BaFa_ja_JP
#include <transliteration_Ignore.hxx>

using namespace com::sun::star::uno;
using namespace rtl;

namespace com { namespace sun { namespace star { namespace i18n {

static Mapping BaFa[] = {
        { 0x30F4, 0x30A1, 0x30D0, sal_True },
        { 0x3094, 0x3041, 0x3070, sal_True },
        { 0x30D5, 0x30A1, 0x30CF, sal_True },
        { 0x3075, 0x3041, 0x306F, sal_True },
        { 0, 0, 0, sal_True }
};

ignoreBaFa_ja_JP::ignoreBaFa_ja_JP()
{
        func = (TransFunc) 0;
        table = 0;
        map = BaFa;
        transliterationName = "ignoreBaFa_ja_JP";
        implementationName = "com.sun.star.i18n.Transliteration.ignoreBaFa_ja_JP";
}

} } } }
