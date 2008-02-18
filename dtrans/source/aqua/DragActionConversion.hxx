/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DragActionConversion.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-02-18 14:46:49 $
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


#include <sal/types.h>

#include <premac.h>
#import <Cocoa/Cocoa.h>
#include <postmac.h>


/* Convert office drag actions as defined in
   <type>com::sun::star::datatransfer::dnd::DNDConstants</type>
   into system conform drag actions.
 */
unsigned int OfficeToSystemDragActions(sal_Int8 dragActions);

/* Convert system conform drag actions into office conform
   drag actions as defined in
   <type>com::sun::star::datatransfer::dnd::DNDConstants</type>.
 */
sal_Int8 SystemToOfficeDragActions(unsigned int dragActions);
