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
    MSAAService.idl                                 \
    TextSegment.idl                                 \
    XAccessible.idl                                 \
    XAccessibleAction.idl                           \
    XAccessibleComponent.idl                        \
    XAccessibleContext.idl                          \
    XAccessibleEditableText.idl                     \
    XAccessibleExtendedAttributes.idl               \
    XAccessibleEventBroadcaster.idl                 \
    XAccessibleEventListener.idl                    \
    XAccessibleExtendedComponent.idl                \
    XAccessibleGroupPosition.idl                    \
    XAccessibleHyperlink.idl                        \
    XAccessibleHypertext.idl                        \
    XAccessibleImage.idl                            \
    XAccessibleKeyBinding.idl                       \
    XAccessibleMultiLineText.idl                    \
    XAccessibleRelationSet.idl                      \
    XAccessibleSelection.idl                        \
    XAccessibleStateSet.idl                         \
    XAccessibleTable.idl                            \
    XAccessibleTableSelection.idl                   \
    XAccessibleText.idl                             \
    XAccessibleTextAttributes.idl                   \
    XAccessibleTextMarkup.idl                       \
    XAccessibleTextSelection.idl                    \
    XAccessibleGetAccFlowTo.idl		            \
    XAccessibleValue.idl                            \
    XMSAAService.idl


# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk
