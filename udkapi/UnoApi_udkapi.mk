# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2000, 2010 Oracle and/or its affiliates.
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

$(eval $(call gb_UnoApi_UnoApi,udkapi))

$(eval $(call gb_UnoApi_set_include,udkapi,\
	$$(INCLUDE) \
	-I$(SRCDIR)/udkapi \
	-I$(OUTDIR)/idl \
))


$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star,\
	udk-modules \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/beans,\
	PropertyBag \
	PropertySet \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/bridge,\
	Bridge \
	BridgeFactory \
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
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/corba,\
	corba \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/corba/giop,\
	giop \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/corba/iiop,\
	iiop \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/corba/iop,\
	iop \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/io,\
	DataInputStream \
	DataOutputStream \
	MarkableInputStream \
	MarkableOutputStream \
	ObjectInputStream \
	ObjectOutputStream \
	Pump \
	TextInputStream \
	TextOutputStream \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/java,\
	JavaVirtualMachine \
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
	ProxyFactory \
	TypeDescriptionManager \
	TypeDescriptionProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/registry,\
	DefaultRegistry \
	NestedRegistry \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/script,\
	Engine \
	InvocationAdapterFactory \
	JavaScript \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/test,\
	TestFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/util,\
	BootstrapMacroExpander \
	MacroExpander \
	theMacroExpander \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,udkapi,udkapi/com/sun/star/util/logging,\
	Logger \
	LoggerRemote \
))


$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/beans,\
	Introspection \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,udkapi,udkapi/com/sun/star/bridge,\
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
	XIntroTest \
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
    XPipe \
	XPersist \
	XPersistObject \
	XSeekable \
	XSeekableInputStream \
	XSequenceOutputStream \
	XStream \
	XStreamListener \
	XTempFile \
	XTextInputStream \
	XTextOutputStream \
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
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/test,\
	TestEvent \
	XSimpleTest \
	XTest \
	XTestListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/test/bridge,\
	XBridgeTest \
))
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/test/performance,\
	XPerformanceTest \
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
$(eval $(call gb_UnoApi_add_idlfiles,udkapi,udkapi/com/sun/star/util/logging,\
	LogLevel \
	XLogger \
	XLoggerRemote \
))


# vim: set noet sw=4 ts=4:
