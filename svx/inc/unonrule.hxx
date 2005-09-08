/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unonrule.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 19:28:17 $
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

#ifndef _SVX_UNONRULE_HXX
#define _SVX_UNONRULE_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XANYCOMPARE_HPP_
#include <com/sun/star/ucb/XAnyCompare.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SdrModel;
class SvxNumRule;

SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule( const SvxNumRule* pRule ) throw();
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > SvxCreateNumRule( SdrModel* pModel ) throw();
const SvxNumRule& SvxGetNumRule( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > xRule ) throw( ::com::sun::star::lang::IllegalArgumentException );
bool SvxGetNumRule( ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace > xRule, SvxNumRule& rNumRule );
SVX_DLLPUBLIC ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > SvxCreateNumRuleCompare() throw();

#endif


