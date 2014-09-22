# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

$(eval $(call gb_Jar_Jar,commonwizards))

$(eval $(call gb_Jar_use_jars,commonwizards,\
	ridl \
	unoil \
	jurt \
	juh \
	java_uno \
))

$(eval $(call gb_Jar_set_packageroot,commonwizards,com))

$(eval $(call gb_Jar_set_manifest,commonwizards,$(SRCDIR)/wizards/com/sun/star/wizards/common/MANIFEST.MF))

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
	wizards/com/sun/star/wizards/common/ParaStyled       \
	wizards/com/sun/star/wizards/common/PlaceholderTextElement       \
	wizards/com/sun/star/wizards/common/TemplateElement       \
	wizards/com/sun/star/wizards/common/TextElement       \
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
