#*************************************************************************
#
#  $RCSfile: makefile.mk,v $
#
#  $Revision: 1.4 $
#
#  last change: $Author: kz $ $Date: 2008-03-07 16:58:16 $
#
#  The Contents of this file are made available subject to the terms of
#  the BSD license.
#  
#  Copyright (c) 2003 by Sun Microsystems, Inc.
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions
#  are met:
#  1. Redistributions of source code must retain the above copyright
#     notice, this list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright
#     notice, this list of conditions and the following disclaimer in the
#     documentation and/or other materials provided with the distribution.
#  3. Neither the name of Sun Microsystems, Inc. nor the names of its
#     contributors may be used to endorse or promote products derived
#     from this software without specific prior written permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
#  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
#  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
#  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
#  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#     
#**************************************************************************

# Builds the Java Canvas implementation.

PRJNAME = avmedia
PRJ     = ..$/..
TARGET  = avmedia
PACKAGE = avmedia

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(GUIBASE)"=="javamedia"

JAVAFILES  = \
    Manager.java            \
    Player.java             \
    PlayerWindow.java       \
    WindowAdapter.java      \
    MediaUno.java           \
    FrameGrabber.java       \
    x11$/SystemWindowAdapter.java   

JARFILES        = sandbox.jar jurt.jar unoil.jar ridl.jar juh.jar java_uno.jar jmf.jar
JAVACLASSFILES  = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java//).class)

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
CUSTOMMANIFESTFILE      = manifest

.ENDIF     # "$(GUIBASE)"=="javamedia" 

# --- Targets ------------------------------------------------------

.INCLUDE: target.mk
