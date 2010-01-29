#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: perl.mk,v $
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

.IF "$(GUI)"=="WNT"
.IF "$(USE_SHELL)"=="4nt"
SOLARINC+=-I$(BUILD_TOOLS)$/perl$/lib$/core
PERL_LIB=$(BUILD_TOOLS)$/perl$/lib$/core$/perl58.lib
.ELSE			# "$(USE_SHELL)"=="4nt"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/perl$/lib$/core
PERL_LIB=$(BUILD_TOOLS)$/..$/perl$/lib$/core$/perl58.lib
.ENDIF			# "$(USE_SHELL)"=="4nt"
.ENDIF 

.IF "$(OS)$(CPU)" == "LINUXI"
    .IF "$(OUTPATH)" == "unxubti8" # Hack for Ububtu x86 builds in SO environment 
        SOLARINC+=-I$/usr$/lib$/perl$/5.8.8$/CORE
        PERL_LIB=	-lcrypt \
            $/usr$/lib$/libperl.a \
            $/usr$/lib$/perl$/5.8.8$/auto/DynaLoader/DynaLoader.a
    .ELSE
        SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/CORE
        PERL_LIB=	-lcrypt \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/CORE$/libperl.a \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/auto/DynaLoader/DynaLoader.a
    .ENDIF
.ENDIF

.IF "$(OS)$(CPU)" == "SOLARISS"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/sun4-solaris$/CORE
PERL_LIB=	-lsocket \
            -lnsl \
            -ldl \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/sun4-solaris$/CORE$/shared$/libperl.so \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/sun4-solaris$/CORE$/shared$/DynaLoader.a
.ENDIF

.IF "$(OS)$(CPU)" == "SOLARISI"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i86pc-solaris$/CORE
PERL_LIB=	-lsocket \
            -lnsl \
            -ldl \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i86pc-solaris$/CORE$/shared$/libperl.so \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i86pc-solaris$/CORE$/shared$/DynaLoader.a 
.ENDIF 

.IF "$(OS)$(CPU)" == "MACOSXI"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/darwin-2level$/CORE
PERL_LIB=	\
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/darwin-2level$/CORE$/libperl.a \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/darwin-2level$/auto/DynaLoader/DynaLoader.a
.ENDIF

