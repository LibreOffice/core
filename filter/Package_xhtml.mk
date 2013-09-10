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

$(eval $(call gb_Package_Package,filter_xhtml,$(SRCDIR)/filter/source/xslt))

$(eval $(call gb_Package_set_outdir,filter_xhtml,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_file,filter_xhtml,$(LIBO_SHARE_FOLDER)/xslt/export/xhtml/body.xsl,odf2xhtml/export/xhtml/body.xsl))
$(eval $(call gb_Package_add_file,filter_xhtml,$(LIBO_SHARE_FOLDER)/xslt/export/xhtml/header.xsl,odf2xhtml/export/xhtml/header.xsl))
$(eval $(call gb_Package_add_file,filter_xhtml,$(LIBO_SHARE_FOLDER)/xslt/export/xhtml/opendoc2xhtml.xsl,odf2xhtml/export/xhtml/opendoc2xhtml.xsl))
$(eval $(call gb_Package_add_file,filter_xhtml,$(LIBO_SHARE_FOLDER)/xslt/export/xhtml/table.xsl,odf2xhtml/export/xhtml/table.xsl))

# vim: set noet sw=4 ts=4:
