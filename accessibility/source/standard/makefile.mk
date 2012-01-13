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



PRJ=..$/..

PRJNAME=accessibility
TARGET=standard

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

SLOFILES=   \
    $(SLO)$/accessiblemenubasecomponent.obj	\
    $(SLO)$/accessiblemenucomponent.obj \
    $(SLO)$/accessiblemenuitemcomponent.obj \
    $(SLO)$/floatingwindowaccessible.obj \
    $(SLO)$/vclxaccessiblebox.obj \
    $(SLO)$/vclxaccessiblebutton.obj \
    $(SLO)$/vclxaccessiblecheckbox.obj \
    $(SLO)$/vclxaccessiblecombobox.obj \
    $(SLO)$/vclxaccessibledropdowncombobox.obj \
    $(SLO)$/vclxaccessibledropdownlistbox.obj \
    $(SLO)$/vclxaccessibleedit.obj \
    $(SLO)$/vclxaccessiblefixedhyperlink.obj \
    $(SLO)$/vclxaccessiblefixedtext.obj \
    $(SLO)$/vclxaccessiblelist.obj \
    $(SLO)$/vclxaccessiblelistbox.obj \
    $(SLO)$/vclxaccessiblelistitem.obj \
    $(SLO)$/vclxaccessiblemenu.obj \
    $(SLO)$/vclxaccessiblemenubar.obj \
    $(SLO)$/vclxaccessiblemenuitem.obj \
    $(SLO)$/vclxaccessiblemenuseparator.obj \
    $(SLO)$/vclxaccessiblepopupmenu.obj \
    $(SLO)$/vclxaccessibleradiobutton.obj \
    $(SLO)$/vclxaccessiblescrollbar.obj \
    $(SLO)$/vclxaccessiblestatusbar.obj \
    $(SLO)$/vclxaccessiblestatusbaritem.obj \
    $(SLO)$/vclxaccessibletabcontrol.obj \
    $(SLO)$/vclxaccessibletabpage.obj \
    $(SLO)$/vclxaccessibletabpagewindow.obj \
    $(SLO)$/vclxaccessibletextcomponent.obj \
    $(SLO)$/vclxaccessibletextfield.obj \
    $(SLO)$/vclxaccessibletoolbox.obj \
    $(SLO)$/vclxaccessibletoolboxitem.obj 

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

