#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.4 $
#
#   last change: $Author: rt $ $Date: 2005-09-07 20:34:24 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2005 by Sun Microsystems, Inc.
#     901 San Antonio Road, Palo Alto, CA 94303, USA
#
#     This library is free software; you can redistribute it and/or
#     modify it under the terms of the GNU Lesser General Public
#     License version 2.1, as published by the Free Software Foundation.
#
#     This library is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#     Lesser General Public License for more details.
#
#     You should have received a copy of the GNU Lesser General Public
#     License along with this library; if not, write to the Free Software
#     Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#     MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..$/..$/..

PRJNAME=api

TARGET=cssaccessibility
PACKAGE=com$/sun$/star$/accessibility

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AccessBridge.idl                                \
    Accessible.idl                                  \
    AccessibleContext.idl                           \
    AccessibleEventId.idl                           \
    AccessibleEventObject.idl                       \
    AccessibleRelation.idl                          \
    AccessibleRelationType.idl                      \
    AccessibleRole.idl                              \
    AccessibleStateType.idl                         \
    AccessibleTableModelChange.idl                  \
    AccessibleTableModelChangeType.idl              \
    AccessibleTextType.idl                          \
    IllegalAccessibleComponentStateException.idl    \
    TextSegment.idl                                 \
    XAccessible.idl                                 \
    XAccessibleAction.idl                           \
    XAccessibleComponent.idl                        \
    XAccessibleContext.idl                          \
    XAccessibleEditableText.idl                     \
    XAccessibleEventBroadcaster.idl                 \
    XAccessibleEventListener.idl                    \
    XAccessibleExtendedComponent.idl                \
    XAccessibleHyperlink.idl                        \
    XAccessibleHypertext.idl                        \
    XAccessibleImage.idl                            \
    XAccessibleKeyBinding.idl                       \
    XAccessibleRelationSet.idl                      \
    XAccessibleSelection.idl                        \
    XAccessibleStateSet.idl                         \
    XAccessibleTable.idl                            \
    XAccessibleText.idl                             \
    XAccessibleValue.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
