#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Jar_Jar,XSLTFilter))

$(eval $(call gb_Jar_use_jars,XSLTFilter,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
))

$(eval $(call gb_Jar_use_externals,XSLTFilter,saxon))

$(eval $(call gb_Jar_set_componentfile,XSLTFilter,filter/source/xsltfilter/XSLTFilter.jar,OOO))

$(eval $(call gb_Jar_set_manifest,XSLTFilter,$(SRCDIR)/filter/source/xsltfilter/Manifest))

$(eval $(call gb_Jar_set_packageroot,XSLTFilter,com))

$(eval $(call gb_Jar_add_sourcefiles,XSLTFilter,\
	filter/source/xsltfilter/com/sun/star/comp/xsltfilter/Base64 \
	filter/source/xsltfilter/com/sun/star/comp/xsltfilter/XSLTFilterOLEExtracter \
	filter/source/xsltfilter/com/sun/star/comp/xsltfilter/XSLTransformer \
))

# vim: set noet sw=4 ts=4:
