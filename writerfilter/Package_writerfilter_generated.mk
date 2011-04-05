#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call \
	gb_Package_Package,writerfilter_generated,$(WORKDIR)/CustomTarget/writerfilter/source))

$(eval $(call \
	gb_Package_add_customtarget,writerfilter_generated,writerfilter/source))


$(eval $(call \
	gb_CustomTarget_add_outdir_dependencies,writerfilter/source,\
		$(gb_XSLTPROCTARGET) \
		$(OUTDIR)/inc$(UPDMINOREXT)/oox/token/tokens.txt \
		$(OUTDIR)/inc$(UPDMINOREXT)/oox/token/namespaces.txt \
))

$(eval $(call \
	gb_CustomTarget_add_dependencies,writerfilter/source,\
		writerfilter/source/doctok/resources.xmi \
		writerfilter/source/doctok/resources.xsl \
		writerfilter/source/doctok/resourcesimpl.xsl \
		writerfilter/source/doctok/resourceids.xsl \
		writerfilter/source/doctok/sprmids.xsl \
		writerfilter/source/doctok/resourcetools.xsl \
		writerfilter/source/ooxml/model.xml \
		writerfilter/source/ooxml/modelpreprocess.xsl \
		writerfilter/source/ooxml/factorytools.xsl \
		writerfilter/source/ooxml/fasttokens.xsl \
		writerfilter/source/ooxml/namespaceids.xsl \
		writerfilter/source/ooxml/factory_values.xsl \
		writerfilter/source/ooxml/factoryimpl_values.xsl \
		writerfilter/source/ooxml/resourceids.xsl \
		writerfilter/source/ooxml/gperffasttokenhandler.xsl \
		writerfilter/source/ooxml/gperffasttokenhandler.xsl \
		writerfilter/source/ooxml/tokenxmlheader \
		writerfilter/source/ooxml/tokenxmlfooter \
		writerfilter/source/ooxml/factoryinc.xsl \
		writerfilter/source/ooxml/factoryimpl.xsl \
		writerfilter/source/ooxml/factoryimpl_ns.xsl \
		writerfilter/source/ooxml/factory_ns.xsl \
		writerfilter/source/rtftok/RTFScanner.lex \
		writerfilter/source/rtftok/RTFScanner.skl \
		writerfilter/source/doctok/sprmcodetostr.xsl \
		writerfilter/source/doctok/qnametostr.xsl \
		writerfilter/source/ooxml/qnametostr.xsl \
		writerfilter/source/resourcemodel/namespace_preprocess.pl \
		writerfilter/source/resourcemodel/qnametostrheader \
		writerfilter/source/resourcemodel/qnametostrfooter \
		writerfilter/source/resourcemodel/sprmcodetostrheader \
		writerfilter/source/resourcemodel/sprmcodetostrfooter \
))

$(eval $(call gb_Package_add_file,writerfilter_generated,inc/writerfilter/doctok/sprmids.hxx,doctok/sprmids.hxx))
$(eval $(call gb_Package_add_file,writerfilter_generated,inc/writerfilter/doctok/resourceids.hxx,doctok/resourceids.hxx))
$(eval $(call gb_Package_add_file,writerfilter_generated,inc/writerfilter/ooxml/resourceids.hxx,ooxml/resourceids.hxx))

