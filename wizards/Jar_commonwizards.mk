# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

$(eval $(call gb_Jar_Jar,commonwizards))

$(eval $(call gb_Jar_use_jars,commonwizards,\
	$(OUTDIR_FOR_BUILD)/bin/ridl.jar \
	$(OUTDIR_FOR_BUILD)/bin/unoil.jar \
	$(OUTDIR_FOR_BUILD)/bin/jurt.jar \
	$(OUTDIR_FOR_BUILD)/bin/juh.jar \
	$(OUTDIR_FOR_BUILD)/bin/java_uno.jar \
))

$(eval $(call gb_Jar_set_packageroot,commonwizards,com))

$(eval $(call gb_Jar_set_manifest,commonwizards,$(SRCDIR)/wizards/com/sun/star/wizards/common/MANIFEST.MF))

# FIXME: what is this doing here?
$(eval $(call gb_Jar_add_manifest_classpath,commonwizards,saxon9.jar))

$(eval $(call gb_Jar_add_sourcefiles,commonwizards,\
	wizards/com/sun/star/wizards/common/HelpIds	                		\
	wizards/com/sun/star/wizards/common/Desktop	                		\
	wizards/com/sun/star/wizards/common/NumberFormatter                 		\
	wizards/com/sun/star/wizards/common/FileAccess                 		\
	wizards/com/sun/star/wizards/common/Helper                     		\
	wizards/com/sun/star/wizards/common/InvalidQueryException      		\
	wizards/com/sun/star/wizards/common/JavaTools                  		\
	wizards/com/sun/star/wizards/common/NoValidPathException       		\
	wizards/com/sun/star/wizards/common/Properties                 		\
	wizards/com/sun/star/wizards/common/PropertyNames                 		\
	wizards/com/sun/star/wizards/common/Resource                   		\
	wizards/com/sun/star/wizards/common/SystemDialog				\
	wizards/com/sun/star/wizards/common/Configuration				\
	wizards/com/sun/star/wizards/common/ConfigNode					\
	wizards/com/sun/star/wizards/common/ConfigGroup				\
	wizards/com/sun/star/wizards/common/ConfigSet					\
	wizards/com/sun/star/wizards/common/XMLProvider				\
	wizards/com/sun/star/wizards/common/XMLHelper					\
	wizards/com/sun/star/wizards/common/IRenderer					\
	wizards/com/sun/star/wizards/common/Indexable					\
	wizards/com/sun/star/wizards/common/TerminateWizardException			\
	wizards/com/sun/star/wizards/common/UCB					\
	wizards/com/sun/star/wizards/common/DebugHelper					\
	wizards/com/sun/star/wizards/common/PropertySetHelper					\
	wizards/com/sun/star/wizards/common/NumericalHelper					\
	wizards/com/sun/star/wizards/common/NamedValueCollection       \
	wizards/com/sun/star/wizards/db/DBMetaData					\
	wizards/com/sun/star/wizards/db/CommandMetaData				\
	wizards/com/sun/star/wizards/db/QueryMetaData					\
	wizards/com/sun/star/wizards/db/BlindtextCreator				\
	wizards/com/sun/star/wizards/db/FieldColumn					\
	wizards/com/sun/star/wizards/db/CommandName					\
	wizards/com/sun/star/wizards/db/RecordParser					\
	wizards/com/sun/star/wizards/db/TypeInspector					\
	wizards/com/sun/star/wizards/db/ColumnPropertySet				\
	wizards/com/sun/star/wizards/db/RelationController				\
	wizards/com/sun/star/wizards/db/TableDescriptor				\
	wizards/com/sun/star/wizards/db/SQLQueryComposer				\
	wizards/com/sun/star/wizards/db/DatabaseObjectWizard					\
	wizards/com/sun/star/wizards/ui/event/AbstractListener			\
	wizards/com/sun/star/wizards/ui/event/CommonListener				\
	wizards/com/sun/star/wizards/ui/event/DataAware				\
	wizards/com/sun/star/wizards/ui/event/DataAwareFields				\
	wizards/com/sun/star/wizards/ui/event/EventNames				\
	wizards/com/sun/star/wizards/ui/event/ListModelBinder				\
	wizards/com/sun/star/wizards/ui/event/MethodInvocation			\
	wizards/com/sun/star/wizards/ui/event/RadioDataAware				\
	wizards/com/sun/star/wizards/ui/event/SimpleDataAware				\
	wizards/com/sun/star/wizards/ui/event/Task					\
	wizards/com/sun/star/wizards/ui/event/TaskEvent				\
	wizards/com/sun/star/wizards/ui/event/TaskListener				\
	wizards/com/sun/star/wizards/ui/event/UnoDataAware				\
	wizards/com/sun/star/wizards/ui/AggregateComponent				\
	wizards/com/sun/star/wizards/ui/CommandFieldSelection				\
	wizards/com/sun/star/wizards/ui/ControlScroller				\
	wizards/com/sun/star/wizards/ui/DBLimitedFieldSelection			\
	wizards/com/sun/star/wizards/ui/DocumentPreview				\
	wizards/com/sun/star/wizards/ui/FilterComponent				\
	wizards/com/sun/star/wizards/ui/FieldSelection					\
	wizards/com/sun/star/wizards/ui/ImageList					\
	wizards/com/sun/star/wizards/ui/ButtonList					\
	wizards/com/sun/star/wizards/ui/PathSelection					\
	wizards/com/sun/star/wizards/ui/PeerConfig					\
	wizards/com/sun/star/wizards/ui/SortingComponent				\
	wizards/com/sun/star/wizards/ui/TitlesComponent				\
	wizards/com/sun/star/wizards/ui/WizardDialog					\
	wizards/com/sun/star/wizards/ui/UIConsts					\
	wizards/com/sun/star/wizards/ui/UnoDialog					\
	wizards/com/sun/star/wizards/ui/UnoDialog2					\
	wizards/com/sun/star/wizards/ui/XCommandSelectionListener			\
	wizards/com/sun/star/wizards/ui/XFieldSelectionListener			\
	wizards/com/sun/star/wizards/ui/XPathSelectionListener				\
	wizards/com/sun/star/wizards/ui/XCompletion					\
	wizards/com/sun/star/wizards/document/Control					\
	wizards/com/sun/star/wizards/document/DatabaseControl				\
	wizards/com/sun/star/wizards/document/FormHandler				\
	wizards/com/sun/star/wizards/document/GridControl				\
	wizards/com/sun/star/wizards/document/OfficeDocument				\
	wizards/com/sun/star/wizards/document/Shape					\
	wizards/com/sun/star/wizards/document/TimeStampControl				\
	wizards/com/sun/star/wizards/text/TextDocument					\
	wizards/com/sun/star/wizards/text/TextFieldHandler				\
	wizards/com/sun/star/wizards/text/TextFrameHandler				\
	wizards/com/sun/star/wizards/text/TextSectionHandler				\
	wizards/com/sun/star/wizards/text/TextStyleHandler				\
	wizards/com/sun/star/wizards/text/TextTableHandler				\
	wizards/com/sun/star/wizards/text/ViewHandler \
))

# vim: set noet sw=4 ts=4:
