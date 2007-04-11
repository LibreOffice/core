/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: frmdir.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 15:54:47 $
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

#ifndef _SVX_FRMDIR_HXX
#define _SVX_FRMDIR_HXX

// ----------------------------------------------------------------------------

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

#endif // #ifndef _SVX_FRMDIR_HXX


