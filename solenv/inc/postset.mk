#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2008 by Sun Microsystems, Inc.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# $RCSfile: postset.mk,v $
#
# $Revision: 1.47.50.1 $
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


# Default ISO code used when no languages specified
# This language will be always included!
defaultlangiso=en-US

# Complete list of all supported ISO codes
completelangiso=af ar as-IN be-BY bg br brx bn bn-BD bn-IN bs by ca cs cy da de dgo dz el en-GB en-US en-ZA eo es et eu fa fi fr ga gd gl gu gu-IN he hi-IN hr hu it ja ka kk km kn ko kok ks ku lo lt lv mai mk mn mni ms ml-IN mr-IN my ne nb nl nn nr ns oc or-IN pa-IN pl pt pt-BR ru rw sat sa-IN sc sd sk sl sh sr ss st sv sw sw-TZ te-IN ti-ER ta-IN th tn tr ts tg ur-IN uk uz ve vi xh zh-CN zh-TW zu

alllangiso=$(strip $(defaultlangiso) $(subst,$(defaultlangiso), $(uniq $(subst,ALL,$(completelangiso) $(WITH_LANG)))))
