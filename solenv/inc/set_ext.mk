#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: set_ext.mk,v $
#
#   $Revision: 1.6 $
#
#   last change: $Author: rt $ $Date: 2005-09-08 09:40:07 $
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

#defaults
TARFILE_ROOTDIR*=$(TARFILE_NAME)
UNTAR_FLAG_FILE*=so_unpacked_$(TARGET)
ADD_FILES_FLAG_FILE*=so_addfiles_$(TARGET)
PATCH_FLAG_FILE*=so_patched_$(TARGET)
CONFIGURE_FLAG_FILE*=so_configured_$(TARGET)
BUILD_FLAG_FILE*=so_built_$(TARGET)
INSTALL_FLAG_FILE*=so_installed_$(TARGET)
PREDELIVER_FLAG_FILE*=so_predeliver_$(TARGET)

