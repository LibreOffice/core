#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: makefile.mk,v $
#
#   $Revision: 1.11 $
#
#   last change: $Author: rt $ $Date: 2005-09-09 01:53:03 $
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
PRJ=..
PRJNAME=scriptinginstaller
TARGET=scriptinginstaller
no_common_build_zip=true

.INCLUDE : settings.mk

.IF "$(SOLAR_JAVA)"!=""
ZIP1FLAGS=-r
ZIP1TARGET=ScriptsJava
ZIP1DIR=$(OUT)$/class$/examples
ZIP1LIST=java
.ENDIF

ZIP2FLAGS=-r
ZIP2TARGET=ScriptsBeanShell
ZIP2LIST=beanshell/HelloWorld beanshell/Highlight beanshell/WordCount beanshell/Capitalise beanshell/MemoryUsage 

ZIP3FLAGS=-r
ZIP3TARGET=ScriptsJavaScript
ZIP3LIST=javascript

ZIP4FLAGS=-r
ZIP4TARGET=ScriptsPython
ZIP4LIST=python/HelloWorld.py python/Capitalise.py python/pythonSamples

ZIP5FLAGS=-r
ZIP5TARGET=scriptbindinglib
ZIP5DIR=$(PRJ)$/workben$/bindings
ZIP5LIST=*.xlb Highlight.xdl

.INCLUDE : target.mk
