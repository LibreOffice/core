#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.1 $
#
#   last change: $Author: rene $ $Date: 2008-02-04 09:02:06 $
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

PRJ=..$/..

PRJNAME=apache-commons
TARGET=commons-httpclient

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# override buildfile
ANT_BUILDFILE=build.xml

.INCLUDE : antsettings.mk

.IF "$(SOLAR_JAVA)" != ""
# --- Files --------------------------------------------------------

TARFILE_NAME=commons-httpclient-3.1-src

TARFILE_ROOTDIR=commons-httpclient-3.1

#PATCH_FILE_NAME=patches$/httpclient.patch

#CONVERTFILES=build.xml
                
OUT2CLASS=dist$/commons-httpclient.jar

COMMONS_LOGGING_JAR=..$/..$/..$/..$/$(INPATH)$/class$/commons-logging-1.1.1-SNAPSHOT.jar
COMMONS_CODEC_JAR=..$/..$/..$/..$/$/$(INPATH)$/class$/commons-codec-1.3.jar

.IF "$(JAVACISGCJ)"=="yes"
JAVA_HOME=
.EXPORT : JAVA_HOME
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)" -Dbuild.compiler=gcj -Dcommons-logging.jar=$(COMMONS_LOGGING_JAR) -Dcommons-codec.jar=$(COMMONS_CODEC_JAR) -f $(ANT_BUILDFILE) dist
.ELSE
BUILD_ACTION=$(ANT) -Dbuild.label="build-$(RSCREVISION)"  -Dcommons-logging.jar=$(COMMONS_LOGGING_JAR) -Dcommons-codec.jar=$(COMMONS_CODEC_JAR) -f $(ANT_BUILDFILE) dist
.ENDIF

.ENDIF # $(SOLAR_JAVA)!= ""

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk

.IF "$(SOLAR_JAVA)" != ""
.INCLUDE : tg_ext.mk
.ENDIF

