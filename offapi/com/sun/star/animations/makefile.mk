#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



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
