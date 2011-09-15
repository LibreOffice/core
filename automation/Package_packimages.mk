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
# Copyright (C) 2011 David Tardon, Red Hat Inc. <dtardon@redhat.com> (initial developer)
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

# TODO: this is probably the same procedure that is used in
# postprocess/packimages -> good candidate for a gbuild class

$(eval $(call gb_Package_Package,automation_packimages,$(call gb_CustomTarget_get_workdir,automation/packimages)))

$(eval $(call gb_Package_add_file,automation_packimages,bin/images_tt.zip,images_tt.zip))

$(eval $(call gb_Package_add_customtarget,automation_packimages,automation/packimages,SRCDIR))

$(eval $(call gb_CustomTarget_add_outdir_dependencies,automation/packimages,\
    $(OUTDIR)/res/img/fps_officeen-US.ilst \
    $(OUTDIR)/res/img/sben-US.ilst \
    $(OUTDIR)/res/img/stten-US.ilst \
    $(OUTDIR)/res/img/svten-US.ilst \
    $(OUTDIR)/res/img/vclen-US.ilst \
))

# vim: set ts=4 sw=4 noet:
