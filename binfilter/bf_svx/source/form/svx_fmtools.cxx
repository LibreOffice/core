/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "fmtools.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XIndexAccess.hpp>

namespace binfilter {

using namespace ::com::sun::star;

/*N*/ sal_Int32 getElementPos(const uno::Reference< container::XIndexAccess>& xCont, const uno::Reference< uno::XInterface >& xElement)
/*N*/ {
/*N*/ 	sal_Int32 nIndex = -1;
/*N*/ 	if (!xCont.is())
/*N*/ 		return nIndex;
/*N*/
/*N*/   uno::Reference< uno::XInterface > xNormalized( xElement, uno::UNO_QUERY );
/*N*/ 	DBG_ASSERT( xNormalized.is(), "getElementPos: invalid element!" );
/*N*/ 	if ( xNormalized.is() )
/*N*/ 	{
/*N*/ 		// Feststellen an welcher Position sich das Kind befindet
/*N*/ 		nIndex = xCont->getCount();
/*N*/ 		while (nIndex--)
/*N*/ 		{
/*N*/ 			try
/*N*/ 			{
/*N*/               uno::Reference< uno::XInterface > xCurrent;
/*N*/ 				xCont->getByIndex( nIndex ) >>= xCurrent;
/*N*/               DBG_ASSERT( xCurrent.get() == uno::Reference< uno::XInterface >( xCurrent, uno::UNO_QUERY ).get(),
/*N*/ 					"getElementPos: container element not normalized!" );
/*N*/ 				if ( xNormalized.get() == xCurrent.get() )
/*N*/ 					break;
/*N*/ 			}
/*N*/           catch(uno::Exception&)
/*N*/ 			{
/*N*/ 				DBG_ERROR( "getElementPos: caught an exception!" );
/*N*/ 			}
/*N*/
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return nIndex;
/*N*/ }

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
