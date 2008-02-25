#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: dp_misc.mk,v $
#
#   $Revision: 1.5 $
#
#   last change: $Author: obo $ $Date: 2008-02-25 16:48:28 $
#
#   The Contents of this file are made available subject to
#   the terms of GNU Lesser General Public License Version 2.1.
#
#
#     GNU Lesser General Public License Version 2.1
#     =============================================
#     Copyright 2006 by Sun Microsystems, Inc.
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

# To be included after settings.mk

# Although the deployment shared library is a UNO component, it also exports
# some C++ functionality:
.IF "$(OS)" == "WNT"
.IF "$(COM)" == "GCC"
DEPLOYMENTMISCLIB = -ldeploymentmisc$(DLLPOSTFIX)
.ELSE
DEPLOYMENTMISCLIB = ideploymentmisc$(DLLPOSTFIX).lib
.ENDIF
.ELIF "$(OS)" == "OS2"
DEPLOYMENTMISCLIB = ideploymentmisc$(DLLPOSTFIX).lib
.ELSE
DEPLOYMENTMISCLIB = -ldeploymentmisc$(DLLPOSTFIX)
.ENDIF
