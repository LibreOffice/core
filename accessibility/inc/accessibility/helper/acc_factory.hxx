/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: acc_factory.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:22:25 $
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

#ifndef ACCESSIBILITY_HELPER_FACTORY_HXX
#define ACCESSIBILITY_HELPER_FACTORY_HXX

#ifndef TOOLKIT_HELPER_ACCESSIBLE_FACTORY_HXX
#include <toolkit/helper/accessiblefactory.hxx>
#endif
#ifndef SVTOOLS_ACCESSIBLE_FACTORY_HXX
#include <svtools/accessiblefactory.hxx>
#endif

/** this is the entry point to retrieve a factory for the toolkit-level Accessible/Contexts supplied
    by this library

    This function implements the factory function needed in toolkit
    (of type GetStandardAccComponentFactory).
*/
extern "C" void* SAL_CALL getStandardAccessibleFactory();

/** this is the entry point to retrieve a factory for the svtools-level Accessible/Contexts supplied
    by this library

    This function implements the factory function needed in svtools
    (of type GetSvtAccessibilityComponentFactory).
*/
extern "C" void* SAL_CALL getSvtAccessibilityComponentFactory();

#endif // ACCESSIBILITY_HELPER_FACTORY_HXX
