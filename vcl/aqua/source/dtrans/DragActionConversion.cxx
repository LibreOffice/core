/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
