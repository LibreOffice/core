/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StdTypeDefs.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:58:13 $
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
#ifndef CONNECTIVITY_STDTYPEDEFS_HXX
#define CONNECTIVITY_STDTYPEDEFS_HXX

namespace connectivity
{
    typedef ::std::vector< ::rtl::OUString>         TStringVector;
    typedef ::std::vector< sal_Int32>               TIntVector;
    typedef ::std::map<sal_Int32,sal_Int32>         TInt2IntMap;
    typedef ::std::map< ::rtl::OUString,sal_Int32>  TString2IntMap;
    typedef ::std::map< sal_Int32,::rtl::OUString>  TInt2StringMap;
}

#endif // CONNECTIVITY_STDTYPEDEFS_HXX

