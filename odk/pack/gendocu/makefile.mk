#*************************************************************************
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: kz $ $Date: 2003-08-27 16:48:16 $
#
#   The Contents of this file are made available subject to the terms of
#   either of the following licenses
#
#          - GNU Lesser General Public License Version 2.1
#          - Sun Industry Standards Source License Version 1.1
#
#   Sun Microsystems Inc., October, 2000
#
#   GNU Lesser General Public License Version 2.1
#   =============================================
#   Copyright 2000 by Sun Microsystems, Inc.
#   901 San Antonio Road, Palo Alto, CA 94303, USA
#
#   This library is free software; you can redistribute it and/or
#   modify it under the terms of the GNU Lesser General Public
#   License version 2.1, as published by the Free Software Foundation.
#
#   This library is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#   Lesser General Public License for more details.
#
#   You should have received a copy of the GNU Lesser General Public
#   License along with this library; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#   MA  02111-1307  USA
#
#
#   Sun Industry Standards Source License Version 1.1
#   =================================================
#   The contents of this file are subject to the Sun Industry Standards
#   Source License Version 1.1 (the "License"); You may not use this file
#   except in compliance with the License. You may obtain a copy of the
#   License at http://www.openoffice.org/license.html.
#
#   Software provided under this License is provided on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
#   WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
#   MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
#   See the License for the specific provisions governing your rights and
#   obligations concerning the Software.
#
#   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
#
#   Copyright: 2000 by Sun Microsystems, Inc.
#
#   All Rights Reserved.
#
#   Contributor(s): _______________________________________
#
#
#
#*************************************************************************

PRJ=..$/..
PRJNAME=odk
TARGET=gendocu

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

CPPDOCREFNAME="UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) C/C++ API Reference"
JAVADOCREFNAME="UDK $(UDK_MAJOR).$(UDK_MINOR).$(UDK_MICRO) Java API Reference"

CPP_DOCU_INDEX_FILE = $(DESTDIRGENCPPREF)$/index.html
JAVA_DOCU_INDEX_FILE = $(DESTDIRGENJAVAREF)$/index.html

JAVA_SRC_DIR=$(MISC)$/java_src
JAVA_PACKAGES=\
        com.sun.star.comp.helper \
        com.sun.star.lib.uno.helper \
        com.sun.star.tools.uno \
        com.sun.star.uno
#        com.sun.star.beans \

JAVA_SRC_FILES=\
        $(JAVA_SRC_DIR)$/jurt_src.zip \
        $(JAVA_SRC_DIR)$/jut_src.zip \
        $(JAVA_SRC_DIR)$/juh_src.zip \
        $(JAVA_SRC_DIR)$/ridl_src.zip \
        $(JAVA_SRC_DIR)$/ridl2_src.zip \
        $(JAVA_SRC_DIR)$/sandbox_src.zip 

JAVA_BEAN_SRC_FILES=\
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/ContainerFactory.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/JavaWindowPeerFake.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/LocalOfficeConnection.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/LocalOfficeWindow.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/NativeConnection.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/NativeService.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/OfficeConnection.java \
        $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/OfficeWindow.java

AUTODOCPARAMS= -lg c++ \
        -p sal $(INCOUT) -t sal -t osl -t rtl \
        -p store $(INCOUT) -t store \
        -p registry $(INCOUT) -t registry \
        -p cppu $(INCOUT) -t cppu -t com -t typelib -t uno \
        -p cppuhelper $(INCOUT) -t cppuhelper \
        -p bridges $(INCOUT) -t bridges

JAVADOCPARAMS= -use -splitindex -windowtitle "Java UNO Runtime Reference" -header $(JAVADOCREFNAME) -d $(DESTDIRGENJAVAREF) -sourcepath $(JAVA_SRC_DIR) -linkoffline ../../common/ref ./uno -linkoffline http://java.sun.com/j2se/1.4.1/docs/api ./java $(JAVA_PACKAGES)
    
JAVADOCLOG = $(MISC)$/javadoc_log.txt

MY_AUTODOC=$(WRAPCMD) $(SOLARBINDIR)$/autodoc

all: \
    $(CPP_DOCU_INDEX_FILE) \
    $(JAVA_DOCU_INDEX_FILE)

$(CPP_DOCU_INDEX_FILE) : $(INCLUDELIST)
    +-$(MKDIRHIER) $(@:d)        
    +$(MY_AUTODOC) -html $(DESTDIRGENCPPREF) -name $(CPPDOCREFNAME) $(AUTODOCPARAMS)

$(JAVA_SRC_DIR)$/%.zip : $(SOLARCOMMONBINDIR)$/%.zip
    +-$(MKDIRHIER) $(@:d)        
    +$(MY_COPY) $< $@
    +cd $(JAVA_SRC_DIR) && unzip -quod . $(@:f)

#$(JAVA_SRC_DIR)$/ridl2_src.zip : $(SOLARCOMMONBINDIR)$/ridl2_src.zip
#	+$(MY_COPY) $(SOLARCOMMONBINDIR)$/ridl2_src.zip $@
#	+cd $(JAVA_SRC_DIR) && unzip -quod . $(@:f)
#	+-rm $(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/*.java >& $(NULLDEV)
     
#$(JAVA_SRC_DIR)$/com$/sun$/star$/beans$/%.java : $(PRJ)$/source$/bean$/com$/sun$/star$/beans$/%.java 
#	+-$(MKDIRHIER) $(@:d)        
#	+$(MY_COPY) $< $@

#$(JAVA_DOCU_INDEX_FILE) : $(JAVA_SRC_FILES) $(JAVA_BEAN_SRC_FILES)
$(JAVA_DOCU_INDEX_FILE) : $(JAVA_SRC_FILES)
    +-$(MKDIRHIER) $(@:d)        
    +javadoc -J-Xmx120m $(JAVADOCPARAMS) > $(JAVADOCLOG)
