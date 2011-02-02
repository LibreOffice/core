#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,fwe))

$(eval $(call gb_AllLangResTarget_set_reslocation,fwe,framework))

$(eval $(call gb_AllLangResTarget_add_srs,fwe,\
    fwe/fwk_classes \
    fwe/fwk_services \
))


$(eval $(call gb_SrsTarget_SrsTarget,fwe/fwk_classes))

$(eval $(call gb_SrsTarget_set_include,fwe/fwk_classes,\
    $$(INCLUDE) \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/classes \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_SrsTarget_add_files,fwe/fwk_classes,\
    framework/source/classes/resource.src \
))

$(eval $(call gb_SrsTarget_SrsTarget,fwe/fwk_services))

$(eval $(call gb_SrsTarget_set_include,fwe/fwk_services,\
    $$(INCLUDE) \
    -I$(SRCDIR)/framework/inc \
    -I$(SRCDIR)/framework/inc/services \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_SrsTarget_add_files,fwe/fwk_services,\
    framework/source/services/fwk_services.src \
))
# vim: set noet sw=4 ts=4:
