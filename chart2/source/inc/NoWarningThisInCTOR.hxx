/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: NoWarningThisInCTOR.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-05-22 18:20:30 $
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
#ifndef CHART2_NOWARNINGTHISINCTOR_HXX
#define CHART2_NOWARNINGTHISINCTOR_HXX

/** Include this file, if you have to use "this" in the base initializer list of
    a constructor.

    Include it only, if the usage of this is unavoidable, like in the
    initialization of controls in a dialog.

    Do not include this header in other header files, because this might result
    in unintended suppression of the warning via indirect inclusion.
 */

#ifdef _MSC_VER
// warning C4355: 'this' : used in base member initializer list
#  pragma warning (disable : 4355)
#endif

// CHART2_NOWARNINGTHISINCTOR_HXX
#endif
