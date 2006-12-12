#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: perl.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: kz $ $Date: 2006-12-12 16:36:14 $
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

.IF "$(GUI)"=="WNT"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/perl$/lib$/core
PERL_LIB=$(BUILD_TOOLS)$/..$/perl$/lib$/core$/perl58.lib
.ENDIF 

.IF "$(OS)$(CPU)" == "LINUXI"
SOLARINC+=-I$(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/CORE
PERL_LIB=	-lcrypt \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/CORE$/libperl.a \
            $(BUILD_TOOLS)$/..$/..$/lib$/perl5$/5.8.3$/i686-linux$/auto/DynaLoader/DynaLoader.a
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

