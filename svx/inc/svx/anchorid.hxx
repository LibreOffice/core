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
#ifndef _SVX_ANCHORID_HXX
#define _SVX_ANCHORID_HXX


enum SvxAnchorIds
{
    SVX_OBJ_AT_CNTNT    = 0x01,     // Frame bound to paragraph
    SVX_OBJ_IN_CNTNT    = 0x02,     // Frame bound to character
    SVX_OBJ_PAGE        = 0x04,     // Frame bound to page
    SVX_OBJ_AT_FLY      = 0x08      // Frame bound to other frame
//  SVX_OBJ_AUTO_CNTNT  = 0x10      // Automatically positioned frame bound to paragraph
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
