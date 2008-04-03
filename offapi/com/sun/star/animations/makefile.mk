#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2008-04-03 15:27:56 $
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

PRJNAME=offapi

TARGET=cssanimations
PACKAGE=com$/sun$/star$/animations

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------------

IDLFILES=\
    AnimationAdditiveMode.idl\
    AnimationCalcMode.idl\
    AnimationColorSpace.idl\
    AnimationEndSync.idl\
    AnimationFill.idl\
    AnimationNodeType.idl\
    AnimationRestart.idl\
    AnimationTransformType.idl\
    AnimationValueType.idl\
    Event.idl\
    EventTrigger.idl\
    TargetProperties.idl \
    TimeFilterPair.idl\
    Timing.idl\
    TransitionSubType.idl\
    TransitionType.idl\
    ValuePair.idl\
    XAnimate.idl\
    XAnimateColor.idl\
    XAnimateMotion.idl\
    XAnimateSet.idl\
    XAnimateTransform.idl\
    XAnimationNode.idl\
    XAnimationNodeSupplier.idl\
    XAudio.idl\
    XIterateContainer.idl\
    XTargetPropertiesCreator.idl \
    XTimeContainer.idl\
    XTransitionFilter.idl\
    XCommand.idl\
    XAnimationListener.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
