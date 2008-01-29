#*************************************************************************
#
#   OpenOffice.org - a multi-platform office productivity suite
#
#   $RCSfile: postset.mk,v $
#
#   $Revision: 1.43 $
#
#   last change: $Author: rt $ $Date: 2008-01-29 08:54:27 $
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


# Default ISO code used when no languages specified
# This language will be always included!
defaultlangiso=en-US

# Complete list of all supported ISO codes
completelangiso=af ar as-IN be-BY bg br bn bn-BD bn-IN bs ca cs cy da de dz el en-GB en-US en-ZA eo es et eu fa fi fr ga gl gu-IN he hi-IN hr hu it ja ka km kn ko ku lo lt lv mk ms ml-IN mr-IN ne nb nl nn nr ns oc or-IN pa-IN pl pt pt-BR ru rw sk sl sh sr ss st sv sw sw-TZ te-IN ti-ER ta-IN th tn tr ts tg ur-IN uk uz ve vi xh zh-CN zh-TW zu

alllangiso=$(strip $(defaultlangiso) $(subst,$(defaultlangiso), $(uniq $(subst,ALL,$(completelangiso) $(WITH_LANG)))))
