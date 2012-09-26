# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,lwpft))

$(eval $(call gb_Library_set_include,lwpft,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_use_sdk_api,lwpft))

$(eval $(call gb_Library_use_libraries,lwpft,\
    comphelper \
    cppu \
    cppuhelper \
    sal \
    sfx \
    sot \
    svt \
    svxcore \
    tl \
    ucbhelper \
    vcl \
    xo \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_standard_system_libs,lwpft))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_Library_use_externals,lwpft,\
    icuin \
    icuuc \
))
else
$(eval $(call gb_Library_use_externals,lwpft,\
    icui18n \
    icuuc \
))
endif

$(eval $(call gb_Library_set_componentfile,lwpft,lotuswordpro/util/lwpfilter))

$(eval $(call gb_Library_add_exception_objects,lwpft,\
    lotuswordpro/source/filter/bencont \
    lotuswordpro/source/filter/benlist \
    lotuswordpro/source/filter/benname \
    lotuswordpro/source/filter/benobj \
    lotuswordpro/source/filter/benval \
    lotuswordpro/source/filter/explode \
    lotuswordpro/source/filter/genericfilter \
    lotuswordpro/source/filter/localtime \
    lotuswordpro/source/filter/LotusWordProImportFilter \
    lotuswordpro/source/filter/lwp9reader \
    lotuswordpro/source/filter/lwpatomholder \
    lotuswordpro/source/filter/lwpbackgroundoverride \
    lotuswordpro/source/filter/lwpbackgroundstuff \
    lotuswordpro/source/filter/lwpbasetype \
    lotuswordpro/source/filter/lwpbookmarkmgr \
    lotuswordpro/source/filter/lwpborderstuff \
    lotuswordpro/source/filter/lwpbreaksoverride \
    lotuswordpro/source/filter/lwpbulletstylemgr \
    lotuswordpro/source/filter/lwpcelllayout \
    lotuswordpro/source/filter/lwpchangemgr \
    lotuswordpro/source/filter/lwpcharacterstyle \
    lotuswordpro/source/filter/lwpcharborderoverride \
    lotuswordpro/source/filter/lwpcharsetmgr \
    lotuswordpro/source/filter/lwpcolor \
    lotuswordpro/source/filter/lwpcontent \
    lotuswordpro/source/filter/lwpdivinfo \
    lotuswordpro/source/filter/lwpdivopts \
    lotuswordpro/source/filter/lwpdllist \
    lotuswordpro/source/filter/lwpdlvlist \
    lotuswordpro/source/filter/lwpdoc \
    lotuswordpro/source/filter/lwpdocdata \
    lotuswordpro/source/filter/lwpdrawobj \
    lotuswordpro/source/filter/lwpdropcapmgr \
    lotuswordpro/source/filter/lwpfilehdr \
    lotuswordpro/source/filter/lwpfilter \
    lotuswordpro/source/filter/lwpfnlayout \
    lotuswordpro/source/filter/lwpfont \
    lotuswordpro/source/filter/lwpfootnote \
    lotuswordpro/source/filter/lwpfoundry \
    lotuswordpro/source/filter/lwpframelayout \
    lotuswordpro/source/filter/lwpfribbreaks \
    lotuswordpro/source/filter/lwpfrib \
    lotuswordpro/source/filter/lwpfribframe \
    lotuswordpro/source/filter/lwpfribmark \
    lotuswordpro/source/filter/lwpfribptr \
    lotuswordpro/source/filter/lwpfribsection \
    lotuswordpro/source/filter/lwpfribtable \
    lotuswordpro/source/filter/lwpfribtext \
    lotuswordpro/source/filter/lwpglobalmgr \
    lotuswordpro/source/filter/lwpgrfobj \
    lotuswordpro/source/filter/lwpholder \
    lotuswordpro/source/filter/lwphyperlinkmgr \
    lotuswordpro/source/filter/lwpidxmgr \
    lotuswordpro/source/filter/lwplayout \
    lotuswordpro/source/filter/lwplaypiece \
    lotuswordpro/source/filter/lwplnopts \
    lotuswordpro/source/filter/lwpmarker \
    lotuswordpro/source/filter/lwpnotes \
    lotuswordpro/source/filter/lwpnumberingoverride \
    lotuswordpro/source/filter/lwpnumericfmt \
    lotuswordpro/source/filter/lwpobj \
    lotuswordpro/source/filter/lwpobjfactory \
    lotuswordpro/source/filter/lwpobjhdr \
    lotuswordpro/source/filter/lwpobjid \
    lotuswordpro/source/filter/lwpobjstrm \
    lotuswordpro/source/filter/lwpoleobject \
    lotuswordpro/source/filter/lwpoverride \
    lotuswordpro/source/filter/lwppagehint \
    lotuswordpro/source/filter/lwppagelayout \
    lotuswordpro/source/filter/lwppara1 \
    lotuswordpro/source/filter/lwpparaborderoverride \
    lotuswordpro/source/filter/lwppara \
    lotuswordpro/source/filter/lwpparaproperty \
    lotuswordpro/source/filter/lwpparastyle \
    lotuswordpro/source/filter/lwpproplist \
    lotuswordpro/source/filter/lwpprtinfo \
    lotuswordpro/source/filter/lwprowlayout \
    lotuswordpro/source/filter/lwpsdwfileloader \
    lotuswordpro/source/filter/lwpsdwgrouploaderv0102 \
    lotuswordpro/source/filter/lwpsdwrect \
    lotuswordpro/source/filter/lwpsection \
    lotuswordpro/source/filter/lwpsilverbullet \
    lotuswordpro/source/filter/lwpsortopt \
    lotuswordpro/source/filter/lwpstory \
    lotuswordpro/source/filter/lwpsvstream \
    lotuswordpro/source/filter/lwptable \
    lotuswordpro/source/filter/lwptablelayout \
    lotuswordpro/source/filter/lwptaboverride \
    lotuswordpro/source/filter/lwptabrack \
    lotuswordpro/source/filter/lwptblcell \
    lotuswordpro/source/filter/lwptblformula \
    lotuswordpro/source/filter/lwptoc \
    lotuswordpro/source/filter/lwptools \
    lotuswordpro/source/filter/lwpuidoc \
    lotuswordpro/source/filter/lwpusrdicts \
    lotuswordpro/source/filter/lwpverdocument \
    lotuswordpro/source/filter/lwpvpointer \
    lotuswordpro/source/filter/tocread \
    lotuswordpro/source/filter/utbenvs \
    lotuswordpro/source/filter/utlist \
    lotuswordpro/source/filter/xfilter/xfbase64 \
    lotuswordpro/source/filter/xfilter/xfbgimage \
    lotuswordpro/source/filter/xfilter/xfborders \
    lotuswordpro/source/filter/xfilter/xfcell \
    lotuswordpro/source/filter/xfilter/xfcellstyle \
    lotuswordpro/source/filter/xfilter/xfchange \
    lotuswordpro/source/filter/xfilter/xfcolor \
    lotuswordpro/source/filter/xfilter/xfcolstyle \
    lotuswordpro/source/filter/xfilter/xfcolumns \
    lotuswordpro/source/filter/xfilter/xfconfigmanager \
    lotuswordpro/source/filter/xfilter/xfcontentcontainer \
    lotuswordpro/source/filter/xfilter/xfcrossref \
    lotuswordpro/source/filter/xfilter/xfdate \
    lotuswordpro/source/filter/xfilter/xfdatestyle \
    lotuswordpro/source/filter/xfilter/xfdocfield \
    lotuswordpro/source/filter/xfilter/xfdrawareastyle \
    lotuswordpro/source/filter/xfilter/xfdrawline \
    lotuswordpro/source/filter/xfilter/xfdrawlinestyle \
    lotuswordpro/source/filter/xfilter/xfdrawobj \
    lotuswordpro/source/filter/xfilter/xfdrawpath \
    lotuswordpro/source/filter/xfilter/xfdrawpolygon \
    lotuswordpro/source/filter/xfilter/xfdrawpolyline \
    lotuswordpro/source/filter/xfilter/xfdrawrect \
    lotuswordpro/source/filter/xfilter/xfdrawstyle \
    lotuswordpro/source/filter/xfilter/xfentry \
    lotuswordpro/source/filter/xfilter/xffloatframe \
    lotuswordpro/source/filter/xfilter/xffont \
    lotuswordpro/source/filter/xfilter/xffontdecl \
    lotuswordpro/source/filter/xfilter/xffontfactory \
    lotuswordpro/source/filter/xfilter/xfframe \
    lotuswordpro/source/filter/xfilter/xfframestyle \
    lotuswordpro/source/filter/xfilter/xfglobal \
    lotuswordpro/source/filter/xfilter/xfheaderstyle \
    lotuswordpro/source/filter/xfilter/xfimage \
    lotuswordpro/source/filter/xfilter/xfimagestyle \
    lotuswordpro/source/filter/xfilter/xfindex \
    lotuswordpro/source/filter/xfilter/xflist \
    lotuswordpro/source/filter/xfilter/xfliststyle \
    lotuswordpro/source/filter/xfilter/xfmargins \
    lotuswordpro/source/filter/xfilter/xfmasterpage \
    lotuswordpro/source/filter/xfilter/xfnumberstyle \
    lotuswordpro/source/filter/xfilter/xfofficemeta \
    lotuswordpro/source/filter/xfilter/xfpagemaster \
    lotuswordpro/source/filter/xfilter/xfparagraph \
    lotuswordpro/source/filter/xfilter/xfparastyle \
    lotuswordpro/source/filter/xfilter/xfrow \
    lotuswordpro/source/filter/xfilter/xfrowstyle \
    lotuswordpro/source/filter/xfilter/xfsaxattrlist \
    lotuswordpro/source/filter/xfilter/xfsaxstream \
    lotuswordpro/source/filter/xfilter/xfsection \
    lotuswordpro/source/filter/xfilter/xfsectionstyle \
    lotuswordpro/source/filter/xfilter/xfshadow \
    lotuswordpro/source/filter/xfilter/xfstylecont \
    lotuswordpro/source/filter/xfilter/xfstyle \
    lotuswordpro/source/filter/xfilter/xfstylemanager \
    lotuswordpro/source/filter/xfilter/xftable \
    lotuswordpro/source/filter/xfilter/xftablestyle \
    lotuswordpro/source/filter/xfilter/xftabstyle \
    lotuswordpro/source/filter/xfilter/xftextboxstyle \
    lotuswordpro/source/filter/xfilter/xftextcontent \
    lotuswordpro/source/filter/xfilter/xftextspan \
    lotuswordpro/source/filter/xfilter/xftextstyle \
    lotuswordpro/source/filter/xfilter/xftimestyle \
    lotuswordpro/source/filter/xfilter/xfutil \
))

# vim: set noet sw=4 ts=4:
