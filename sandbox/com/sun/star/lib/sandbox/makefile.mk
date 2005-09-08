#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.7 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 17:10:56 $
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

PRJ 	= ..$/..$/..$/..$/..
PRJNAME = sandbox
TARGET  = lib_sandbox
PACKAGE = com$/sun$/star$/lib$/sandbox

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

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

.INCLUDE :  target.mk

.ENDIF
