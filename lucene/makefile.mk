#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.2 $
#
#   last change: $Author: rt $ $Date: 2008-06-30 06:51:24 $
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

PRJ=.

PRJNAME=so_lucene
TARGET=so_lucene

.IF "$(USE_JDK_VERSION)" == "140"
JDK_VERSION=140
JAVA_HOME=$(JDK14PATH)

XCLASSPATH:=$(JDK14PATH)$/jre/lib/rt.jar
CLASSPATH:=$(XCLASSPATH)
.ENDIF

ANT_BUILDFILE*=../../../../build.xml

# --- Settings -----------------------------------------------------

#.INCLUDE :	ant.mk
.INCLUDE :	settings.mk
.INCLUDE :	antsettings.mk
# --- Files --------------------------------------------------------

CLASSPATH!:=$(CLASSPATH)$(PATH_SEPERATOR)$(ANT_CLASSPATH)$(PATH_SEPERATOR)$(JAVA_HOME)$/lib$/tools.jar
.EXPORT : CLASSPATH

LUCENE_NAME=lucene-2.3.2

TARFILE_NAME=$(LUCENE_NAME)
PATCH_FILE_NAME=lucene.patch

ANT_FLAGS += -Dbasedir=../../../../$(INPATH)/misc/build/lucene-2.3.2

BUILD_DIR=.
BUILD_ACTION= ${ANT} ${ANT_FLAGS}

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk 

