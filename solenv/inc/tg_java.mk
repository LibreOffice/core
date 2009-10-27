#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: tg_java.mk,v $
#
# $Revision: 1.13 $
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



$(CLASSDIR)/com/sun/star/upd/$(VERSIONINFOFILE)_updversion.class .PHONY: $(MISC)/java/com/sun/star/upd/$(VERSIONINFOFILE)_updversion.java 

$(MISC)/java/com/sun/star/upd/$(VERSIONINFOFILE)_updversion.java .PHONY:
    @echo "Making:   " $(@:f)
    @-$(MKDIRHIER) $(MISC)/java/com/sun/star/upd
.IF "$(GUI)"=="UNX"
    @echo package com.sun.star.upd\; > $@
    @echo 'public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";}' >> $@
.ELSE			# "$(GUI)"=="UNX"
    @echo package com.sun.star.upd; > $@
    @echo public class $(VERSIONINFOFILE)_updversion { public static String Version = "$(RSCREVISION)";} >> $@
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(JAVATARGET)"!=""
.IF "$(PACKAGE)"!=""
$(CLASSDIR)/$(IDLPACKAGE)/%.class .NOINFER .IGNORE : %.java
#	echo $@
    @@-$(RM) $(JAVATARGET)
.ELSE			# "$(PACKAGE)"!=""
$(CLASSDIR)/%.class .NOINFER .IGNORE : %.java
#	echo $@
    @@-$(RM) $(JAVATARGET)
.ENDIF			# "$(PACKAGE)"!=""

$(JAVATARGET) :	$(JAVAFILES) $(JAVACLASSFILES) 
.IF "$(JAVARESPONSE)"!=""
    $(JAVAC) @<<
    $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
    <<keep
.ELSE			# "$(JAVARESPONSE)"!=""
.IF "$(use_jdep)"!=""
    $(JAVAC) -depend $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(use_jdep)"!=""
.IF "$(javauno)"!=""
.IF "$(JAVAFILES:d)"==""
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) $(JAVAFILES)
.ELSE			# "$(JAVAFILES:d)"==""
    @@$(TOUCH) $(INPATH)_$(VCSID)_a_dummy.java
    $(JAVAC) $(JAVACPS) $(CLASSPATH) -d $(CLASSDIR) $(JAVAFLAGS) ./*.java $(uniq $(JAVAFILES:d:+"*.java"))
    @@-$(RM) $(INPATH)_$(VCSID)_a_dummy.java
.ENDIF			# "$(JAVAFILES:d)"==""
.ELSE			# "$(javauno)"!=""
    $(JAVAC) $(JAVACPS) "$(CLASSPATH)" -d $(CLASSDIR) $(JAVAFLAGS) @$(mktmp $(strip $(JAVAFILES)))
.ENDIF			# "$(javauno)"!=""
.ENDIF			# "$(use_jdep)"!=""
.ENDIF			# "$(JAVARESPONSE)"!=""
.IF "$(UPDATER)"!=""
.IF "$(OS)$(CPU)"=="SOLARISS"
    @@-find $(CLASSDIR) -type d -user $(USER) \! -perm -5 -print | xargs chmod a+r $$1
.ENDIF
.ENDIF
    @echo > $@

.ENDIF			# "$(JAVATARGET)"!=""

