/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DragActionConversion.cxx,v $
 * $Revision: 1.3 $
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

#include "DragActionConversion.hxx"
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>


using namespace com::sun::star::datatransfer::dnd;


/* Convert office drag actions as defined in
   <type>com::sun::star::datatransfer::dnd::DNDConstants</type>
   into system conform drag actions.
 */
unsigned int OfficeToSystemDragActions(sal_Int8 dragActions)
{
  unsigned int actions = NSDragOperationNone;

  if (dragActions & DNDConstants::ACTION_COPY)
    {
      actions |= NSDragOperationCopy;
    }

  if (dragActions & DNDConstants::ACTION_MOVE)
    {
      actions |= NSDragOperationMove;
    }

  if (dragActions & DNDConstants::ACTION_LINK)
    {
      actions |= NSDragOperationLink;
    }

  return actions;
}

/* Convert system conform drag actions into office conform
   drag actions as defined in
   <type>com::sun::star::datatransfer::dnd::DNDConstants</type>.
 */
sal_Int8 SystemToOfficeDragActions(unsigned int dragActions)
{
  sal_Int8 actions = DNDConstants::ACTION_NONE;

  if (dragActions & NSDragOperationCopy)
    {
      actions |= DNDConstants::ACTION_COPY;
    }

  if (dragActions & NSDragOperationMove)
    {
      actions |= DNDConstants::ACTION_MOVE;
    }

  if (dragActions & NSDragOperationLink)
    {
      actions |= DNDConstants::ACTION_LINK;
    }

  // We map NSDragOperationGeneric to ACTION_DEFAULT to
  // signal that we have to decide for a drag action
  if (dragActions & NSDragOperationGeneric)
    {
      actions |= DNDConstants::ACTION_DEFAULT;
    }

  return actions;
}
