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

#ifndef _SVX_FRMDIR_HXX
#define _SVX_FRMDIR_HXX

// ----------------------------------------------------------------------------
namespace binfilter {

/** Defines possible text directions in frames. */
enum SvxFrameDirection
{
    /** Horizontal, from left to right, from top to bottom
        (typical for western languages). */
    FRMDIR_HORI_LEFT_TOP,

    /** Horizontal, from right to left, from top to bottom
        (typical for ararbic/hebrew languages). */
    FRMDIR_HORI_RIGHT_TOP,

    /** Vertical, from top to bottom, from right to left
        (typical for asian languages). */
    FRMDIR_VERT_TOP_RIGHT,

    /** Vertical, from top to bottom, from left to right
        (typical for mongol language). */
    FRMDIR_VERT_TOP_LEFT,

    /** Use the value from the environment, can only be used in frames. */
    FRMDIR_ENVIRONMENT
};

// ----------------------------------------------------------------------------

}//end of namespace binfilter
#endif // #ifndef _SVX_FRMDIR_HXX

