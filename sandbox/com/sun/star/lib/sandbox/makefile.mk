#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: makefile.mk,v $
#
# $Revision: 1.8 $
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ 	= ..$/..$/..$/..$/..
PRJNAME = sandbox
TARGET  = lib_sandbox
PACKAGE = com$/sun$/star$/lib$/sandbox

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.IF "$(L10N_framework)"==""

.IF "$(JDK)" == "gcj"
all:
    @echo This dir cannot be build with gcj because of sun.applet.AppletAudioClip
.ELSE

JARFILES=

# --- Files --------------------------------------------------------

JAVAFILES=						    \
    AudioProxy.java				    \
    Cachable.java					\
    ClassContext.java	 			\
    ClassContextImpl.java 			\
    ClassContextProxy.java 		    \
    CodeSource.java				    \
    Disposable.java				    \
    ExecutionContext.java			\
    Holder.java					    \
    ImageProducerProxy.java		    \
    JarEntry.java					\
    JarInputStream.java			    \
    Permission.java				    \
    PermissionCollection.java		\
    ProtectionDomain.java			\
    Resource.java					\
    ResourceImpl.java				\
    ResourceProxy.java				\
    ResourceViewer.java    		    \
    SandboxMessageHandler.java		\
    SandboxThreadGroup.java		    \
    SandboxSecurity.java			\
    SandboxSecurityException.java	\
    WeakRef.java					\
    WeakTable.java					\
    WeakEntry.java					\
    XImage.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

RC_SUBDIRSDEPS=$(JAVATARGET)

# --- Targets ------------------------------------------------------
.ENDIF
.ENDIF

.INCLUDE :  target.mk

