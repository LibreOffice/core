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



PRJ=..$/..$/..

PRJNAME=sc
TARGET=miscdlgs
LIBTARGET=no
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  scpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  sc.mk
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES =  \
    $(SLO)$/delcldlg.obj		\
    $(SLO)$/inscldlg.obj		\
    $(SLO)$/delcodlg.obj		\
    $(SLO)$/inscodlg.obj		\
    $(SLO)$/strindlg.obj		\
    $(SLO)$/mtrindlg.obj		\
    $(SLO)$/lbseldlg.obj		\
    $(SLO)$/filldlg.obj			\
    $(SLO)$/autofmt.obj			\
    $(SLO)$/solvrdlg.obj		\
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/solverutil.obj      \
    $(SLO)$/mvtabdlg.obj		\
    $(SLO)$/groupdlg.obj		\
    $(SLO)$/tabopdlg.obj		\
    $(SLO)$/crdlg.obj			\
    $(SLO)$/namecrea.obj		\
    $(SLO)$/namepast.obj		\
    $(SLO)$/textdlgs.obj		\
    $(SLO)$/anyrefdg.obj		\
    $(SLO)$/crnrdlg.obj			\
    $(SLO)$/shtabdlg.obj		\
    $(SLO)$/instbdlg.obj		\
    $(SLO)$/acredlin.obj		\
    $(SLO)$/highred.obj			\
    $(SLO)$/simpref.obj			\
    $(SLO)$/redcom.obj			\
    $(SLO)$/linkarea.obj		\
    $(SLO)$/warnbox.obj	\
    $(SLO)$/scuiautofmt.obj     \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj \
    $(SLO)$/protectiondlg.obj \
    $(SLO)$/tabbgcolordlg.obj   \
    $(SLO)$/retypepassdlg.obj

EXCEPTIONSFILES = \
    $(SLO)$/acredlin.obj        \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj		\
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/crnrdlg.obj         \
    $(SLO)$/solverutil.obj \
    $(SLO)$/protectiondlg.obj \
    $(SLO)$/tabbgcolordlg.obj   \
    $(SLO)$/retypepassdlg.obj

SRS1NAME=$(TARGET)
SRC1FILES = \
    instbdlg.src				\
    acredlin.src				\
    highred.src					\
    linkarea.src                \
    conflictsdlg.src            \
    sharedocdlg.src \
    protectiondlg.src \
    retypepassdlg.src

LIB1TARGET = $(SLB)$/$(TARGET).lib

LIB1OBJFILES =  \
    $(SLO)$/autofmt.obj			\
    $(SLO)$/solvrdlg.obj		\
    $(SLO)$/optsolver.obj       \
    $(SLO)$/solveroptions.obj   \
    $(SLO)$/solverutil.obj      \
    $(SLO)$/tabopdlg.obj		\
    $(SLO)$/anyrefdg.obj		\
    $(SLO)$/crnrdlg.obj			\
    $(SLO)$/acredlin.obj		\
    $(SLO)$/highred.obj			\
    $(SLO)$/simpref.obj			\
    $(SLO)$/redcom.obj			\
    $(SLO)$/warnbox.obj         \
    $(SLO)$/conflictsdlg.obj    \
    $(SLO)$/sharedocdlg.obj \
    $(SLO)$/protectiondlg.obj \
    $(SLO)$/retypepassdlg.obj

# --- Tagets -------------------------------------------------------

.INCLUDE :  target.mk

