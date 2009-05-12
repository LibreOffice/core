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
# $Revision: 1.10 $
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

PRJ=..$/..$/..$/..

PRJNAME=offapi
TARGET=csssetup
PACKAGE=com$/sun$/star$/setup

# --- Settings -----------------------------------------------------
.INCLUDE :  $(PRJ)$/util$/makefile.pmk

# ------------------------------------------------------------------

IDLFILES=\
    UpdateType.idl			\
    ActionType.idl			\
    BaseAction.idl			\
    CopyFileAction.idl		\
    DeleteDirAction.idl		\
    DeleteFileAction.idl		\
    DeleteFolderAction.idl		\
    DeleteFolderItemAction.idl	\
    DownloadAction.idl		\
    FontAction.idl	     		\
    InstallEnvironment.idl		\
    InstallResponse.idl		\
    InstallType.idl			\
    MakeDirAction.idl		\
    MakeFolderAction.idl		\
    MakeFolderItemAction.idl	\
    MakeShortcutAction.idl		\
    MirrorEntry.idl			\
    ModuleInfo.idl			\
    ModuleState.idl			\
    OSType.idl			\
    ProductRegistration.idl		\
    ProfileItemAction.idl		\
    Setup.idl			\
    SizeInfo.idl			\
    UnzipAction.idl			\
    VersionIdentifier.idl		\
    WindowsRegistryAction.idl	\
    XSetup.idl

# ------------------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE :  $(PRJ)$/util$/target.pmk

