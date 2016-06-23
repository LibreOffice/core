# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,odfvalidator))

$(eval $(call gb_ExternalProject_register_targets,odfvalidator,\
	build \
))

ODFVALIDATOR_SCRIPT:="$(WORKDIR)/odfvalidator.sh"
ODFVALIDATOR_PATH:="$(gb_Package_SOURCEDIR_odfvalidator)/odfvalidator-1.1.8.jar"

$(eval $(call gb_Output_info,Generate script $(gb_Package_SOURCEDIR_odfvalidator)))

$(call gb_ExternalProject_get_state_target,odfvalidator,build) :
	$(call gb_ExternalProject_run,build,\
		echo "#!/usr/bin/env bash" > "$(ODFVALIDATOR_SCRIPT)" \
		&& echo "java -Djavax.xml.validation.SchemaFactory:http://relaxng.org/ns/structure/1.0=org.iso_relax.verifier.jaxp.validation.RELAXNGSchemaFactoryImpl -Dorg.iso_relax.verifier.VerifierFactoryLoader=com.sun.msv.verifier.jarv.FactoryLoaderImpl -jar $(ODFVALIDATOR_PATH) -e \"\$$@\"" >> "$(ODFVALIDATOR_SCRIPT)" \
		&& chmod +x "$(ODFVALIDATOR_SCRIPT)" \
	)

# vim: set noet sw=4 ts=4:
