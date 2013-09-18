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

$(eval $(call gb_UnoApi_UnoApi,udkapi))

$(eval $(call gb_UnoApi_install,udkapi,$(LIBO_URE_SHARE_FOLDER)/misc/types.rdb))

$(eval $(call gb_UnoApi_package_idlfiles,udkapi))

$(eval $(call gb_UnoApi_set_include,udkapi,\
	$$(INCLUDE) \
	-I$(SRCDIR)/udkapi \
))


$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/beans,\
	PropertySet \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/bridge,\
	Bridge \
	IiopBridge \
	OleApplicationRegistration \
	OleBridgeSupplier \
	OleBridgeSupplier2 \
	OleBridgeSupplierVar1 \
	OleObjectFactory \
	UrpBridge \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/bridge/oleautomation,\
	ApplicationRegistration \
	BridgeSupplier \
	Factory \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/io,\
	DataInputStream \
	DataOutputStream \
	MarkableInputStream \
	MarkableOutputStream \
	ObjectInputStream \
	ObjectOutputStream \
	Pump \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/lang,\
	MultiServiceFactory \
	RegistryServiceManager \
	ServiceManager \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/loader,\
	Dynamic \
	Java \
	Java2 \
	SharedLibrary \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/reflection,\
	CoreReflection \
	TypeDescriptionManager \
	TypeDescriptionProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/registry,\
	DefaultRegistry \
	NestedRegistry \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/script,\
	Engine \
	JavaScript \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/util,\
	BootstrapMacroExpander \
))

$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/beans,\
	PropertyBag \
	Introspection \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/bridge,\
	BridgeFactory \
	UnoUrlResolver \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/connection,\
	Acceptor \
	Connector \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/container,\
	EnumerableMap \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/io,\
	Pipe \
	SequenceInputStream \
	SequenceOutputStream \
	TempFile \
	TextInputStream \
	TextOutputStream \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/java,\
	JavaVirtualMachine \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/reflection,\
	theCoreReflection \
	ProxyFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/registry,\
	ImplementationRegistration \
	SimpleRegistry \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/security,\
	AccessController \
	Policy \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/script,\
	AllListenerAdapter \
	Converter \
	Invocation \
	InvocationAdapterFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/uno,\
	NamingService \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/uri,\
	ExternalUriReferenceTranslator \
	UriReferenceFactory \
	UriSchemeParser_vndDOTsunDOTstarDOTexpand \
	UriSchemeParser_vndDOTsunDOTstarDOTscript \
	VndSunStarPkgUrlReferenceFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/util,\
	MacroExpander \
	theMacroExpander \
))


$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/beans,\
	Ambiguous \
	Defaulted \
	GetDirectPropertyTolerantResult \
	GetPropertyTolerantResult \
	IllegalTypeException \
	IntrospectionException \
	MethodConcept \
	NamedValue \
	NotRemoveableException \
	Optional \
	Pair \
	Property \
	PropertyAttribute \
	PropertyChangeEvent \
	PropertyConcept \
	PropertyExistException \
	PropertySetInfoChange \
	PropertySetInfoChangeEvent \
	PropertyState \
	PropertyStateChangeEvent \
	PropertyValue \
	PropertyValues \
	PropertyVetoException \
	SetPropertyTolerantFailed \
	StringPair \
	TolerantPropertySetResultType \
	UnknownPropertyException \
	XExactName \
	XFastPropertySet \
	XHierarchicalPropertySet \
	XHierarchicalPropertySetInfo \
	XIntrospection \
	XIntrospectionAccess \
	XMaterialHolder \
	XMultiHierarchicalPropertySet \
	XMultiPropertySet \
	XMultiPropertyStates \
	XPropertiesChangeListener \
	XPropertiesChangeNotifier \
	XProperty \
	XPropertyAccess \
	XPropertyBag \
	XPropertyChangeListener \
	XPropertyContainer \
	XPropertySet \
	XPropertySetInfo \
	XPropertySetInfoChangeListener \
	XPropertySetInfoChangeNotifier \
	XPropertySetOption \
	XPropertyState \
	XPropertyStateChangeListener \
	XPropertyWithState \
	XTolerantMultiPropertySet \
	XVetoableChangeListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/bridge,\
	BridgeExistsException \
	InvalidProtocolChangeException \
	ModelDependent \
	ProtocolProperty \
	XBridge \
	XBridgeFactory \
	XBridgeFactory2 \
	XBridgeSupplier \
	XBridgeSupplier2 \
	XInstanceProvider \
	XProtocolProperties \
	XUnoUrlResolver \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/bridge/oleautomation,\
	Currency \
	Date \
	Decimal \
	NamedArgument \
	PropertyPutArgument \
	SCode \
	XAutomationObject \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/connection,\
	AlreadyAcceptingException \
	ConnectionSetupException \
	NoConnectException \
	SocketPermission \
	XAcceptor \
	XConnection \
	XConnection2 \
	XConnectionBroadcaster \
	XConnector \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/container,\
	ContainerEvent \
	ElementExistException \
	NoSuchElementException \
	XChild \
	XComponentEnumeration \
	XComponentEnumerationAccess \
	XContainer \
	XContainerApproveBroadcaster \
	XContainerApproveListener \
	XContainerListener \
	XContainerQuery \
	XContentEnumerationAccess \
	XElementAccess \
	XEnumerableMap \
	XEnumeration \
	XEnumerationAccess \
	XHierarchicalName \
	XHierarchicalNameAccess \
	XHierarchicalNameContainer \
	XHierarchicalNameReplace \
	XIdentifierAccess \
	XIdentifierContainer \
	XIdentifierReplace \
	XImplicitIDAccess \
	XImplicitIDContainer \
	XImplicitIDReplace \
	XIndexAccess \
	XIndexContainer \
	XIndexReplace \
	XMap \
	XNameAccess \
	XNameContainer \
	XNameReplace \
	XNamed \
	XSet \
	XStringKeyMap \
	XUniqueIDAccess \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/io,\
	AlreadyConnectedException \
	BufferSizeExceededException \
	ConnectException \
	DataTransferEvent \
	FilePermission \
	IOException \
	NoRouteToHostException \
	NotConnectedException \
	SocketException \
	UnexpectedEOFException \
	UnknownHostException \
	WrongFormatException \
	XActiveDataControl \
	XActiveDataSink \
	XActiveDataSource \
	XActiveDataStreamer \
	XAsyncOutputMonitor \
	XConnectable \
	XDataExporter \
	XDataImporter \
	XDataInputStream \
	XDataOutputStream \
	XDataTransferEventListener \
	XInputStream \
	XInputStreamProvider \
	XMarkableStream \
	XObjectInputStream \
	XObjectOutputStream \
	XOutputStream \
	XPersist \
	XPersistObject \
    XPipe \
	XSeekable \
	XSeekableInputStream \
	XSequenceOutputStream \
	XStream \
	XStreamListener \
	XTempFile \
	XTextInputStream \
	XTextInputStream2 \
	XTextOutputStream \
	XTextOutputStream2 \
	XTruncate \
	XXMLExtractor \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/java,\
	InvalidJavaSettingsException \
	JavaDisabledException \
	JavaInitializationException \
	JavaNotConfiguredException \
	JavaNotFoundException \
	JavaVMCreationFailureException \
	MissingJavaRuntimeException \
	RestartRequiredException \
	WrongJavaVersionException \
	XJavaThreadRegister_11 \
	XJavaVM \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/lang,\
	ArrayIndexOutOfBoundsException \
	ClassNotFoundException \
	DisposedException \
	EventObject \
	IllegalAccessException \
	IllegalArgumentException \
	IndexOutOfBoundsException \
	InvalidListenerException \
	ListenerExistException \
	Locale \
	NoSuchFieldException \
	NoSuchMethodException \
	NoSupportException \
	NotInitializedException \
	NullPointerException \
	ServiceNotRegisteredException \
	SystemDependent \
	WrappedTargetException \
	WrappedTargetRuntimeException \
	XComponent \
	XConnectionPoint \
	XConnectionPointContainer \
	XEventListener \
	XInitialization \
	XLocalizable \
	XMain \
	XMultiComponentFactory \
	XMultiServiceFactory \
	XServiceDisplayName \
	XServiceInfo \
	XServiceName \
	XSingleComponentFactory \
	XSingleServiceFactory \
	XTypeProvider \
	XUnoTunnel \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/loader,\
	CannotActivateFactoryException \
	XImplementationLoader \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/reflection,\
	FieldAccessMode \
	InvalidTypeNameException \
	InvocationTargetException \
	MethodMode \
	NoSuchTypeNameException \
	ParamInfo \
	ParamMode \
	TypeDescriptionSearchDepth \
	XArrayTypeDescription \
	XCompoundTypeDescription \
	XConstantTypeDescription \
	XConstantsTypeDescription \
	XEnumTypeDescription \
	XIdlArray \
	XIdlClass \
	XIdlClassProvider \
	XIdlField \
	XIdlField2 \
	XIdlMember \
	XIdlMethod \
	XIdlReflection \
	XIndirectTypeDescription \
	XInterfaceAttributeTypeDescription \
	XInterfaceAttributeTypeDescription2 \
	XInterfaceMemberTypeDescription \
	XInterfaceMethodTypeDescription \
	XInterfaceTypeDescription \
	XInterfaceTypeDescription2 \
	XMethodParameter \
	XModuleTypeDescription \
	XParameter \
	XPropertyTypeDescription \
	XProxyFactory \
	XPublished \
	XServiceConstructorDescription \
	XServiceTypeDescription \
	XServiceTypeDescription2 \
	XSingletonTypeDescription \
	XSingletonTypeDescription2 \
	XStructTypeDescription \
	XTypeDescription \
	XTypeDescriptionEnumeration \
	XTypeDescriptionEnumerationAccess \
	XUnionTypeDescription \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/registry,\
	CannotRegisterImplementationException \
	InvalidRegistryException \
	InvalidValueException \
	MergeConflictException \
	RegistryKeyType \
	RegistryValueType \
	XImplementationRegistration \
	XImplementationRegistration2 \
	XRegistryKey \
	XSimpleRegistry \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/script,\
	AllEventObject \
	ArrayWrapper \
	BasicErrorException \
	CannotConvertException \
	CannotCreateAdapterException \
	ContextInformation \
	EventListener \
	FailReason \
	FinishEngineEvent \
	FinishReason \
	InterruptEngineEvent \
	InterruptReason \
	InvocationInfo \
	MemberType \
	ScriptEvent \
	ScriptEventDescriptor \
	XAllListener \
	XAllListenerAdapterService \
    XAutomationInvocation \
	XDebugging \
	XDefaultMethod \
	XDefaultProperty \
	XDirectInvocation \
	XEngine \
	XEngineListener \
	XErrorQuery \
	XEventAttacher \
	XEventAttacher2 \
	XEventAttacherManager \
	XInvocation \
	XInvocation2 \
	XInvocationAdapterFactory \
	XInvocationAdapterFactory2 \
	XLibraryAccess \
	XScriptEventsAttacher \
	XScriptEventsSupplier \
	XScriptListener \
	XStarBasicAccess \
	XStarBasicDialogInfo \
	XStarBasicLibraryInfo \
	XStarBasicModuleInfo \
	XTypeConverter \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/security,\
	AccessControlException \
	AllPermission \
	RuntimePermission \
	XAccessControlContext \
	XAccessController \
	XAction \
	XPolicy \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/task,\
	XInteractionAbort \
	XInteractionContinuation \
	XInteractionHandler \
	XInteractionHandler2 \
	XInteractionRequest \
	XInteractionRetry \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/uno,\
	DeploymentException \
	Exception \
	RuntimeException \
	SecurityException \
	TypeClass \
	Uik \
	XAdapter \
	XAggregation \
	XComponentContext \
	XCurrentContext \
	XInterface \
	XNamingService \
	XReference \
	XUnloadingPreference \
	XWeak \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/uri,\
	RelativeUriExcessParentSegments \
	XExternalUriReferenceTranslator \
	XUriReference \
	XUriReferenceFactory \
	XUriSchemeParser \
	XVndSunStarExpandUrl \
	XVndSunStarExpandUrlReference \
	XVndSunStarPkgUrlReferenceFactory \
	XVndSunStarScriptUrl \
	XVndSunStarScriptUrlReference \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/util,\
	XMacroExpander \
	XVeto \
))

$(eval $(call gb_UnoApi_set_reference_rdbfile,udkapi,$(SRCDIR)/udkapi/type_reference/udkapi.idl))

# vim: set noet sw=4 ts=4:
