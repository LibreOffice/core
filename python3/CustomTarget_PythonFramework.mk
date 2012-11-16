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
# Copyright (C) 2012 Christian Lohmaier <lohmaier+LibreOffice@gmail.com>
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

$(eval $(call gb_CustomTarget_CustomTarget,python3/fixscripts))
$(eval $(call gb_CustomTarget_CustomTarget,python3/fixinstallnames))
$(eval $(call gb_CustomTarget_CustomTarget,python3/excludelist))

python3_fw_prefix=$(call gb_UnpackedTarball_get_dir,python3)/installroot/Library/Frameworks/LibreOfficePython.framework

# rule to allow relocating the whole framework, removing reference to buildinstallation directory
$(call gb_CustomTarget_get_target,python3/fixscripts): $(call gb_ExternalProject_get_target,python3)
	$(call gb_Output_announce,python3 - remove reference to installroot from scripts,build,CUS,5)
	$(COMMAND_ECHO)for file in \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/2to3 \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/2to3-$(python3_PYMAJOR).$(python3_PYMINOR) \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/idle$(python3_PYMAJOR).$(python3_PYMINOR) \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/pydoc$(python3_PYMAJOR).$(python3_PYMINOR) \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/python$(python3_PYMAJOR).$(python3_PYMINOR)-config \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/python$(python3_PYMAJOR).$(python3_PYMINOR)m-config \
			$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/bin/pyvenv-$(python3_PYMAJOR).$(python3_PYMINOR) ; do \
	{ rm "$$file" && awk '\
		BEGIN {print "#!/bin/bash\n\
origpath=$$(pwd)\n\
bindir=$$(cd $$(dirname \"$$0\") ; pwd)\n\
cd \"$$origpath\"\n\
\"$$bindir/../Resources/Python.app/Contents/MacOS/LibreOfficePython\" - $$@ <<EOF"} \
		FNR==1{next} \
		      {print} \
		END   {print "EOF"}' > "$$file" ; } < "$$file" ; chmod +x "$$file" ; done
	touch $@

$(call gb_CustomTarget_get_target,python3/fixinstallnames): $(call gb_ExternalProject_get_target,python3)
	$(call gb_Output_announce,python3 - fix installname,build,CUS,5)
	install_name_tool -change \
		$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/LibreOfficePython \
		@executable_path/../../../../LibreOfficePython \
		$(python3_fw_prefix)/Versions/$(python3_PYMAJOR).$(python3_PYMINOR)/Resources/Python.app/Contents/MacOS/LibreOfficePython
	touch $@

# delete symlinks that cannot be squeezed through scp2-process (scp2 rules set them up again)
# also delete binaries that are symlinked in scp2
$(call gb_CustomTarget_get_target,python3/excludelist): $(call gb_ExternalProject_get_target,python3)
	$(call gb_Output_announce,python3 - prepare list of files to exclude from zip,build,CUS,5)
	cd $(call gb_UnpackedTarball_get_dir,python3)/installroot/Library/Frameworks/ ; \
	find LibreOfficePython.framework -type l \
		-o -name python$(python3_PYMAJOR).$(python3_PYMINOR) \
		-o -name python$(python3_PYMAJOR).$(python3_PYMINOR)m \
		-o -name pythonw$(python3_PYMAJOR).$(python3_PYMINOR) > $@

# vim: set noet sw=4 ts=4:
