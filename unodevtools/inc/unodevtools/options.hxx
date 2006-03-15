/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: options.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2006-03-15 09:17:23 $
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

#ifndef INCLUDED_UNODEVTOOLS_OPTIONS_HXX
#define INCLUDED_UNODEVTOOLS_OPTIONS_HXX

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

namespace com { namespace sun { namespace star { namespace uno {
class RuntimeException;
} } } }

namespace unodevtools {

//-------------------------------------------------------------------------------
sal_Bool readOption( rtl::OUString * pValue, const sal_Char * pOpt,
                     sal_Int32 * pnIndex, const rtl::OUString & aArg)
    throw (com::sun::star::uno::RuntimeException);

//-------------------------------------------------------------------------------
sal_Bool readOption( sal_Bool * pbOpt, const sal_Char * pOpt,
                     sal_Int32 * pnIndex, const rtl::OUString & aArg);

} // end of namespace unodevtools

#endif // INCLUDED_UNODEVTOOLS_OPTIONS_HXX
