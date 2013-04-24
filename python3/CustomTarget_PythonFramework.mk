# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,python3/fixscripts))
$(eval $(call gb_CustomTarget_CustomTarget,python3/fixinstallnames))
$(eval $(call gb_CustomTarget_CustomTarget,python3/executables))

python3_fw_prefix=$(call gb_UnpackedTarball_get_dir,python3)/python-inst/@__________________________________________________OOO/LibreOfficePython.framework

# rule to allow relocating the whole framework, removing reference to buildinstallation directory
$(call gb_CustomTarget_get_target,python3/fixscripts): $(call gb_ExternalProject_get_target,python3)
	$(call gb_Output_announce,python3 - remove reference to installroot from scripts,build,CUS,5)
	$(COMMAND_ECHO)for file in \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/2to3 \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/2to3-$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/idle$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/pydoc$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)-config \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m-config \
			$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin/pyvenv-$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) ; do \
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
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/LibreOfficePython \
		@executable_path/../../../../LibreOfficePython \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/Resources/Python.app/Contents/MacOS/LibreOfficePython
	touch $@

# also delete binaries that are symlinked in scp2
$(call gb_CustomTarget_get_target,python3/executables): $(call gb_ExternalProject_get_target,python3)
	$(call gb_Output_announce,python3 - fix installnames in executables,build,CUS,5)
	cd $(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/bin ; \
	for file in python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) \
	            python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)m \
	            pythonw$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR) ; do \
	install_name_tool -change \
		$(python3_fw_prefix)/Versions/$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)/LibreOfficePython \
		@executable_path/../LibreOfficePython $$file ; done
	touch $@

# vim: set noet sw=4 ts=4:
