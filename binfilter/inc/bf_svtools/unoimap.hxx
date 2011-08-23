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

#ifndef _SVTOOLS_UNOIMAP_HXX
#define _SVTOOLS_UNOIMAP_HXX

#include "bf_svtools/svtdllapi.h"

#include <com/sun/star/uno/XInterface.hpp>

namespace binfilter
{

class ImageMap;
struct SvEventDescription;

 com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SvUnoImageMapRectangleObject_createInstance( const SvEventDescription* pSupportedMacroItems );
 com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SvUnoImageMapCircleObject_createInstance( const SvEventDescription* pSupportedMacroItems );
 com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SvUnoImageMapPolygonObject_createInstance( const SvEventDescription* pSupportedMacroItems );

 com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SvUnoImageMap_createInstance( const SvEventDescription* pSupportedMacroItems );
 com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SvUnoImageMap_createInstance( const ImageMap& rMap, const SvEventDescription* pSupportedMacroItems );
 sal_Bool SvUnoImageMap_fillImageMap( com::sun::star::uno::Reference< com::sun::star::uno::XInterface > xImageMap, ImageMap& rMap );

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
