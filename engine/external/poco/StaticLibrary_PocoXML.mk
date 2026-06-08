# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,PocoXML))

$(eval $(call gb_StaticLibrary_use_unpacked,PocoXML,poco))

# keep the default std::vector ABI - the non-dbgutil online server links this
$(eval $(call gb_StaticLibrary_add_defs,PocoXML,\
	-U_GLIBCXX_DEBUG \
))

$(eval $(call gb_StaticLibrary_set_warnings_disabled,PocoXML))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,PocoXML,cpp))

$(eval $(call gb_StaticLibrary_set_include,PocoXML,\
	-I$(gb_UnpackedTarball_workdir)/poco/include \
	$$(INCLUDE) \
))

# POCO_UNBUNDLED makes Poco/XML use the engine's expat (Poco/XML/ParserEngine.h
# then includes <expat.h>) rather than POCO's bundled copy.
$(eval $(call gb_StaticLibrary_add_defs,PocoXML,\
	-DPOCO_STATIC \
	-DPOCO_NO_AUTOMATIC_LIBS \
	-UIOS \
	-DPOCO_UNBUNDLED \
))

$(eval $(call gb_StaticLibrary_use_externals,PocoXML,\
	expat \
))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,PocoXML,\
	UnpackedTarball/poco/XML/src/AbstractContainerNode \
	UnpackedTarball/poco/XML/src/AbstractNode \
	UnpackedTarball/poco/XML/src/Attr \
	UnpackedTarball/poco/XML/src/AttrMap \
	UnpackedTarball/poco/XML/src/Attributes \
	UnpackedTarball/poco/XML/src/AttributesImpl \
	UnpackedTarball/poco/XML/src/CDATASection \
	UnpackedTarball/poco/XML/src/CharacterData \
	UnpackedTarball/poco/XML/src/ChildNodesList \
	UnpackedTarball/poco/XML/src/Comment \
	UnpackedTarball/poco/XML/src/ContentHandler \
	UnpackedTarball/poco/XML/src/DeclHandler \
	UnpackedTarball/poco/XML/src/DefaultHandler \
	UnpackedTarball/poco/XML/src/Document \
	UnpackedTarball/poco/XML/src/DocumentEvent \
	UnpackedTarball/poco/XML/src/DocumentFragment \
	UnpackedTarball/poco/XML/src/DocumentType \
	UnpackedTarball/poco/XML/src/DOMBuilder \
	UnpackedTarball/poco/XML/src/DOMException \
	UnpackedTarball/poco/XML/src/DOMImplementation \
	UnpackedTarball/poco/XML/src/DOMObject \
	UnpackedTarball/poco/XML/src/DOMParser \
	UnpackedTarball/poco/XML/src/DOMSerializer \
	UnpackedTarball/poco/XML/src/DOMWriter \
	UnpackedTarball/poco/XML/src/DTDHandler \
	UnpackedTarball/poco/XML/src/DTDMap \
	UnpackedTarball/poco/XML/src/Element \
	UnpackedTarball/poco/XML/src/ElementsByTagNameList \
	UnpackedTarball/poco/XML/src/Entity \
	UnpackedTarball/poco/XML/src/EntityReference \
	UnpackedTarball/poco/XML/src/EntityResolver \
	UnpackedTarball/poco/XML/src/EntityResolverImpl \
	UnpackedTarball/poco/XML/src/ErrorHandler \
	UnpackedTarball/poco/XML/src/Event \
	UnpackedTarball/poco/XML/src/EventDispatcher \
	UnpackedTarball/poco/XML/src/EventException \
	UnpackedTarball/poco/XML/src/EventListener \
	UnpackedTarball/poco/XML/src/EventTarget \
	UnpackedTarball/poco/XML/src/InputSource \
	UnpackedTarball/poco/XML/src/LexicalHandler \
	UnpackedTarball/poco/XML/src/Locator \
	UnpackedTarball/poco/XML/src/LocatorImpl \
	UnpackedTarball/poco/XML/src/MutationEvent \
	UnpackedTarball/poco/XML/src/Name \
	UnpackedTarball/poco/XML/src/NamedNodeMap \
	UnpackedTarball/poco/XML/src/NamePool \
	UnpackedTarball/poco/XML/src/NamespaceStrategy \
	UnpackedTarball/poco/XML/src/NamespaceSupport \
	UnpackedTarball/poco/XML/src/NodeAppender \
	UnpackedTarball/poco/XML/src/Node \
	UnpackedTarball/poco/XML/src/NodeFilter \
	UnpackedTarball/poco/XML/src/NodeIterator \
	UnpackedTarball/poco/XML/src/NodeList \
	UnpackedTarball/poco/XML/src/Notation \
	UnpackedTarball/poco/XML/src/ParserEngine \
	UnpackedTarball/poco/XML/src/ProcessingInstruction \
	UnpackedTarball/poco/XML/src/QName \
	UnpackedTarball/poco/XML/src/SAXException \
	UnpackedTarball/poco/XML/src/SAXParser \
	UnpackedTarball/poco/XML/src/Text \
	UnpackedTarball/poco/XML/src/TreeWalker \
	UnpackedTarball/poco/XML/src/ValueTraits \
	UnpackedTarball/poco/XML/src/WhitespaceFilter \
	UnpackedTarball/poco/XML/src/XMLException \
	UnpackedTarball/poco/XML/src/XMLFilter \
	UnpackedTarball/poco/XML/src/XMLFilterImpl \
	UnpackedTarball/poco/XML/src/XMLReader \
	UnpackedTarball/poco/XML/src/XMLStreamParser \
	UnpackedTarball/poco/XML/src/XMLStreamParserException \
	UnpackedTarball/poco/XML/src/XMLString \
	UnpackedTarball/poco/XML/src/XMLWriter \
))

# vim: set noet sw=4 ts=4:
