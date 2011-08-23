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

#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDERINTERCEPTION_HPP_
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XINTERCEPTORINFO_HPP_
#include <com/sun/star/frame/XInterceptorInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif


#ifndef _SVSTDARR_ULONGS
#define _SVSTDARR_ULONGS
#endif

#ifndef _SVX_FMPROP_HRC
#include "fmprop.hrc"
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_H_
#include <cppuhelper/interfacecontainer.h>
#endif
#ifndef _CPPUHELPER_COMPBASE3_HXX_
#include <cppuhelper/compbase3.hxx>
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _CPPUHELPER_IMPLBASE2_HXX_
#include <cppuhelper/implbase2.hxx>
#endif
#ifndef _CPPUHELPER_COMPONENT_HXX_
#include <cppuhelper/component.hxx>
#endif

#ifndef _FM_IMPLEMENTATION_IDS_HXX_
#include "fmimplids.hxx"
#endif

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

