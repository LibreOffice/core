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
#ifndef _SVX_FMTOOLS_HXX
#define _SVX_FMTOOLS_HXX

#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>


#ifndef _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGS
#endif

#include "fmprop.hrc"
#include <cppuhelper/interfacecontainer.h>
#include <cppuhelper/compbase3.hxx>
#include <comphelper/uno3.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/component.hxx>

#include "fmimplids.hxx"

class Window;
namespace binfilter {



//==================================================================
// allgemeine Typen
//==================================================================
// displaying a database exception for the user
// display info about a simple ::com::sun::star::sdbc::SQLException
// display info about chained SQLExceptions

// StringConversion
namespace svxform
{
    // modes for the XModeSelector
    DECLARE_CONSTASCII_USTRING(DATA_MODE);
    DECLARE_CONSTASCII_USTRING(FILTER_MODE);

}	// namespace svxform


// Kopieren von Persistenten Objecten


// Suchen in einer Indexliste nach einem Element

sal_Int32 getElementPos(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess>& xCont, const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface>& xElement);




// ===================================================================================================
// = class CursorWrapper - eine Hilfsklasse, die parallel mit je einem ::com::sun::star::uno::Reference<XDatabaseUpdateCursor>,
//							XDatabaseBookmarkCursor und XDatabaseDirectCursor arbeitet
// ===================================================================================================



}//end of namespace binfilter
#endif // _SVX_FMTOOLS_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
