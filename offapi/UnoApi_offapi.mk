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

$(eval $(call gb_UnoApi_UnoApi,offapi))

$(eval $(call gb_UnoApi_package_idlfiles,offapi))

$(eval $(call gb_UnoApi_use_api,offapi,\
    udkapi \
))

$(eval $(call gb_UnoApi_set_include,offapi,\
	$$(INCLUDE) \
	-I$(SRCDIR)/udkapi \
	-I$(SRCDIR)/offapi \
))


$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/accessibility,\
	AccessBridge \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/animations,\
	AnimateColor \
	AnimateMotion \
	AnimateSet \
	Audio \
	Command \
	IterateContainer \
	ParallelTimeContainer \
	SequenceTimeContainer \
	TargetPropertiesCreator \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/auth,\
	SSOManagerFactory \
	SSOPasswordCache \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/awt,\
	AsyncCallback \
	ContainerWindowProvider \
	DialogProvider \
	DialogProvider2 \
	MenuBar \
	Pointer \
	PopupMenu \
	TabController \
	Toolkit \
	UnoControlDialog \
	UnoControlDialogModelProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/awt/grid,\
	DefaultGridColumnModel \
	DefaultGridDataModel \
	SortableGridDataModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/awt/tab,\
	UnoControlTabPageModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/awt/tree,\
	MutableTreeDataModel \
	MutableTreeNode \
	TreeControl \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/chart2,\
	CartesianCoordinateSystem2d \
	CartesianCoordinateSystem3d \
	ExponentialRegressionCurve \
	ExponentialScaling \
	FormattedString \
	LogarithmicRegressionCurve \
	LogarithmicScaling \
	LinearRegressionCurve \
	LinearScaling \
	MovingAverageRegressionCurve \
	PolarCoordinateSystem2d \
	PolarCoordinateSystem3d \
	PolynomialRegressionCurve \
	PotentialRegressionCurve \
	PowerScaling \
	RegressionEquation \
	Scaling \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/chart2/data,\
	DatabaseDataProvider \
    LabeledDataSequence \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/configuration,\
    ReadOnlyAccess \
    ReadWriteAccess \
	Update \
    theDefaultProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/cui,\
    ColorPicker \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/datatransfer,\
	DataFormatTranslator \
	MimeContentTypeFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/datatransfer/clipboard,\
	SystemClipboard \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/deployment,\
	ExtensionManager \
	PackageInformationProvider \
	PackageRegistryBackend \
	UpdateInformationProvider \
	thePackageManagerFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/deployment/test,\
	SmoketestCommandEnvironment \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/deployment/ui,\
	LicenseDialog \
	PackageManagerDialog \
	UpdateRequiredDialog \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/document,\
	DocumentProperties \
	DocumentRevisionListPersistence \
	IndexedPropertyValues \
	FilterConfigRefresh \
	GraphicObjectResolver \
	NamedPropertyValues \
	OleEmbeddedServerRegistration \
	OOXMLDocumentPropertiesImporter \
	XMLBasicExporter \
	XMLOasisBasicExporter \
	XMLOasisBasicImporter \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/drawing,\
	ColorTable \
	GraphicExportFilter \
	ModuleDispatcher \
	ShapeCollection \
	SlideRenderer \
	SlideSorter \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/drawing/framework,\
	BasicPaneFactory \
	BasicToolBarFactory \
	BasicViewFactory \
	Configuration \
	ConfigurationController \
	ModuleController \
	ResourceId \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/embed,\
	DocumentCloser \
	EmbeddedObjectCreator \
	FileSystemStorageFactory \
	HatchWindowFactory \
	InstanceLocker \
	MSOLEObjectSystemCreator \
	OLESimpleStorage \
	OLEEmbeddedObjectFactory \
	OOoEmbeddedObjectFactory \
	StorageFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/form,\
	ControlFontDialog \
	Forms \
	TabOrderDialog \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/form/control,\
	FilterControl \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/form/inspection,\
	DefaultFormComponentInspectorModel \
	FormComponentPropertyHandler \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/form/runtime,\
	FormController \
	FormOperations \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/frame,\
	AutoRecovery \
	AppDispatchProvider \
	Bibliography \
	ContentHandlerFactory \
	Desktop \
	DispatchHelper \
	DispatchRecorder \
	DispatchRecorderSupplier \
	DocumentTemplates \
	Frame \
	FrameLoaderFactory \
	GlobalEventBroadcaster \
	LayoutManager \
	MediaTypeDetectionHelper \
	ModuleManager \
	OfficeFrameLoader \
    SessionListener \
    StartModule \
    TaskCreator \
	UICommandDescription \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/graphic,\
	GraphicObject \
	GraphicProvider \
	Primitive2DTools \
	SvgTools \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/inspection,\
	DefaultHelpProvider \
	GenericPropertyHandler \
	ObjectInspector \
	ObjectInspectorModel \
	StringRepresentation \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/i18n,\
	BreakIterator \
	CharacterClassification \
	Collator \
	IndexEntrySupplier \
	InputSequenceChecker \
	LocaleCalendar \
	LocaleData \
	NativeNumberSupplier \
	NumberFormatMapper \
	OrdinalSuffix \
	TextConversion \
	Transliteration \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/linguistic2,\
	ConversionDictionaryList \
	DictionaryList \
	LanguageGuessing \
	LinguProperties \
	LinguServiceManager \
	Proofreader \
	ProofreadingIterator \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/logging,\
	ConsoleHandler \
	CsvLogFormatter \
	DocumentIOLogRing \
	FileHandler \
	LoggerPool \
	PlainTextFormatter \
	SimpleLogRing \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/mail,\
	MailMessage \
	MailServiceProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/media,\
	Manager \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/mozilla,\
	MozillaBootstrap \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/office,\
	Quickstart \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/packages/manifest,\
	ManifestReader \
	ManifestWriter \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/packages/zip,\
	ZipFileAccess \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/plugin,\
	PluginManager \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/presentation,\
	SlideShow \
	TransitionFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/rdf,\
	BlankNode \
	Literal \
	Repository \
	URI \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/rendering,\
	Canvas \
	CanvasFactory \
	MtfRenderer \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/report,\
	FixedLine \
	FixedText \
	FormatCondition \
	FormattedField \
	Function \
	Group \
	Groups \
	ImageControl \
	ReportControlFormat \
	ReportControlModel \
	ReportDefinition \
    ReportEngine \
	Section \
	Shape \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/report/inspection,\
	DefaultComponentInspectorModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/resource,\
	OfficeResourceLoader \
	StringResource \
	StringResourceWithLocation \
	StringResourceWithStorage \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/scanner,\
	ScannerManager \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/script,\
	DocumentDialogLibraryContainer \
	DocumentScriptLibraryContainer \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/script/browse,\
	theBrowseNodeFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/script/provider,\
	theMasterScriptProviderFactory \
	MasterScriptProviderFactory \
	ScriptURIHelper \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/script/vba,\
	VBAEventProcessor \
	VBAMacroResolver \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/sdb,\
	CommandDefinition \
	DataAccessDescriptorFactory \
	DatabaseContext \
	DatabaseInteractionHandler \
	ErrorMessageDialog \
	InteractionHandler \
    FilterDialog \
    OrderDialog \
	QueryDefinition \
	ReportDesign \
	TableDefinition \
	TextConnectionSettings \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/sdb/application,\
	CopyTableWizard \
	MacroMigrationWizard \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/sdb/tools,\
	ConnectionTools \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/sdbc,\
	ConnectionPool \
	DriverManager \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/security,\
	CertificateContainer \
	DocumentDigitalSignatures \
	SerialNumberAdapter \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/setup,\
	UpdateCheck \
	UpdateCheckConfig \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/sheet,\
	ExternalDocLink \
	ExternalDocLinks \
	ExternalSheetCache \
	FilterFormulaParser \
	GlobalSheetSettings \
	FormulaOpCodeMapper \
	RecentFunctions \
	Solver \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/smarttags,\
	SmartTagAction \
	SmartTagRecognizer \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/system,\
	SimpleCommandMail \
	SimpleSystemMail \
	SystemShellExecute \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/task,\
	InteractionHandler \
	InteractionRequestStringResolver \
	JobExecutor \
	OfficeRestartManager \
	PasswordContainer \
	PasswordContainerInteractionHandler \
	StatusIndicatorFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/text,\
	AutoTextContainer \
	DefaultNumberingProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/ucb,\
	AnyCompareFactory \
	CachedContentResultSetFactory \
	CachedContentResultSetStubFactory \
	CachedDynamicResultSetFactory \
	CachedDynamicResultSetStubFactory \
	CommandEnvironment \
	ContentProviderProxyFactory \
	PropertiesManager \
	SimpleFileAccess \
	SortedDynamicResultSetFactory \
	Store \
	UniversalContentBroker \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/ui,\
    AddressBookSourceDialog \
	DocumentAcceleratorConfiguration \
	GlobalAcceleratorConfiguration \
	ModuleAcceleratorConfiguration \
	ModuleUIConfigurationManager \
    ModuleUIConfigurationManagerSupplier \
	UICategoryDescription \
	UIConfigurationManager \
    UIElementFactoryManager \
	WindowContentFactory \
    WindowContentFactoryManager \
	WindowStateConfiguration \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/ui/dialogs,\
	AddressBookSourcePilot \
	FilePicker \
	FolderPicker \
	Wizard \
	XSLTFilterDialog \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/util,\
	JobManager \
	NumberFormatter \
	NumberFormatsSupplier \
	PathSubstitution \
	PathSettings \
	TextSearch \
	theOfficeInstallationDirectories \
	UriAbbreviation \
	URLTransformer \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/xforms,\
	Model \
	XForms \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/xml/crypto,\
	NSSInitializer \
	SecurityEnvironment \
	SEInitializer \
	XMLEncryptionTemplate \
	XMLSecurityContext \
	XMLSignatureTemplate \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/xml/dom,\
	DocumentBuilder \
	SAXDocumentBuilder \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/xml/sax,\
	FastShapeContextHandler \
	FastTokenHandler \
	Parser \
    Writer \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/xml/xpath,\
	XPathAPI \
	XPathExtension \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/com/sun/star/xml/xslt,\
	XSLTTransformer \
	XSLT2Transformer \
))


$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star,\
	modules \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/accessibility,\
	Accessible \
	AccessibleContext \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/awt,\
	AccessibleButton \
	AccessibleCheckBox \
	AccessibleComboBox \
	AccessibleDropDownComboBox \
	AccessibleDropDownListBox \
	AccessibleEdit \
	AccessibleFixedText \
	AccessibleIconChoiceControl \
	AccessibleIconChoiceControlEntry \
	AccessibleList \
	AccessibleListBox \
	AccessibleListBoxList \
	AccessibleListItem \
	AccessibleMenu \
	AccessibleMenuBar \
	AccessibleMenuItem \
	AccessibleMenuSeparator \
	AccessiblePopupMenu \
	AccessibleRadioButton \
	AccessibleScrollBar \
	AccessibleStatusBar \
	AccessibleStatusBarItem \
	AccessibleTabBar \
	AccessibleTabBarPage \
	AccessibleTabBarPageList \
	AccessibleTabControl \
	AccessibleTabPage \
	AccessibleTextField \
	AccessibleToolBox \
	AccessibleToolBoxItem \
	AccessibleTreeListBox \
	AccessibleTreeListBoxEntry \
	AccessibleWindow \
	AnimatedImagesControl \
	AnimatedImagesControlModel \
	RoadmapItem \
	SpinningProgressControlModel \
	TabControllerModel \
	UnoControl \
	UnoControlButton \
	UnoControlButtonModel \
	UnoControlCheckBox \
	UnoControlCheckBoxModel \
	UnoControlComboBox \
	UnoControlComboBoxModel \
	UnoControlContainer \
	UnoControlContainerModel \
	UnoControlCurrencyField \
	UnoControlCurrencyFieldModel \
	UnoControlDateField \
	UnoControlDateFieldModel \
	UnoControlDialogElement \
	UnoControlDialogModel \
	UnoControlEdit \
	UnoControlEditModel \
	UnoControlFileControl \
	UnoControlFileControlModel \
	UnoControlFixedHyperlink \
	UnoControlFixedHyperlinkModel \
	UnoControlFixedLine \
	UnoControlFixedLineModel \
	UnoControlFixedText \
	UnoControlFixedTextModel \
	UnoControlFormattedField \
	UnoControlFormattedFieldModel \
	UnoControlGroupBox \
	UnoControlGroupBoxModel \
	UnoControlImageControl \
	UnoControlImageControlModel \
	UnoControlListBox \
	UnoControlListBoxModel \
	UnoControlModel \
	UnoControlNumericField \
	UnoControlNumericFieldModel \
	UnoControlPatternField \
	UnoControlPatternFieldModel \
	UnoControlProgressBar \
	UnoControlProgressBarModel \
	UnoControlRadioButton \
	UnoControlRadioButtonModel \
	UnoControlRoadmap \
	UnoControlRoadmapModel \
	UnoControlScrollBar \
	UnoControlScrollBarModel \
	UnoControlSpinButton \
	UnoControlSpinButtonModel \
	UnoControlTimeField \
	UnoControlTimeFieldModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/awt/grid,\
	GridColumn \
	UnoControlGrid \
	UnoControlGridModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/awt/tab,\
	UnoControlTabPage \
	UnoControlTabPageContainer \
	UnoControlTabPageContainerModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/awt/tree,\
	TreeControlModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/chart,\
	AccessibleChartDocumentView \
	AccessibleChartElement \
	AreaDiagram \
	BarDiagram \
	BubbleDiagram \
	Chart3DBarProperties \
	ChartArea \
	ChartAxis \
	ChartAxisXSupplier \
	ChartAxisYSupplier \
	ChartAxisZSupplier \
	ChartData \
	ChartDataArray \
	ChartDataPointProperties \
	ChartDataRowProperties \
	ChartDocument \
	ChartGrid \
	ChartLegend \
	ChartLine \
	ChartPieSegmentProperties \
	ChartStatistics \
	ChartTableAddressSupplier \
	ChartTitle \
	ChartTwoAxisXSupplier \
	ChartTwoAxisYSupplier \
	Diagram \
	Dim3DDiagram \
	DonutDiagram \
	FilledNetDiagram \
	LineDiagram \
	NetDiagram \
	PieDiagram \
	StackableDiagram \
	StockDiagram \
	XYDiagram \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/chart2,\
	Axis \
	CandleStickChartType \
	ChartDocument \
	ChartDocumentWrapper \
	ChartType \
	ChartTypeManager \
	ChartTypeTemplate\
	CoordinateSystem \
	CoordinateSystemType \
	DataPoint \
	DataPointProperties \
	DataSeries \
	Diagram \
	ErrorBar \
	GridProperties \
	Legend \
	LogicTargetModel \
	PropertyPool \
	RegressionCurve \
	RegressionCurveEquation \
	StandardDiagramCreationParameters \
	Title \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/chart2/data,\
	DataFilter \
	DataProvider \
	DataSequence \
	DataSink \
	DataSource \
	RangeHighlighter \
	RangeHighlightListener \
	TabularDataProviderArguments \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/configuration,\
	AccessRootElement \
	AdministrationProvider \
	ConfigurationAccess \
	ConfigurationProvider \
	ConfigurationRegistry \
	ConfigurationUpdateAccess \
	DefaultProvider \
	GroupAccess \
	GroupElement \
	GroupUpdate \
	HierarchyAccess \
	HierarchyElement \
	PropertyHierarchy \
	SetAccess \
	SetElement \
	SetUpdate \
	SimpleSetAccess \
	SimpleSetUpdate \
	UpdateRootElement \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/configuration/backend,\
	Backend \
	BackendAdapter \
	CopyImporter \
	DefaultBackend \
	DataImporter \
	HierarchyBrowser \
	Importer \
	InteractionHandler \
	Layer \
	LayerDescriber \
	LayerFilter \
	LayerUpdateMerger \
	LdapMultiLayerStratum \
	LdapSingleBackend \
	LdapSingleStratum \
	LocalDataImporter \
	LocalHierarchyBrowser \
	LocalSchemaSupplier \
	LocalSingleBackend \
	LocalSingleStratum \
	MergeImporter \
	MultiLayerStratum \
	MultiStratumBackend \
	OfflineBackend \
	OnlineBackend \
	PlatformBackend \
	Schema \
	SchemaSupplier \
	SingleBackend \
	SingleBackendAdapter \
	SingleLayerStratum \
	SystemIntegration \
	UpdatableLayer \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/configuration/backend/xml,\
	LayerParser \
	LayerWriter \
	SchemaParser \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/configuration/bootstrap,\
	BootstrapContext \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/datatransfer/clipboard,\
	ClipboardManager \
	GenericClipboard \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/datatransfer/dnd,\
	OleDragSource \
	OleDropTarget \
	X11DragSource \
	X11DropTarget \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/document,\
	EventDescriptor \
	Events \
	ExportFilter \
	ExtendedTypeDetection \
	ExtendedTypeDetectionFactory \
	FilterAdapter \
	FilterFactory \
	HeaderFooterSettings \
	ImportFilter \
	LinkTarget \
	LinkTargets \
	MediaDescriptor \
	OfficeDocument \
	PDFDialog \
	Settings \
	TypeDetection \
	XMLBasicImporter \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/drawing,\
	AccessibleDrawDocumentView \
	AccessibleGraphControl \
	AccessibleGraphicShape \
	AccessibleImageBullet \
	AccessibleOLEShape \
	AccessibleShape \
	AccessibleSlideView \
	AccessibleSlideViewObject \
	AppletShape \
	Background \
	BitmapTable \
	CaptionShape \
	ClosedBezierShape \
	ConnectorProperties \
	ConnectorShape \
	ControlShape \
	CustomShape \
	CustomShapeEngine \
	DashTable \
	Defaults \
	DocumentSettings \
	DrawPage \
	DrawPages \
	DrawingDocument \
	DrawingDocumentDrawView \
	DrawingDocumentFactory \
	EllipseShape \
	EnhancedCustomShapeExtrusion \
	EnhancedCustomShapeGeometry \
	EnhancedCustomShapeHandle \
	EnhancedCustomShapePath \
	EnhancedCustomShapeTextPath \
	FillProperties \
	GenericDrawPage \
	GenericDrawingDocument \
	GradientTable \
	GraphicObjectShape \
	GroupShape \
	HatchTable \
	Layer \
	LayerManager \
	LineProperties \
	LineShape \
	MarkerTable \
	MasterPage \
	MasterPages \
	MeasureProperties \
	MeasureShape \
	OLE2Shape \
	OpenBezierShape \
	PageShape \
	PluginShape \
	PolyLineShape \
	PolyPolygonBezierDescriptor \
	PolyPolygonBezierShape \
	PolyPolygonDescriptor \
	PolyPolygonShape \
	RectangleShape \
	RotationDescriptor \
	ShadowProperties \
	Shape \
	Shapes \
	Text \
	TextProperties \
	TextShape \
	TransparencyGradientTable \
	modules \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/embed,\
	BaseStorage \
	EmbeddedObjectDescriptor \
	FileSystemStorage \
	Storage \
	StorageStream \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/form,\
	DataAwareControlModel \
	FormComponent \
	FormComponents \
	FormControlModel \
	FormController \
	FormControllerDispatcher \
	PropertyBrowserController \
	modules \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/form/binding,\
	BindableControlModel \
	BindableDataAwareControlModel \
	BindableDatabaseCheckBox \
	BindableDatabaseComboBox \
	BindableDatabaseDateField \
	BindableDatabaseFormattedField \
	BindableDatabaseListBox \
	BindableDatabaseNumericField \
	BindableDatabaseRadioButton \
	BindableDatabaseTextField \
	BindableDatabaseTimeField \
	BindableIntegerValueRange \
	ListEntrySource \
	ValueBinding \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/form/component,\
	CheckBox \
	ComboBox \
	CommandButton \
	CurrencyField \
	DataForm \
	DatabaseCheckBox \
	DatabaseComboBox \
	DatabaseCurrencyField \
	DatabaseDateField \
	DatabaseFormattedField \
	DatabaseImageControl \
	DatabaseListBox \
	DatabaseNumericField \
	DatabasePatternField \
	DatabaseRadioButton \
	DatabaseTextField \
	DatabaseTimeField \
	DateField \
	FileControl \
	FixedText \
	Form \
	FormattedField \
	GridControl \
	GroupBox \
	HTMLForm \
	HiddenControl \
	ImageButton \
	ListBox \
	NavigationToolBar \
	NumericField \
	PatternField \
	RadioButton \
	RichTextControl \
	ScrollBar \
	SpinButton \
	SubmitButton \
	TextField \
	TimeField \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/form/control,\
	CheckBox \
	ComboBox \
	CommandButton \
	CurrencyField \
	DateField \
	FormattedField \
	GridControl \
	GroupBox \
	ImageButton \
	ImageControl \
	InteractionGridControl \
	ListBox \
	NavigationToolBar \
	NumericField \
	PatternField \
	RadioButton \
	SubmitButton \
	TextField \
	TimeField \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/form/inspection,\
	ButtonNavigationHandler \
	CellBindingPropertyHandler \
	EditPropertyHandler \
	EventHandler \
	SubmissionPropertyHandler \
	XMLFormsPropertyHandler \
	XSDValidationPropertyHandler \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/form/validation,\
	ValidatableBindableControlModel \
	ValidatableControlModel \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/formula,\
	AccessibleFormulaText \
	AccessibleFormulaView \
	FormulaProperties \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/frame,\
	Components \
	ContentHandler \
	Controller \
	DesktopTask \
	DesktopTasks \
	DispatchProvider \
	FrameControl \
	FrameLoader \
	FramesContainer \
	PopupMenuController \
	PopupMenuControllerFactory \
	ProtocolHandler \
	SessionManager \
	Settings \
	StatusbarController \
	StatusbarControllerFactory \
	SynchronousFrameLoader \
	Task \
	TemplateAccess \
	ToolbarController \
	ToolbarControllerFactory \
	TransientDocumentsDocumentContentFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/gallery,\
	GalleryItem \
	GalleryTheme \
	GalleryThemeProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/graphic,\
	Graphic \
	GraphicDescriptor \
	GraphicRasterizer \
	GraphicRendererVCL \
	MediaProperties \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/i18n,\
	ChapterCollator \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/image,\
	ImageMap \
	ImageMapCircleObject \
	ImageMapObject \
	ImageMapPolygonObject \
	ImageMapRectangleObject \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/linguistic2,\
	ConversionDictionary \
	Dictionary \
	HangulHanjaConversionDictionary \
	Hyphenator \
	SpellChecker \
	Thesaurus \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/mozilla,\
	MenuProxy \
	MenuProxyListener \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/packages,\
	Package \
	PackageFolder \
	PackageFolderEnumeration \
	PackageStream \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/presentation,\
	ChartShape \
	CustomPresentation \
	CustomPresentationAccess \
	DateTimeShape \
	DocumentSettings \
	DrawPage \
	FooterShape \
	GraphicObjectShape \
	HandoutShape \
	HandoutView \
	HeaderShape \
	NotesShape \
	NotesView \
	OLE2Shape \
	OutlineView \
	OutlinerShape \
	PageShape \
	Presentation \
	Presentation2 \
	PresentationDocument \
	PresentationView \
	PreviewView \
	Shape \
	SlideNumberShape \
	SlidesView \
	SubtitleShape \
	TitleTextShape \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/presentation/textfield,\
	DateTime \
	Footer \
	Header \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/rendering,\
	BitmapCanvas \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/report,\
	modules \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/report/inspection,\
	DataProviderHandler \
	ReportComponentHandler \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/script/browse,\
	BrowseNode \
	BrowseNodeFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/script/provider,\
	LanguageScriptProvider \
	MasterScriptProvider \
	ScriptProvider \
	ScriptProviderForBasic \
	ScriptProviderForBeanShell \
	ScriptProviderForJava \
	ScriptProviderForJavaScript \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/script/vba,\
	VBASpreadsheetEventProcessor \
	VBATextEventProcessor \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/sdb,\
	CallableStatement \
	Column \
	ColumnDescriptorControl \
	ColumnDescriptorControlModel \
	ColumnSettings \
	Connection \
	ContentLoader \
	DataAccessDescriptor \
	DataColumn \
	DataSettings \
	DataSource \
	DataSourceBrowser \
	DatabaseAccess \
	DatabaseAccessConnection \
	DatabaseAccessContext \
	DatabaseAccessDataSource \
	DatabaseDocument \
	DatabaseEnvironment \
	DatasourceAdministrationDialog \
	DefinitionContainer \
	DefinitionContent \
	Document \
	DocumentContainer \
	DocumentDataSource \
	DocumentDefinition \
	Forms \
	OfficeDatabaseDocument \
	OrderColumn \
	PreparedStatement \
	Query \
	QueryDescriptor \
	QueryDesign \
	RelationDesign \
	Reports \
	ResultColumn \
	ResultSet \
	RowSet \
	SQLQueryComposer \
	SingleSelectQueryAnalyzer \
	SingleSelectQueryComposer \
	Table \
	TableDescriptor \
	TableDesign \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/sdb/application,\
	DefaultViewController \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/sdbc,\
	CallableStatement \
	Connection \
	ConnectionProperties \
	DBASEConnectionProperties \
	Driver \
	FILEConnectionProperties \
	FLATConnectionProperties \
	JDBCConnectionProperties \
	ODBCConnectionProperties \
	PreparedStatement \
	ResultSet \
	RowSet \
	Statement \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/sdbcx,\
	Column \
	ColumnDescriptor \
	Container \
	DatabaseDefinition \
	Descriptor \
	Driver \
	Group \
	GroupDescriptor \
	Index \
	IndexColumn \
	IndexColumnDescriptor \
	IndexDescriptor \
	Key \
	KeyColumn \
	KeyColumnDescriptor \
	KeyDescriptor \
	PreparedStatement \
	ReferenceColumn \
	ResultSet \
	Statement \
	Table \
	TableDescriptor \
	User \
	UserDescriptor \
	View \
	ViewDescriptor \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/sheet,\
	AccessibleCell \
	AccessibleCsvCell \
	AccessibleCsvRuler \
	AccessibleCsvTable \
	AccessiblePageHeaderFooterAreasView \
	AccessibleSpreadsheet \
	AccessibleSpreadsheetDocumentView \
	AccessibleSpreadsheetPageView \
	AddIn \
	CellAnnotation \
	CellAnnotationShape \
	CellAnnotations \
	CellAnnotationsEnumeration \
	CellAreaLink \
	CellAreaLinks \
	CellAreaLinksEnumeration \
	CellFormatRanges \
	CellFormatRangesEnumeration \
	Cells \
	CellsEnumeration \
	ConsolidationDescriptor \
	DDELink \
	DDELinks \
	DDELinksEnumeration \
	DataPilotDescriptor \
	DataPilotField \
	DataPilotFieldGroup \
	DataPilotFieldGroupEnumeration \
	DataPilotFieldGroupItem \
	DataPilotFieldGroups \
	DataPilotFieldGroupsEnumeration \
	DataPilotFields \
	DataPilotFieldsEnumeration \
	DataPilotItem \
	DataPilotItems \
	DataPilotItemsEnumeration \
	DataPilotSource \
	DataPilotSourceDimension \
	DataPilotSourceDimensions \
	DataPilotSourceHierarchies \
	DataPilotSourceHierarchy \
	DataPilotSourceLevel \
	DataPilotSourceLevels \
	DataPilotSourceMember \
	DataPilotSourceMembers \
	DataPilotTable \
	DataPilotTables \
	DataPilotTablesEnumeration \
	DatabaseImportDescriptor \
	DatabaseRange \
	DatabaseRanges \
	DatabaseRangesEnumeration \
	DocumentSettings \
	FormulaParser \
	FunctionAccess \
	FunctionDescription \
	FunctionDescriptionEnumeration \
	FunctionDescriptions \
	HeaderFooterContent \
	LabelRange \
	LabelRanges \
	LabelRangesEnumeration \
	NamedRange \
	NamedRanges \
	NamedRangesEnumeration \
	RangeSelectionArguments \
	Scenario \
	Scenarios \
	ScenariosEnumeration \
	Shape \
	SheetCell \
	SheetCellCursor \
	SheetCellRange \
	SheetCellRanges \
	SheetCellRangesEnumeration \
	SheetFilterDescriptor \
	SheetLink \
	SheetLinks \
	SheetLinksEnumeration \
	SheetRangesQuery \
	SheetSortDescriptor \
	SheetSortDescriptor2 \
	Spreadsheet \
	SpreadsheetDocument \
	SpreadsheetDocumentSettings \
	SpreadsheetDrawPage \
	SpreadsheetView \
	SpreadsheetViewPane \
	SpreadsheetViewPanesEnumeration \
	SpreadsheetViewSettings \
	Spreadsheets \
	SpreadsheetsEnumeration \
	SubTotalDescriptor \
	SubTotalField \
	SubTotalFieldsEnumeration \
	TableAutoFormat \
	TableAutoFormatEnumeration \
	TableAutoFormatField \
	TableAutoFormats \
	TableAutoFormatsEnumeration \
	TableCellStyle \
	TableConditionalEntry \
	TableConditionalEntryEnumeration \
	TableConditionalFormat \
	TablePageStyle \
	TableValidation \
	UniqueCellFormatRanges \
	UniqueCellFormatRangesEnumeration \
	VolatileResult \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/style,\
	CellStyle \
	CharacterProperties \
	CharacterPropertiesAsian \
	CharacterPropertiesComplex \
	CharacterStyle \
	NumberingAlignment \
	NumberingLevel \
	NumberingRule \
	PageProperties \
	PageStyle \
	ParagraphProperties \
	ParagraphPropertiesAsian \
	ParagraphPropertiesComplex \
	ParagraphStyle \
	Style \
	StyleFamilies \
	StyleFamily \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/table,\
	AccessibleCellView \
	AccessibleTableView \
	Cell \
	CellCursor \
	CellProperties \
	CellRange \
	CellRangeListSource \
	CellValueBinding \
	ListPositionCellBinding \
	TableChart \
	TableCharts \
	TableChartsEnumeration \
	TableColumn \
	TableColumns \
	TableColumnsEnumeration \
	TableRow \
	TableRows \
	TableRowsEnumeration \
	TableSortDescriptor \
	TableSortDescriptor2 \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/task,\
	AsyncJob \
	Job \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/text,\
	AccessibleEndnoteView \
	AccessibleFootnoteView \
	AccessibleHeaderFooterView \
	AccessiblePageView \
	AccessibleParagraphView \
	AccessibleTextDocumentPageView \
	AccessibleTextDocumentView \
	AccessibleTextEmbeddedObject \
	AccessibleTextFrameView \
	AccessibleTextGraphicObject \
	AutoTextEntry \
	AutoTextGroup \
	BaseFrame \
	BaseFrameProperties \
	BaseIndex \
	BaseIndexMark \
	Bibliography \
	Bookmark \
	Bookmarks \
	Cell \
	CellProperties \
	CellRange \
	ChainedTextFrame \
	ChapterNumberingRule \
	ContentIndex \
	ContentIndexMark \
	Defaults \
	DependentTextField \
	DocumentIndex \
	DocumentIndexLevelFormat \
	DocumentIndexMark \
	DocumentIndexMarkAsian \
	DocumentIndexParagraphStyles \
	DocumentIndexes \
	DocumentSettings \
	Endnote \
	EndnoteSettings \
	Footnote \
	FootnoteSettings \
	Footnotes \
	GenericTextDocument \
	GlobalDocument \
	GlobalSettings \
	IllustrationsIndex \
	InContentMetadata \
	LineNumberingProperties \
	MailMerge \
	NumberingLevel \
	NumberingRules \
	NumberingStyle \
	ObjectIndex \
	PageFootnoteInfo \
	PagePrintSettings \
	Paragraph \
	ParagraphEnumeration \
	PrintSettings \
	RedlinePortion \
	ReferenceMark \
	ReferenceMarks \
	Shape \
	TableColumns \
	TableIndex \
	TableRows \
	Text \
	TextColumns \
	TextContent \
	TextContentCollection \
	TextCursor \
	TextDocument \
	TextDocumentView \
	TextEmbeddedObject \
	TextEmbeddedObjects \
	TextField \
	TextFieldEnumeration \
	TextFieldMaster \
	TextFieldMasters \
	TextFields \
	TextFrame \
	TextFrames \
	TextGraphicObject \
	TextGraphicObjects \
	TextLayoutCursor \
	TextPageStyle \
	TextPortion \
	TextPortionEnumeration \
	TextRange \
	TextRangeContentProperties \
	TextRanges \
	TextSection \
	TextSections \
	TextSortDescriptor \
	TextSortDescriptor2 \
	TextSortable \
	TextTable \
	TextTableCursor \
	TextTableRow \
	TextTables \
	TextViewCursor \
	UserDefinedIndex \
	UserIndex \
	UserIndexMark \
	ViewSettings \
	WebDocument \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/text/fieldmaster,\
	Bibliography \
	DDE \
	Database \
	SetExpression \
	User \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/text/textfield,\
	Annotation \
	Author \
	Bibliography \
	Chapter \
	CharacterCount \
	CombinedCharacters \
	ConditionalText \
	DDE \
	Database \
	DatabaseName \
	DatabaseNextSet \
	DatabaseNumberOfSet \
	DatabaseSetNumber \
	DateTime \
	DropDown \
	EmbeddedObjectCount \
	ExtendedUser \
	FileName \
	GetExpression \
	GetReference \
	GraphicObjectCount \
	HiddenParagraph \
	HiddenText \
	Input \
	InputUser \
	JumpEdit \
	Macro \
	MetadataField \
	PageCount \
	PageNumber \
	ParagraphCount \
	ReferencePageGet \
	ReferencePageSet \
	Script \
	SetExpression \
	TableCount \
	TableFormula \
	TemplateName \
	URL \
	User \
	WordCount \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/text/textfield,\
	Type \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/text/textfield/docinfo,\
	ChangeAuthor \
	ChangeDateTime \
	CreateAuthor \
	CreateDateTime \
	Custom \
	Description \
	EditTime \
	Keywords \
	PrintAuthor \
	PrintDateTime \
	Revision \
	Subject \
	Title \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/ucb,\
	CachedContentResultSet \
	CachedContentResultSetStub \
	CachedDynamicResultSet \
	CachedDynamicResultSetStub \
	CmisContentProvider \
	Content \
	ContentProvider \
	ContentProviderProxy \
	ContentResultSet \
	ContentTransmitter \
	DefaultHierarchyDataSource \
	DynamicResultSet \
	ExpandContentProvider \
	FTPContent \
	FTPContentProvider \
	FileContent \
	FileContentProvider \
	GIOContentProvider \
	GnomeVFSContentProvider \
	GnomeVFSDocumentContent \
	GnomeVFSFolderContent \
	HelpContent \
	HelpContentProvider \
	HierarchyContentProvider \
	HierarchyDataReadAccess \
	HierarchyDataReadWriteAccess \
	HierarchyDataSource \
	HierarchyFolderContent \
	HierarchyLinkContent \
	HierarchyRootFolderContent \
	ODMAContent \
	ODMAContentProvider \
	PackageContentProvider \
	PackageFolderContent \
	PackageStreamContent \
	PersistentPropertySet \
	PropertySetRegistry \
	RemoteAccessContentProvider \
	RemoteContentProviderAcceptor \
	RemoteProxyContentProvider \
	TransientDocumentsContentProvider \
	TransientDocumentsDocumentContent \
	TransientDocumentsFolderContent \
	TransientDocumentsRootContent \
	TransientDocumentsStreamContent \
	WebDAVContentProvider \
	WebDAVDocumentContent \
	WebDAVFolderContent \
	WebDAVHTTPMethod \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/ui,\
	ActionTrigger \
	ActionTriggerContainer \
	ActionTriggerSeparator \
	ConfigurableUIElement \
	ItemDescriptor \
	ModuleUICategoryDescription \
	ModuleUICommandDescription \
	ModuleWindowStateConfiguration \
	UIElement \
	UIElementFactory \
	UIElementSettings \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/ui/dialogs,\
	FilterOptionsDialog \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/util,\
	NumberFormatProperties \
	NumberFormatSettings \
	NumberFormats \
	OfficeInstallationDirectories \
	ReplaceDescriptor \
	SearchDescriptor \
	SortDescriptor \
	SortDescriptor2 \
	Sortable \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/view,\
	OfficeDocumentView \
	PrintOptions \
	PrintSettings \
	PrinterDescriptor \
	RenderDescriptor \
	RenderOptions \
	ViewSettings \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xforms,\
	Binding \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xml,\
	AttributeContainer \
	ExportFilter \
	ImportFilter \
	NamespaceContainer \
	ParaUserDefinedAttributesSupplier \
	TextUserDefinedAttributesSupplier \
	UserDefinedAttributesSupplier \
	XMLExportFilter \
	XMLImportFilter \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xml/crypto,\
	XMLEncryption \
	XMLSignature \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xml/crypto/sax,\
	Decryptor \
	Encryptor \
	SAXEventKeeper \
	SignatureCreator \
	SignatureVerifier \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xml/input,\
	SaxDocumentHandler \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xml/wrapper,\
	XMLDocumentWrapper \
	XMLElementWrapper \
))
$(eval $(call gb_UnoApi_add_idlfiles_noheader,offapi,offapi/com/sun/star/xsd,\
	Boolean \
	Date \
	DateTime \
	Day \
	Decimal \
	Month \
	String \
	Time \
	Year \
))


$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/accessibility,\
	AccessibleEventId \
	AccessibleEventObject \
	AccessibleRelation \
	AccessibleRelationType \
	AccessibleRole \
	AccessibleStateType \
	AccessibleTableModelChange \
	AccessibleTableModelChangeType \
	AccessibleTextType \
	IllegalAccessibleComponentStateException \
	TextSegment \
	XAccessible \
	XAccessibleAction \
	XAccessibleComponent \
	XAccessibleContext \
	XAccessibleEditableText \
	XAccessibleEventBroadcaster \
	XAccessibleEventListener \
	XAccessibleExtendedComponent \
	XAccessibleHyperlink \
	XAccessibleHypertext \
	XAccessibleImage \
	XAccessibleKeyBinding \
	XAccessibleMultiLineText \
	XAccessibleRelationSet \
	XAccessibleSelection \
	XAccessibleStateSet \
	XAccessibleTable \
	XAccessibleText \
	XAccessibleTextAttributes \
	XAccessibleTextMarkup \
	XAccessibleValue \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/animations,\
	AnimationAdditiveMode \
	AnimationCalcMode \
	AnimationColorSpace \
	AnimationEndSync \
	AnimationFill \
	AnimationNodeType \
	AnimationRestart \
	AnimationTransformType \
	AnimationValueType \
	Event \
	EventTrigger \
	TargetProperties \
	TimeFilterPair \
	Timing \
	TransitionSubType \
	TransitionType \
	ValuePair \
	XAnimate \
	XAnimateColor \
	XAnimateMotion \
	XAnimateSet \
	XAnimateTransform \
	XAnimationListener \
	XAnimationNode \
	XAnimationNodeSupplier \
	XAudio \
	XCommand \
	XIterateContainer \
    XParallelTimeContainer \
	XTargetPropertiesCreator \
	XTimeContainer \
	XTransitionFilter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/auth,\
	AuthenticationFailedException \
	InvalidArgumentException \
	InvalidContextException \
	InvalidCredentialException \
	InvalidPrincipalException \
	PersistenceFailureException \
	UnsupportedException \
	XSSOAcceptorContext \
	XSSOContext \
	XSSOInitiatorContext \
	XSSOManager \
	XSSOManagerFactory \
	XSSOPasswordCache \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/awt,\
	ActionEvent \
	AdjustmentEvent \
	AdjustmentType \
	CharSet \
	Command \
	DeviceCapability \
	DeviceInfo \
	DockingData \
	DockingEvent \
	EndDockingEvent \
	EndPopupModeEvent \
	EnhancedMouseEvent \
	FieldUnit \
	FocusChangeReason \
	FocusEvent \
	FontDescriptor \
	FontEmphasisMark \
	FontFamily \
	FontPitch \
	FontRelief \
	FontSlant \
	FontStrikeout \
	FontType \
	FontUnderline \
	FontWeight \
	FontWidth \
	Gradient \
	GradientStyle \
	ImageDrawMode \
	ImageAlign \
	ImagePosition \
	ImageScaleMode \
	ImageStatus \
	InputEvent \
	InvalidateStyle \
	ItemEvent \
	ItemListEvent \
	Key \
	KeyEvent \
	KeyFunction \
	KeyGroup \
	KeyModifier \
	KeyStroke \
	LineEndFormat \
	MenuEvent \
	MenuItemStyle \
	MenuItemType \
	MessageBoxButtons \
	MessageBoxResults \
	MessageBoxType \
	MouseButton \
	MouseEvent \
	MouseWheelBehavior \
	PaintEvent \
	Point \
	PopupMenuDirection \
	PosSize \
	PrinterException \
	PrinterServer \
	PushButtonType \
	RasterOperation \
	Rectangle \
	ScrollBarOrientation \
	Selection \
	SimpleFontMetric \
	Size \
	SpinEvent \
	Style \
	SystemDependentXWindow \
	SystemPointer \
	TextAlign \
	TextEvent \
	VclContainerEvent \
	VclWindowPeerAttribute \
	VisualEffect \
	WindowAttribute \
	WindowClass \
	WindowDescriptor \
	WindowEvent \
	XActionListener \
	XActivateListener \
	XAdjustmentListener \
	XAnimatedImages \
	XAnimation \
	XBitmap \
	XButton \
	XCallback \
	XCheckBox \
	XComboBox \
	XContainerWindowEventHandler \
	XContainerWindowProvider \
	XControl \
	XControlContainer \
	XControlModel \
	XCurrencyField \
	XDataTransferProviderAccess \
	XDateField \
	XDevice \
	XDialog \
	XDialog2 \
	XDialogEventHandler \
	XDialogProvider \
	XDialogProvider2 \
	XDisplayBitmap \
	XDisplayConnection \
	XDockableWindow \
	XDockableWindowListener \
	XEnhancedMouseClickHandler \
	XEventHandler \
	XExtendedToolkit \
	XFileDialog \
	XFixedHyperlink \
	XFixedText \
	XFocusListener \
	XFont \
	XFont2 \
	XGraphics \
	XGraphics2 \
	XImageButton \
	XImageConsumer \
	XImageProducer \
	XInfoPrinter \
	XItemEventBroadcaster \
	XItemList \
	XItemListListener \
	XItemListener \
	XKeyHandler \
	XKeyListener \
	XLayoutConstrains \
	XListBox \
	XMenu \
	XMenuBar \
	XMenuListener \
	XMessageBox \
	XMessageBoxFactory \
	XMetricField \
	XMouseClickHandler \
	XMouseListener \
	XMouseMotionHandler \
	XMouseMotionListener \
	XNumericField \
	XPaintListener \
	XPatternField \
	XPointer \
	XPopupMenu \
	XPrinter \
	XPrinterPropertySet \
	XPrinterServer \
	XProgressBar \
	XProgressMonitor \
	XRadioButton \
	XRegion \
	XRequestCallback \
	XReschedule \
	XScrollBar \
	XSimpleTabController \
	XSpinField \
	XSpinListener \
	XSpinValue \
	XStyleChangeListener \
	XStyleSettings \
	XStyleSettingsSupplier \
	XSystemChildFactory \
	XSystemDependentMenuPeer \
	XSystemDependentWindowPeer \
	XTabController \
	XTabControllerModel \
	XTabListener \
	XTextArea \
	XTextComponent \
	XTextEditField \
	XTextLayoutConstrains \
	XTextListener \
	XTimeField \
	XToggleButton \
	XToolkit \
	XToolkit2 \
	XToolkitExperimental \
	XTopWindow \
	XTopWindow2 \
	XTopWindowListener \
	XUnitConversion \
	XUnoControlContainer \
	XUnoControlDialog \
	XUserInputInterception \
	XVclContainer \
	XVclContainerListener \
	XVclContainerPeer \
	XVclWindowPeer \
	XView \
	XWindow \
	XWindow2 \
	XWindowListener \
	XWindowListener2 \
	XWindowPeer \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/awt/grid,\
	GridColumnEvent \
	GridDataEvent \
	GridInvalidDataException \
	GridInvalidModelException \
	GridSelectionEvent \
	XGridColumn \
	XGridColumnListener \
	XGridColumnModel \
	XGridControl \
	XGridDataListener \
	XGridDataModel \
	XGridRowSelection \
	XGridSelectionListener \
	XMutableGridDataModel \
	XSortableGridData \
	XSortableMutableGridDataModel \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/awt/tab,\
	TabPageActivatedEvent \
	XTabPage \
	XTabPageContainer \
	XTabPageContainerListener \
	XTabPageContainerModel \
	XTabPageModel \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/awt/tree,\
	ExpandVetoException \
	TreeDataModelEvent \
	TreeExpansionEvent \
	XMutableTreeDataModel \
	XMutableTreeNode \
	XTreeControl \
	XTreeDataModel \
	XTreeDataModelListener \
	XTreeEditListener \
	XTreeExpansionListener \
	XTreeNode \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/chart,\
	ChartAxisArrangeOrderType \
	ChartAxisAssign \
	ChartAxisLabelPosition \
	ChartAxisMarkPosition \
	ChartAxisMarks \
	ChartAxisPosition \
	ChartAxisType \
	ChartDataCaption \
	ChartDataChangeEvent \
	ChartDataChangeType \
	ChartDataPoint \
	ChartDataRow \
	ChartDataRowSource \
	ChartDataValue \
	ChartErrorCategory \
	ChartErrorIndicatorType \
	ChartLegendExpansion \
	ChartLegendPosition \
	ChartRegressionCurveType \
	ChartSeriesAddress \
	ChartSolidType \
	ChartSymbolType \
	DataLabelPlacement \
	ErrorBarStyle \
	MissingValueTreatment \
	TimeIncrement \
	TimeInterval \
	TimeUnit \
	X3DDefaultSetter \
	X3DDisplay \
	XAxis \
	XAxisSupplier \
	XAxisXSupplier \
	XAxisYSupplier \
	XAxisZSupplier \
	XChartData \
	XChartDataArray \
	XChartDataChangeEventListener \
	XChartDocument \
	XComplexDescriptionAccess \
	XDateCategories \
	XDiagram \
	XDiagramPositioning \
	XSecondAxisTitleSupplier \
	XStatisticDisplay \
	XTwoAxisXSupplier \
	XTwoAxisYSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/chart2,\
	AxisOrientation \
	AxisType \
	CoordinateSystemTypeID \
	CurveStyle \
	DataPointGeometry3D \
	DataPointLabel \
	FillBitmap \
	IncrementData \
	InterpretedData \
	LegendPosition \
	LightSource \
	PieChartOffsetMode \
	RelativePosition \
	RelativeSize \
	ScaleData \
	StackingDirection \
	SubIncrement \
	Symbol \
	SymbolStyle \
	TickmarkStyle \
	TransparencyStyle \
	XAnyDescriptionAccess \
	XAxis \
	XChartDocument \
	XChartShape \
	XChartShapeContainer \
	XChartType \
	XChartTypeContainer \
	XChartTypeManager \
	XChartTypeTemplate \
	XColorScheme \
	XCoordinateSystem \
	XCoordinateSystemContainer \
	XDataInterpreter \
	XDataSeries \
	XDataSeriesContainer \
	XDefaultSizeTransmitter \
	XDiagram \
	XDiagramProvider \
	XFormattedString \
	XFormattedString2 \
	XInternalDataProvider \
	XLabeled \
	XLegend \
	XRegressionCurve \
	XRegressionCurveCalculator \
	XRegressionCurveContainer \
	XScaling \
	XTarget \
	XTitle \
	XTitled \
	XTransformation \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/chart2/data,\
	DataSequenceRole \
	HighlightedRange \
	LabelOrigin \
	XDataProvider \
	XDataReceiver \
	XDataSequence \
	XDataSink \
	XDataSource \
	XDatabaseDataProvider \
	XLabeledDataSequence \
	XLabeledDataSequence2 \
	XNumericalDataSequence \
	XRangeHighlighter \
	XRangeXMLConversion \
	XSheetDataProvider \
	XTextualDataSequence \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/configuration,\
	CannotLoadConfigurationException \
	CorruptedConfigurationException \
	CorruptedUIConfigurationException \
	InstallationIncompleteException \
	InvalidBootstrapFileException \
	MissingBootstrapFileException \
    XReadWriteAccess \
	XTemplateContainer \
	XTemplateInstance \
	XUpdate \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/configuration/backend,\
	AuthenticationFailedException \
	BackendAccessException \
	BackendSetupException \
	CannotConnectException \
	ComponentChangeEvent \
	ConnectionLostException \
	InsufficientAccessRightsException \
	InvalidAuthenticationMechanismException \
	MalformedDataException \
	MergeRecoveryRequest \
	NodeAttribute \
	PropertyInfo \
	SchemaAttribute \
	StratumCreationException \
	TemplateIdentifier \
	XBackend \
	XBackendChangesListener \
	XBackendChangesNotifier \
	XBackendEntities \
	XCompositeLayer \
	XLayer \
	XLayerContentDescriber \
	XLayerHandler \
	XLayerImporter \
	XMultiLayerStratum \
	XSchema \
	XSchemaHandler \
	XSchemaSupplier \
	XSingleLayerStratum \
	XUpdatableLayer \
	XUpdateHandler \
	XVersionedSchemaSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/datatransfer,\
	DataFlavor \
	UnsupportedFlavorException \
	XDataFormatTranslator \
	XMimeContentType \
	XMimeContentTypeFactory \
	XSystemTransferable \
	XTransferDataAccess \
	XTransferable \
	XTransferableEx \
	XTransferableSource \
	XTransferableSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/datatransfer/clipboard,\
	ClipboardEvent \
	RenderingCapabilities \
	XClipboard \
	XClipboardEx \
	XClipboardFactory \
	XClipboardListener \
	XClipboardManager \
	XClipboardNotifier \
	XClipboardOwner \
	XFlushableClipboard \
	XSystemClipboard \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/datatransfer/dnd,\
	DNDConstants \
	DragGestureEvent \
	DragSourceDragEvent \
	DragSourceDropEvent \
	DragSourceEvent \
	DropTargetDragEnterEvent \
	DropTargetDragEvent \
	DropTargetDropEvent \
	DropTargetEvent \
	InvalidDNDOperationException \
	XAutoscroll \
	XDragGestureListener \
	XDragGestureRecognizer \
	XDragSource \
	XDragSourceContext \
	XDragSourceListener \
	XDropTarget \
	XDropTargetDragContext \
	XDropTargetDropContext \
	XDropTargetListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/deployment,\
	DependencyException \
	DeploymentException \
	ExtensionRemovedException \
	InstallException \
	InvalidRemovedParameterException \
	LicenseException \
	PlatformException \
	Prerequisites \
	UpdateInformationEntry \
	VersionException \
	XExtensionManager \
	XPackage \
	XPackageInformationProvider \
	XPackageManager \
	XPackageManagerFactory \
	XPackageRegistry \
	XPackageTypeInfo \
	XUpdateInformationProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/document,\
	AmbigousFilterRequest \
	BrokenPackageRequest \
	ChangedByOthersRequest \
	CmisProperty \
	CorruptedFilterConfigurationException \
	DocumentEvent \
	EmptyUndoStackException \
	EventObject \
	FilterOptionsRequest \
	LinkUpdateModes \
	LockFileIgnoreRequest \
	LockedDocumentRequest \
	LockedOnSavingRequest \
	MacroExecMode \
	NoSuchFilterRequest \
	OwnLockOnDocumentRequest \
	PrinterIndependentLayout \
	RedlineDisplayType \
	UndoContextNotClosedException \
	UndoFailedException \
	UndoManagerEvent \
	UpdateDocMode \
	XActionLockable \
	XBinaryStreamResolver \
	XCmisDocument \
	XCodeNameQuery \
	XCompatWriterDocProperties \
	XDocumentEventBroadcaster \
	XDocumentEventListener \
	XDocumentInsertable \
	XDocumentLanguages \
	XDocumentProperties \
	XDocumentPropertiesSupplier \
	XDocumentRecovery \
	XDocumentRevisionListPersistence \
	XDocumentSubStorageSupplier \
	XEmbeddedObjectResolver \
	XEmbeddedObjectSupplier \
	XEmbeddedObjectSupplier2 \
	XEmbeddedScripts \
	XEventBroadcaster \
	XEventListener \
	XEventsSupplier \
	XExporter \
	XExtendedFilterDetection \
	XFilter \
	XFilterAdapter \
	XGraphicObjectResolver \
	XImporter \
	XInteractionFilterOptions \
	XInteractionFilterSelect \
	XLinkTargetSupplier \
	XMimeTypeInfo \
	XOOXMLDocumentPropertiesImporter \
	XRedlinesSupplier \
	XScriptInvocationContext \
	XStorageBasedDocument \
	XStorageChangeListener \
	XTypeDetection \
	XUndoAction \
	XUndoManager \
	XUndoManagerListener \
	XUndoManagerSupplier \
	XVbaMethodParameter \
	XViewDataSupplier \
	XXMLBasicExporter \
	XXMLOasisBasicImporter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/drawing,\
	Alignment \
	Arrangement \
	BezierPoint \
	BitmapMode \
	BoundVolume \
	CameraGeometry \
	CanvasFeature \
	CaptionEscapeDirection \
	CaptionType \
	CircleKind \
	ColorMode \
	ConnectionType \
	ConnectorType \
	CoordinateSequence \
	CoordinateSequenceSequence \
	DashStyle \
	Direction3D \
	DoubleSequence \
	DoubleSequenceSequence \
	DrawViewMode \
	EnhancedCustomShapeAdjustmentValue \
	EnhancedCustomShapeGluePointType \
	EnhancedCustomShapeParameter \
	EnhancedCustomShapeParameterPair \
	EnhancedCustomShapeParameterType \
	EnhancedCustomShapeSegment \
	EnhancedCustomShapeSegmentCommand \
	EnhancedCustomShapeTextFrame \
	EnhancedCustomShapeTextPathMode \
	EscapeDirection \
	FillStyle \
	FlagSequence \
	FlagSequenceSequence \
	GluePoint \
	GluePoint2 \
	GraphicFilterRequest \
	Hatch \
	HatchStyle \
	HomogenMatrix \
	HomogenMatrix3 \
	HomogenMatrix4 \
	HomogenMatrixLine \
	HomogenMatrixLine3 \
	HomogenMatrixLine4 \
	HorizontalDimensioning \
	LayerType \
	LineCap \
	LineDash \
	LineEndType \
	LineJoint \
	LineStyle \
	MeasureKind \
	MeasureTextHorzPos \
	MeasureTextVertPos \
	MirrorAxis \
	NormalsKind \
	PointSequence \
	PointSequenceSequence \
	PolyPolygonBezierCoords \
	PolyPolygonShape3D \
	PolygonFlags \
	PolygonKind \
	Position3D \
	ProjectionMode \
	RectanglePoint \
	ShadeMode \
	ShadingPattern \
	SnapObjectType \
	TextAdjust \
	TextAnimationDirection \
	TextAnimationKind \
	TextFitToSizeType \
	TextHorizontalAdjust \
	TextVerticalAdjust \
	TextureKind \
	TextureKind2 \
	TextureMode \
	TextureProjectionMode \
	VerticalDimensioning \
	XConnectableShape \
	XConnectorShape \
	XControlShape \
	XCustomShapeEngine \
	XCustomShapeHandle \
	XDrawPage \
	XDrawPageDuplicator \
	XDrawPageExpander \
	XDrawPageSummarizer \
	XDrawPageSupplier \
	XDrawPages \
	XDrawPagesSupplier \
	XDrawSubController \
	XDrawView \
	XEnhancedCustomShapeDefaulter \
	XGluePointsSupplier \
	XGraphicExportFilter \
	XLayer \
	XLayerManager \
	XLayerSupplier \
	XMasterPageTarget \
	XMasterPagesSupplier \
	XPresenterHelper \
	XSelectionFunction \
	XShape \
	XShapeAligner \
	XShapeArranger \
	XShapeBinder \
	XShapeCombiner \
	XShapeDescriptor \
	XShapeGroup \
	XShapeGrouper \
	XShapeMirror \
	XShapes \
	XSlidePreviewCache \
	XSlidePreviewCacheListener \
	XSlideRenderer \
	XSlideSorterBase \
	XUniversalShapeDescriptor \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/drawing/framework,\
	AnchorBindingMode \
	BorderType \
	ConfigurationChangeEvent \
	ResourceActivationMode \
	TabBarButton \
	XConfiguration \
	XConfigurationChangeListener \
	XConfigurationChangeRequest \
	XConfigurationController \
	XConfigurationControllerBroadcaster \
	XConfigurationControllerRequestQueue \
	XControllerManager \
	XModuleController \
	XPane \
	XPane2 \
	XPaneBorderPainter \
	XRelocatableResource \
	XResource \
	XResourceFactory \
	XResourceFactoryManager \
	XResourceId \
	XTabBar \
	XToolBar \
	XView \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/embed,\
	Actions \
	Aspects \
	ElementModes \
	EmbedMapUnits \
	EmbedMisc \
	EmbedStates \
	EmbedUpdateModes \
	EmbedVerbs \
	EntryInitModes \
	InsertedObjectInfo \
	InvalidStorageException \
	LinkageMisuseException \
	NeedsRunningStateException \
	NoVisualAreaSizeException \
	ObjectSaveVetoException \
	StateChangeInProgressException \
	StorageFormats \
	StorageWrappedTargetException \
	UnreachableStateException \
	UseBackupException \
	VerbAttributes \
	VerbDescriptor \
	VisualRepresentation \
	WrongStateException \
	XActionsApproval \
	XClassifiedObject \
	XCommonEmbedPersist \
	XComponentSupplier \
	XEmbedObjectClipboardCreator \
	XEmbedObjectCreator \
    XEmbeddedObjectCreator \
	XEmbedObjectFactory \
	XEmbedPersist \
	XEmbeddedClient \
	XEmbeddedObject \
	XEncryptionProtectedSource \
	XEncryptionProtectedSource2 \
	XEncryptionProtectedStorage \
	XExtendedStorageStream \
	XHatchWindow \
	XHatchWindowController \
	XHatchWindowFactory \
	XHierarchicalStorageAccess \
	XHierarchicalStorageAccess2 \
	XInplaceClient \
	XInplaceObject \
	XInsertObjectDialog \
	XLinkCreator \
	XLinkFactory \
	XLinkageSupport \
	XOLESimpleStorage \
	XOptimizedStorage \
	XPackageStructureCreator \
	XPersistanceHolder \
	XRelationshipAccess \
	XStateChangeBroadcaster \
	XStateChangeListener \
	XStorage \
	XStorage2 \
	XStorageRawAccess \
	XTransactedObject \
	XTransactionBroadcaster \
	XTransactionListener \
	XTransferableSupplier \
	XVisualObject \
	XWindowSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/form,\
	DataSelectionType \
	DatabaseDeleteEvent \
	DatabaseParameterEvent \
	ErrorEvent \
	FormButtonType \
	FormComponentType \
	FormSubmitEncoding \
	FormSubmitMethod \
	ListSourceType \
	NavigationBarMode \
	TabulatorCycle \
	XApproveActionBroadcaster \
	XApproveActionListener \
	XBoundComponent \
	XBoundControl \
	XChangeBroadcaster \
	XChangeListener \
	XConfirmDeleteBroadcaster \
	XConfirmDeleteListener \
	XDatabaseParameterBroadcaster \
	XDatabaseParameterBroadcaster2 \
	XDatabaseParameterListener \
	XDeleteListener \
	XErrorBroadcaster \
	XErrorListener \
	XForm \
	XForms \
	XFormComponent \
	XFormController \
	XFormControllerListener \
	XFormsSupplier \
	XFormsSupplier2 \
	XGrid \
	XGridColumnFactory \
	XGridControl \
	XGridControlListener \
	XGridFieldDataSupplier \
	XGridPeer \
	XImageProducerSupplier \
	XInsertListener \
	XLoadListener \
	XLoadable \
	XPositioningListener \
	XReset \
	XResetListener \
	XRestoreListener \
	XSubmit \
	XSubmitListener \
	XUpdateBroadcaster \
	XUpdateListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/form/binding,\
	IncompatibleTypesException \
	InvalidBindingStateException \
	ListEntryEvent \
	XBindableValue \
	XListEntryListener \
	XListEntrySink \
	XListEntrySource \
	XValueBinding \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/form/runtime,\
	FeatureState \
	FilterEvent \
	FormFeature \
	XFeatureInvalidation \
	XFilterController \
	XFilterControllerListener \
	XFormController \
	XFormControllerContext \
	XFormOperations \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/form/submission,\
	XSubmission \
	XSubmissionSupplier \
	XSubmissionVetoListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/form/validation,\
	XFormComponentValidityListener \
	XValidatable \
	XValidatableFormComponent \
	XValidator \
	XValidityConstraintListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/formula,\
	SymbolDescriptor \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/frame,\
	BorderWidths \
	CommandGroup \
	ControlCommand \
	ControlEvent \
	DispatchDescriptor \
	DispatchInformation \
	DispatchResultEvent \
	DispatchResultState \
	DispatchStatement \
	DoubleInitializationException \
	FeatureStateEvent \
	FrameAction \
	FrameActionEvent \
	FrameSearchFlag \
	IllegalArgumentIOException \
	LayoutManagerEvents \
	TerminationVetoException \
	TitleChangedEvent \
	UnknownModuleException \
	UntitledNumbersConst \
	WindowArrange \
	XAppDispatchProvider \
	XBorderResizeListener \
	XBrowseHistoryRegistry \
	XComponentLoader \
	XComponentRegistry \
	XConfigManager \
	XControlNotificationListener \
	XController \
	XController2 \
	XControllerBorder \
	XDesktop \
	XDesktop2 \
	XDesktopTask \
	XDispatch \
	XDispatchHelper \
	XDispatchInformationProvider \
	XDispatchProvider \
	XDispatchProviderInterception \
	XDispatchProviderInterceptor \
	XDispatchRecorder \
	XDispatchRecorderSupplier \
	XDispatchResultListener \
	XDocumentTemplates \
	XExtendedFilterDetection \
	XFilterDetect \
	XFrame \
	XFrame2 \
	XFrameActionListener \
	XFrameLoader \
	XFrameLoaderQuery \
	XFrameSetModel \
	XFrames \
	XFramesSupplier \
	XGlobalEventBroadcaster \
	XInterceptorInfo \
	XLayoutManager \
	XLayoutManager2 \
	XLayoutManagerEventBroadcaster \
	XLayoutManagerListener \
	XLoadEventListener \
	XLoadable \
	XLoaderFactory \
	XMenuBarAcceptor \
	XMenuBarMergingAcceptor \
	XModel \
	XModel2 \
	XModule \
	XModuleManager \
	XModuleManager2 \
	XNotifyingDispatch \
	XPopupMenuController \
	XRecordableDispatch \
	XSessionManagerClient \
	XSessionManagerListener \
	XSessionManagerListener2 \
	XStatusListener \
	XStatusbarController \
	XStorable \
	XStorable2 \
	XSubToolbarController \
	XSynchronousDispatch \
	XSynchronousFrameLoader \
	XTask \
	XTasksSupplier \
	XTerminateListener \
	XTerminateListener2 \
	XTitle \
	XTitleChangeBroadcaster \
	XTitleChangeListener \
	XToolbarController \
	XToolbarControllerListener \
	XTransientDocumentsDocumentContentFactory \
	XUIControllerFactory \
	XUIControllerRegistration \
	XUntitledNumbers \
	XUrlList \
	XWindowArranger \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/frame/status,\
	ClipboardFormats \
	FontHeight \
	ItemState \
	ItemStatus \
	LeftRightMargin \
	Template \
	UpperLowerMargin \
	UpperLowerMarginScale \
	Verb \
	Visibility \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/gallery,\
	GalleryItemType \
	XGalleryItem \
	XGalleryTheme \
	XGalleryThemeProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/geometry,\
	AffineMatrix2D \
	AffineMatrix3D \
	EllipticalArc \
	IntegerBezierSegment2D \
	IntegerPoint2D \
	IntegerRectangle2D \
	IntegerSize2D \
	Matrix2D \
	RealBezierSegment2D \
	RealPoint2D \
	RealRectangle2D \
	RealRectangle3D \
	RealSize2D \
	XMapping2D \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/graphic,\
	GraphicColorMode \
	GraphicType \
	PrimitiveFactory2D \
	XGraphic \
	XGraphicObject \
	XGraphicProvider \
	XGraphicRasterizer \
	XGraphicRenderer \
	XGraphicTransformer \
	XPrimitive2D \
	XPrimitive2DRenderer \
	XPrimitive3D \
	XPrimitiveFactory2D \
	XSvgParser \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/i18n,\
	AmPmValue \
	Boundary \
	BreakType \
	CTLScriptType \
	Calendar \
	Calendar2 \
	CalendarDisplayCode \
	CalendarDisplayIndex \
	CalendarFieldIndex \
	CalendarItem \
	CalendarItem2 \
	CharType \
	CharacterIteratorMode \
	CollatorOptions \
	Currency \
	Currency2 \
	DirectionProperty \
	ForbiddenCharacters \
	FormatElement \
	Implementation \
	InputSequenceCheckMode \
	KCharacterType \
	KNumberFormatType \
	KNumberFormatUsage \
	KParseTokens \
	KParseType \
	LanguageCountryInfo \
	LineBreakHyphenationOptions \
	LineBreakResults \
	LineBreakUserOptions \
	LocaleDataItem \
	LocaleItem \
	Months \
	MultipleCharsOutputException \
	NativeNumberMode \
	NativeNumberXmlAttributes \
	NumberFormatCode \
	NumberFormatIndex \
	ParseResult \
	ScriptDirection \
	ScriptType \
	TextConversionOption \
	TextConversionResult \
	TextConversionType \
	TransliterationModules \
	TransliterationModulesExtra \
	TransliterationModulesNew \
	TransliterationType \
	UnicodeScript \
	UnicodeType \
	Weekdays \
	WordType \
	XBreakIterator \
	XCalendar \
	XCalendar3 \
	XCharacterClassification \
	XCollator \
	XExtendedCalendar \
	XExtendedIndexEntrySupplier \
	XExtendedInputSequenceChecker \
	XExtendedTextConversion \
	XExtendedTransliteration \
	XForbiddenCharacters \
	XIndexEntrySupplier \
	XInputSequenceChecker \
	XLocaleData \
	XLocaleData2 \
	XLocaleData3 \
	XLocaleData4 \
	XNativeNumberSupplier \
	XNumberFormatCode \
	XOrdinalSuffix \
	XScriptTypeDetector \
	XTextConversion \
	XTransliteration \
	reservedWords \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/inspection,\
	InteractiveSelectionResult \
	LineDescriptor \
	PropertyCategoryDescriptor \
	PropertyControlType \
	PropertyLineElement \
	XHyperlinkControl \
	XNumericControl \
	XObjectInspector \
	XObjectInspectorModel \
	XObjectInspectorUI \
	XPropertyControl \
	XPropertyControlContext \
	XPropertyControlFactory \
	XPropertyControlObserver \
	XPropertyHandler \
	XStringListControl \
	XStringRepresentation \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/ldap,\
	LdapConnectionException \
	LdapGenericException \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/linguistic2,\
	ConversionDictionaryType \
	ConversionDirection \
	ConversionPropertyType \
	DictionaryEvent \
	DictionaryEventFlags \
	DictionaryListEvent \
	DictionaryListEventFlags \
	DictionaryType \
	LinguServiceEvent \
	LinguServiceEventFlags \
	ProofreadingResult \
	SingleProofreadingError \
	SpellFailure \
	XAvailableLocales \
	XConversionDictionary \
	XConversionDictionaryList \
	XConversionPropertyType \
	XDictionary \
	XDictionary1 \
	XDictionaryEntry \
	XDictionaryEventListener \
	XDictionaryList \
	XDictionaryListEventListener \
	XHyphenatedWord \
	XHyphenator \
	XLanguageGuessing \
	XLinguProperties \
	XLinguServiceEventBroadcaster \
	XLinguServiceEventListener \
	XLinguServiceManager \
	XLinguServiceManager2 \
	XMeaning \
	XPossibleHyphens \
	XProofreader \
	XProofreadingIterator \
	XSearchableDictionary \
	XSearchableDictionaryList \
	XSetSpellAlternatives \
	XSpellAlternatives \
	XSpellChecker \
	XSpellChecker1 \
	XSupportedLanguages \
	XSupportedLocales \
	XThesaurus \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/logging,\
	LogLevel \
	LogRecord \
	XConsoleHandler \
	XCsvLogFormatter \
	XLogFormatter \
	XLogHandler \
	XLogger \
	XLoggerPool \
	XSimpleLogRing \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/mail,\
	MailAttachment \
	MailException \
	MailServiceType \
	NoMailServiceProviderException \
	NoMailTransportProviderException \
	SendMailMessageFailedException \
	XAuthenticator \
	XConnectionListener \
	XMailMessage \
	XMailService \
	XMailServiceProvider \
	XSmtpService \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/media,\
	XFrameGrabber \
	XManager \
	XPlayer \
	XPlayerWindow \
	ZoomLevel \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/mozilla,\
	MenuMultipleChange \
	MenuSingleChange \
	MozillaProductType \
	XCloseSessionListener \
	XCodeProxy \
	XMenuProxy \
	XMenuProxyListener \
	XMozillaBootstrap \
	XPluginInstance \
	XPluginInstanceNotifySink \
	XPluginInstancePeer \
	XPluginInstanceSyncPeer \
	XPluginWindowPeer \
	XProfileDiscover \
	XProfileManager \
	XProxyRunner \
	XRemoteServiceManagerProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/office,\
	XAnnotation \
	XAnnotationAccess \
	XAnnotationEnumeration \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/packages,\
	EncryptionNotAllowedException \
	NoEncryptionException \
	NoRawFormatException \
	WrongPasswordException \
	XDataSinkEncrSupport \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/packages/manifest,\
	XManifestReader \
	XManifestWriter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/packages/zip,\
	XZipFileAccess \
	XZipFileAccess2 \
	ZipConstants \
	ZipEntry \
	ZipException \
	ZipIOException \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/plugin,\
	PluginDescription \
	PluginException \
	PluginMode \
	PluginVariable \
	XPlugin \
	XPluginContext \
	XPluginManager \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/presentation,\
	AnimationEffect \
	AnimationSpeed \
	ClickAction \
	EffectCommands \
	EffectNodeType \
	EffectPresetClass \
	FadeEffect \
	ParagraphTarget \
	PresentationRange \
	ShapeAnimationSubType \
	TextAnimationType \
	XCustomPresentationSupplier \
	XHandoutMasterSupplier \
	XPresentation \
	XPresentation2 \
	XPresentationPage \
	XPresentationSupplier \
	XShapeEventListener \
	XSlideShow \
	XSlideShowController \
	XSlideShowListener \
	XSlideShowView \
	XTransition \
	XTransitionFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/qa,\
	XDumper \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/rdf,\
	FileFormat \
	ParseException \
	QueryException \
	RepositoryException \
	Statement \
	URIs \
	XBlankNode \
	XDocumentMetadataAccess \
	XDocumentRepository \
	XLiteral \
	XMetadatable \
	XNamedGraph \
	XNode \
	XQuerySelectResult \
	XReifiedStatement \
	XRepository \
	XRepositorySupplier \
	XResource \
	XURI \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/rendering,\
	ARGBColor \
	AnimationAttributes \
	AnimationRepeat \
	BlendMode \
	Caret \
	Color \
	ColorComponent \
	ColorComponentTag \
	ColorProfile \
	ColorSpaceType \
	CompositeOperation \
	EmphasisMark \
	FillRule \
	FloatingPointBitmapFormat \
	FloatingPointBitmapLayout \
	FontInfo \
	FontMetrics \
	FontRequest \
	IntegerBitmapLayout \
	InterpolationMode \
	Panose \
	PanoseArmStyle \
	PanoseContrast \
	PanoseFamilyTypes \
	PanoseLetterForm \
	PanoseMidline \
	PanoseProportion \
	PanoseSerifStyle \
	PanoseStrokeVariation \
	PanoseWeight \
	PanoseXHeight \
	PathCapType \
	PathJoinType \
	RGBColor \
	RenderState \
	RenderingIntent \
	RepaintResult \
	StringContext \
	StrokeAttributes \
	TextDirection \
	TextHit \
	Texture \
	TexturingMode \
	ViewState \
	VolatileContentDestroyedException \
	XAnimatedSprite \
	XAnimation \
	XBezierPolyPolygon2D \
	XBitmap \
	XBitmapCanvas \
	XBitmapPalette \
	XBufferController \
	XCachedPrimitive \
	XCanvas \
	XCanvasFont \
	XColorSpace \
	XCustomSprite \
	XGraphicDevice \
	XHalfFloatBitmap \
	XHalfFloatReadOnlyBitmap \
	XIeeeDoubleBitmap \
	XIeeeDoubleReadOnlyBitmap \
	XIeeeFloatBitmap \
	XIeeeFloatReadOnlyBitmap \
	XIntegerBitmap \
	XIntegerBitmapColorSpace \
	XIntegerReadOnlyBitmap \
	XLinePolyPolygon2D \
	XMtfRenderer \
	XParametricPolyPolygon2D \
	XPolyPolygon2D \
	XSimpleCanvas \
	XSprite \
	XSpriteCanvas \
	XTextLayout \
	XVolatileBitmap \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/report,\
	Calculation \
	ForceNewPage \
	GroupKeepTogether \
	GroupOn \
	KeepTogether \
	ReportPrintOption \
	SectionPageBreak \
	XFixedLine \
	XFixedText \
	XFormatCondition \
	XFormattedField \
	XFunction \
	XFunctions \
	XFunctionsSupplier \
	XGroup \
	XGroups \
	XImageControl \
	XReportComponent \
	XReportControlFormat \
	XReportControlModel \
	XReportDefinition \
	XReportEngine \
	XSection \
	XShape \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/report/meta,\
	XFormulaParser \
	XFunctionCategory \
	XFunctionDescription \
	XFunctionManager \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/resource,\
	MissingResourceException \
	XLocale \
	XResourceBundle \
	XResourceBundleLoader \
	XStringResourceManager \
	XStringResourcePersistence \
	XStringResourceResolver \
	XStringResourceSupplier \
	XStringResourceWithLocation \
	XStringResourceWithStorage \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/scanner,\
	ScanError \
	ScannerContext \
	ScannerException \
	XScannerManager \
	XScannerManager2 \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/script,\
	LibraryNotLoadedException \
	ModuleInfo \
	ModuleSizeExceededRequest \
	ModuleType \
	NativeObjectWrapper \
	XLibraryContainer \
	XLibraryContainer2 \
	XLibraryContainer3 \
	XLibraryContainerExport \
	XLibraryContainerPassword \
	XLibraryQueryExecutable \
	XPersistentLibraryContainer \
	XStorageBasedLibraryContainer \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/script/browse,\
	BrowseNodeFactoryViewTypes \
	BrowseNodeTypes \
	XBrowseNode \
	XBrowseNodeFactory \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/script/provider,\
	ScriptErrorRaisedException \
	ScriptExceptionRaisedException \
	ScriptFrameworkErrorException \
	ScriptFrameworkErrorType \
	XScript \
	XScriptContext \
	XScriptProvider \
	XScriptProviderFactory \
	XScriptProviderSupplier \
	XScriptURIHelper \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/script/vba,\
	VBAEventId \
	VBAScriptEvent \
	VBAScriptEventId \
	XVBACompatibility \
	XVBAEventProcessor \
	XVBAMacroResolver \
	XVBAModuleInfo \
	XVBAScriptListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/sdb,\
	BooleanComparisonMode \
	CommandType \
	DatabaseRegistrationEvent \
	DocumentSaveRequest \
	ErrorCondition \
	ParametersRequest \
	RowChangeAction \
	RowChangeEvent \
	RowSetVetoException \
	RowsChangeEvent \
	SQLContext \
	SQLErrorEvent \
	SQLFilterOperator \
	XAlterQuery \
	XBookmarksSupplier \
	XColumn \
	XColumnUpdate \
	XCommandPreparation \
	XCompletedConnection \
	XCompletedExecution \
	XDataAccessDescriptorFactory \
	XDatabaseAccess \
	XDatabaseAccessListener \
	XDatabaseContext \
	XDatabaseEnvironment \
	XDatabaseRegistrations \
	XDatabaseRegistrationsListener \
	XDocumentDataSource \
	XFormDocumentsSupplier \
	XInteractionDocumentSave \
	XInteractionSupplyParameters \
	XOfficeDatabaseDocument \
	XParametersSupplier \
	XQueriesSupplier \
	XQueryDefinition \
	XQueryDefinitionsSupplier \
	XReportDocumentsSupplier \
	XResultSetAccess \
	XRowSetApproveBroadcaster \
	XRowSetApproveListener \
	XRowSetChangeBroadcaster \
	XRowSetChangeListener \
	XRowSetSupplier \
	XRowsChangeBroadcaster \
	XRowsChangeListener \
	XSQLErrorBroadcaster \
	XSQLErrorListener \
	XSQLQueryComposer \
	XSQLQueryComposerFactory \
	XSingleSelectQueryAnalyzer \
	XSingleSelectQueryComposer \
	XSubDocument \
	XTextConnectionSettings \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/sdb/application,\
	CopyTableContinuation \
	CopyTableOperation \
	CopyTableRowEvent \
	DatabaseObject \
	DatabaseObjectContainer \
	NamedDatabaseObject \
	XCopyTableListener \
	XCopyTableWizard \
	XDatabaseDocumentUI \
	XTableUIProvider \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/sdb/tools,\
	CompositionType \
	XConnectionSupplier \
	XConnectionTools \
	XDataSourceMetaData \
	XIndexAlteration \
	XKeyAlteration \
	XObjectNames \
	XTableAlteration \
	XTableName \
	XTableRename \
	XViewAccess \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/sdbc,\
	BatchUpdateException \
	BestRowScope \
	BestRowType \
	ChangeAction \
	ChangeEvent \
	ColumnSearch \
	ColumnType \
	ColumnValue \
	DataTruncation \
	DataType \
	Deferrability \
	DriverPropertyInfo \
	FetchDirection \
	IndexType \
	KeyRule \
	ProcedureColumn \
	ProcedureResult \
	ResultSetConcurrency \
	ResultSetType \
	SQLException \
	SQLWarning \
	TransactionIsolation \
	XArray \
	XBatchExecution \
	XBlob \
	XClob \
	XCloseable \
	XColumnLocate \
	XConnection \
	XConnectionPool \
	XDataSource \
	XDatabaseMetaData \
	XDatabaseMetaData2 \
	XDriver \
	XDriverAccess \
	XDriverManager \
	XDriverManager2 \
	XGeneratedResultSet \
	XIsolatedConnection \
	XMultipleResults \
	XOutParameters \
	XParameters \
	XPooledConnection \
	XPreparedBatchExecution \
	XPreparedStatement \
	XRef \
	XResultSet \
	XResultSetMetaData \
	XResultSetMetaDataSupplier \
	XResultSetUpdate \
	XRow \
	XRowSet \
	XRowSetListener \
	XRowUpdate \
	XSQLData \
	XSQLInput \
	XSQLOutput \
	XStatement \
	XStruct \
	XWarningsSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/sdbcx,\
	CheckOption \
	CompareBookmark \
	KeyType \
	Privilege \
	PrivilegeObject \
	XAlterTable \
	XAlterView \
	XAppend \
	XAuthorizable \
	XColumnsSupplier \
	XCreateCatalog \
	XDataDefinitionSupplier \
	XDataDescriptorFactory \
	XDeleteRows \
	XDrop \
	XDropCatalog \
	XGroupsSupplier \
	XIndexesSupplier \
	XKeysSupplier \
	XRename \
	XRowLocate \
	XTablesSupplier \
	XUser \
	XUsersSupplier \
	XViewsSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/security,\
	CertAltNameEntry \
	CertificateCharacters \
	CertificateContainerStatus \
	CertificateException \
	CertificateValidity \
	CryptographyException \
	DocumentSignatureInformation \
	EncryptionException \
	ExtAltNameType \
	KeyException \
	KeyUsage \
	NoPasswordException \
	SecurityInfrastructureException \
	SignatureException \
	XCertificate \
	XCertificateContainer \
	XCertificateExtension \
	XDocumentDigitalSignatures \
	XSanExtension \
	XSerialNumberAdapter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/sheet,\
	ActivationEvent \
	AddressConvention \
	Border \
	CellDeleteMode \
	CellFlags \
	CellInsertMode \
	ComplexReference \
	ConditionOperator \
	ConditionOperator2 \
	DDEItemInfo \
	DDELinkInfo \
	DDELinkMode \
	DataImportMode \
	DataPilotFieldAutoShowInfo \
	DataPilotFieldFilter \
	DataPilotFieldGroupBy \
	DataPilotFieldGroupInfo \
	DataPilotFieldLayoutInfo \
	DataPilotFieldLayoutMode \
	DataPilotFieldOrientation \
	DataPilotFieldReference \
	DataPilotFieldReferenceItemType \
	DataPilotFieldReferenceType \
	DataPilotFieldShowItemsMode \
	DataPilotFieldSortInfo \
	DataPilotFieldSortMode \
	DataPilotOutputRangeType \
	DataPilotTableHeaderData \
	DataPilotTablePositionData \
	DataPilotTablePositionType \
	DataPilotTableResultData \
	DataResult \
	DataResultFlags \
	DimensionFlags \
	ExternalLinkInfo \
	ExternalLinkType \
	ExternalReference \
	FillDateMode \
	FillDirection \
	FillMode \
	FilterConnection \
	FilterFieldValue \
	FilterOperator \
	FilterOperator2 \
	FormulaLanguage \
	FormulaMapGroup \
	FormulaMapGroupSpecialOffset \
	FormulaOpCodeMapEntry \
	FormulaResult \
	FormulaToken \
	FunctionArgument \
	FunctionCategory \
	GeneralFunction \
	GoalResult \
	LocalizedName \
	MemberResult \
	MemberResultFlags \
	MoveDirection \
	NamedRangeFlag \
	NameToken \
	NoConvergenceException \
	PasteOperation \
	RangeSelectionEvent \
	ReferenceFlags \
	ResultEvent \
	SheetLinkMode \
	SingleReference \
	SolverConstraint \
	SolverConstraintOperator \
	SpreadsheetViewObjectsMode \
	StatusBarFunction \
	SubTotalColumn \
	TableFilterField \
	TableFilterField2 \
	TableFilterField3 \
	TableOperationMode \
	TablePageBreakData \
	TableValidationVisibility \
	ValidationAlertStyle \
	ValidationType \
	XActivationBroadcaster \
	XActivationEventListener \
	XAddIn \
	XAreaLink \
	XAreaLinks \
	XArrayFormulaRange \
	XArrayFormulaTokens \
	XCalculatable \
	XCellAddressable \
	XCellFormatRangesSupplier \
	XCellRangeAddressable \
	XCellRangeData \
	XCellRangeFormula \
	XCellRangeMovement \
	XCellRangeReferrer \
	XCellRangesAccess \
	XCellRangesQuery \
	XCellSeries \
	XCompatibilityNames \
	XConsolidatable \
	XConsolidationDescriptor \
	XDDELink \
	XDDELinkResults \
	XDDELinks \
	XDataPilotDataLayoutFieldSupplier \
	XDataPilotDescriptor \
	XDataPilotField \
	XDataPilotFieldGrouping \
	XDataPilotMemberResults \
	XDataPilotResults \
	XDataPilotTable \
	XDataPilotTable2 \
	XDataPilotTables \
	XDataPilotTablesSupplier \
	XDatabaseRange \
	XDatabaseRanges \
	XDimensionsSupplier \
	XDocumentAuditing \
	XDrillDownDataSupplier \
	XEnhancedMouseClickBroadcaster \
	XExternalDocLink \
	XExternalDocLinks \
	XExternalSheetCache \
	XExternalSheetName \
	XFillAcrossSheet \
	XFilterFormulaParser \
	XFormulaOpCodeMapper \
	XFormulaParser \
	XFormulaQuery \
	XFormulaTokens \
	XFunctionAccess \
	XFunctionDescriptions \
	XGlobalSheetSettings \
	XGoalSeek \
	XHeaderFooterContent \
	XHierarchiesSupplier \
	XLabelRange \
	XLabelRanges \
	XLevelsSupplier \
	XMembersSupplier \
	XMultiFormulaTokens \
	XMultipleOperation \
	XNamedRange \
	XNamedRanges \
	XPrintAreas \
	XRangeSelection \
	XRangeSelectionChangeListener \
	XRangeSelectionListener \
	XRecentFunctions \
	XResultListener \
	XScenario \
	XScenarioEnhanced \
	XScenarios \
	XScenariosSupplier \
	XSelectedSheetsSupplier \
	XSheetAnnotation \
	XSheetAnnotationAnchor \
	XSheetAnnotationShapeSupplier \
	XSheetAnnotations \
	XSheetAnnotationsSupplier \
	XSheetAuditing \
	XSheetCellCursor \
	XSheetCellRange \
	XSheetCellRangeContainer \
	XSheetCellRanges \
	XSheetCondition \
	XSheetCondition2 \
	XSheetConditionalEntries \
	XSheetConditionalEntry \
	XSheetFilterDescriptor \
	XSheetFilterDescriptor2 \
	XSheetFilterDescriptor3 \
	XSheetFilterable \
	XSheetFilterableEx \
	XSheetLinkable \
	XSheetOperation \
	XSheetOutline \
	XSheetPageBreak \
	XSheetPastable \
	XSolver \
	XSolverDescription \
	XSpreadsheet \
	XSpreadsheetDocument \
	XSpreadsheetView \
	XSpreadsheets \
	XSpreadsheets2 \
	XSubTotalCalculatable \
	XSubTotalDescriptor \
	XSubTotalField \
	XUniqueCellFormatRangesSupplier \
	XUnnamedDatabaseRanges \
	XUsedAreaCursor \
	XViewFreezable \
	XViewPane \
	XViewPanesSupplier \
	XViewSplitable \
	XVolatileResult \
	_NamedRange \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/smarttags,\
	SmartTagRecognizerMode \
	XRangeBasedSmartTagRecognizer \
	XSmartTagAction \
	XSmartTagRecognizer \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/style,\
	BreakType \
	CaseMap \
	DropCapFormat \
	FootnoteLineStyle \
	GraphicLocation \
	HorizontalAlignment \
	LineNumberPosition \
	LineSpacing \
	LineSpacingMode \
	NumberingType \
	PageStyleLayout \
	ParagraphAdjust \
	ParagraphStyleCategory \
	TabAlign \
	TabStop \
	VerticalAlignment \
	XAutoStyle \
	XAutoStyleFamily \
	XAutoStyles \
	XAutoStylesSupplier \
	XDefaultsSupplier \
	XStyle \
	XStyleFamiliesSupplier \
	XStyleLoader \
	XStyleSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/svg,\
	XSVGPrinter \
	XSVGWriter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/system,\
	SimpleMailClientFlags \
	SystemShellExecuteException \
	SystemShellExecuteFlags \
	XSimpleMailClient \
	XSimpleMailClientSupplier \
	XSimpleMailMessage \
	XSimpleMailMessage2 \
	XSystemShellExecute \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/table,\
	BorderLine \
	BorderLine2 \
	BorderLineStyle \
	CellAddress \
	CellContentType \
	CellHoriJustify \
	CellJustifyMethod \
	CellOrientation \
	CellRangeAddress \
	CellVertJustify \
	CellVertJustify2 \
	ShadowFormat \
	ShadowLocation \
	TableBorder \
	TableBorder2 \
	TableBorderDistances \
	TableOrientation \
	TableSortField \
	TableSortFieldType \
	XAutoFormattable \
	XCell \
	XCell2 \
	XCellCursor \
	XCellRange \
	XColumnRowRange \
	XMergeableCell \
	XMergeableCellRange \
	XTable \
	XTableChart \
	XTableCharts \
	XTableChartsSupplier \
	XTableColumns \
	XTableRows \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/task,\
	ClassifiedInteractionRequest \
	DocumentMSPasswordRequest \
	DocumentMSPasswordRequest2 \
	DocumentMacroConfirmationRequest \
	DocumentPasswordRequest \
	DocumentPasswordRequest2 \
	ErrorCodeIOException \
	ErrorCodeRequest \
	FutureDocumentVersionProductUpdateRequest \
	InteractionClassification \
	MasterPasswordRequest \
	NoMasterException \
	PDFExportException \
	PasswordRequest \
	PasswordRequestMode \
	UnsupportedOverwriteRequest \
	UrlRecord \
	UserRecord \
	XAbortChannel \
	XAsyncJob \
	XInteractionApprove \
	XInteractionAskLater \
	XInteractionDisapprove \
	XInteractionPassword \
	XInteractionPassword2 \
	XInteractionRequestStringResolver \
	XJob \
	XJobExecutor \
	XJobListener \
	XMasterPasswordHandling \
	XMasterPasswordHandling2 \
	XPasswordContainer \
	XPasswordContainer2 \
	XRestartManager \
	XStatusIndicator \
	XStatusIndicatorFactory \
	XStatusIndicatorSupplier \
	XUrlContainer \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/text,\
	AuthorDisplayFormat \
	BibliographyDataField \
	BibliographyDataType \
	ChapterFormat \
	CharacterCompressionType \
	ColumnSeparatorStyle \
	ControlCharacter \
	DateDisplayFormat \
	DocumentStatistic \
	FilenameDisplayFormat \
	FontEmphasis \
	FontRelief \
	FootnoteNumbering \
	GraphicCrop \
	HoriOrientation \
	HoriOrientationFormat \
	HorizontalAdjust \
	InvalidTextContentException \
	LabelFollow \
	MailMergeEvent \
	MailMergeType \
	ModuleDispatcher \
	NotePrintMode \
	PageNumberType \
	ParagraphVertAlign \
	PlaceholderType \
	PositionAndSpaceMode \
	PositionLayoutDir \
	ReferenceFieldPart \
	ReferenceFieldSource \
	RelOrientation \
	RubyAdjust \
	SectionFileLink \
	SetVariableType \
	SizeType \
	TableColumnSeparator \
	TemplateDisplayFormat \
	TextColumn \
	TextColumnSequence \
	TextContentAnchorType \
	TextGridMode \
	TextMarkupDescriptor \
	TextMarkupType \
	TextPosition \
	TextRangeSelection \
	TimeDisplayFormat \
	UserDataPart \
	UserFieldFormat \
	VertOrientation \
	VertOrientationFormat \
	WrapInfluenceOnPosition \
	WrapTextMode \
	WritingMode \
	WritingMode2 \
	XAutoTextContainer \
	XAutoTextContainer2 \
	XAutoTextEntry \
	XAutoTextGroup \
	XBookmarkInsertTool \
	XBookmarksSupplier \
	XChapterNumberingSupplier \
	XDefaultNumberingProvider \
	XDependentTextField \
	XDocumentIndex \
	XDocumentIndexMark \
	XDocumentIndexesSupplier \
	XEndnotesSettingsSupplier \
	XEndnotesSupplier \
	XFlatParagraph \
	XFlatParagraphIterator \
	XFlatParagraphIteratorProvider \
	XFootnote \
	XFootnotesSettingsSupplier \
	XFootnotesSupplier \
	XFormField \
	XLineNumberingProperties \
	XMailMergeBroadcaster \
	XMailMergeListener \
	XMarkingAccess \
	XMultiTextMarkup \
	XNumberingFormatter \
	XNumberingRulesSupplier \
	XNumberingTypeInfo \
	XPageCursor \
	XPagePrintable \
	XParagraphAppend \
	XParagraphCursor \
	XRedline \
	XReferenceMarksSupplier \
	XRelativeTextContentInsert \
	XRelativeTextContentRemove \
	XRubySelection \
	XSentenceCursor \
	XSimpleText \
	XText \
	XTextAppend \
	XTextAppendAndConvert \
	XTextColumns \
	XTextContent \
	XTextContentAppend \
	XTextConvert \
	XTextCopy \
	XTextCursor \
	XTextDocument \
	XTextEmbeddedObjectsSupplier \
	XTextField \
	XTextFieldsSupplier \
	XTextFrame \
	XTextFramesSupplier \
	XTextGraphicObjectsSupplier \
	XTextMarkup \
	XTextPortionAppend \
	XTextRange \
	XTextRangeCompare \
	XTextRangeMover \
	XTextSection \
	XTextSectionsSupplier \
	XTextShapesSupplier \
	XTextTable \
	XTextTableCursor \
	XTextTablesSupplier \
	XTextViewCursor \
	XTextViewCursorSupplier \
	XWordCursor \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/ucb,\
	AlreadyInitializedException \
	AuthenticationRequest \
	CheckinArgument \
	CertificateValidationRequest \
	Command \
	CommandAbortedException \
	CommandFailedException \
	CommandInfo \
	CommandInfoChange \
	CommandInfoChangeEvent \
	ConnectionMode \
	ContentAction \
	ContentCreationError \
	ContentCreationException \
	ContentEvent \
	ContentInfo \
	ContentInfoAttribute \
	ContentProviderInfo \
	ContentResultSetCapability \
	CrossReference \
	DocumentHeaderField \
	DocumentStoreMode \
	DuplicateCommandIdentifierException \
	DuplicateProviderException \
	Error \
	ExportStreamInfo \
	FetchError \
	FetchResult \
	FileSystemNotation \
	FolderList \
	FolderListCommand \
	FolderListEntry \
	GlobalTransferCommandArgument \
	GlobalTransferCommandArgument2 \
	IOErrorCode \
	IllegalIdentifierException \
	InsertCommandArgument \
	InsertCommandArgument2 \
	InteractiveAppException \
	InteractiveAugmentedIOException \
	InteractiveBadTransferURLException \
	InteractiveFileIOException \
	InteractiveIOException \
	InteractiveLockingException \
	InteractiveLockingLockExpiredException \
	InteractiveLockingLockedException \
	InteractiveLockingNotLockedException \
	InteractiveNetworkConnectException \
	InteractiveNetworkException \
	InteractiveNetworkGeneralException \
	InteractiveNetworkOffLineException \
	InteractiveNetworkReadException \
	InteractiveNetworkResolveNameException \
	InteractiveNetworkWriteException \
	InteractiveWrongMediumException \
	Link \
	ListAction \
	ListActionType \
	ListEvent \
	ListenerAlreadySetException \
	Lock \
	LockDepth \
	LockEntry \
	LockScope \
	LockType \
	MissingInputStreamException \
	MissingPropertiesException \
	NameClash \
	NameClashException \
	NameClashResolveRequest \
	NumberedSortingInfo \
	OpenCommandArgument \
	OpenCommandArgument2 \
	OpenCommandArgument3 \
	OpenMode \
	OutgoingMessageState \
	PostCommandArgument \
	PostCommandArgument2 \
	Priority \
	PropertyCommandArgument \
	PropertyValueInfo \
	PropertyValueState \
	RecipientInfo \
	RememberAuthentication \
	RemoteContentProviderChangeAction \
	RemoteContentProviderChangeEvent \
	ResultSetException \
	Rule \
	RuleAction \
	RuleOperator \
	RuleSet \
	RuleTerm \
	SearchCommandArgument \
	SearchCriterium \
	SearchInfo \
	SearchRecursion \
	SendInfo \
	SendMediaTypes \
	ServiceNotFoundException \
	SortingInfo \
	SynchronizePolicy \
	TransferCommandOperation \
	TransferInfo \
	TransferInfo2 \
	TransferResult \
	URLAuthenticationRequest \
	UnsupportedCommandException \
	UnsupportedDataSinkException \
	UnsupportedNameClashException \
	UnsupportedOpenModeException \
	VerificationMode \
	WelcomeDynamicResultSetStruct \
	XAnyCompare \
	XAnyCompareFactory \
	XCachedContentResultSetFactory \
	XCachedContentResultSetStubFactory \
	XCachedDynamicResultSetFactory \
	XCachedDynamicResultSetStubFactory \
	XCommandEnvironment \
	XCommandInfo \
	XCommandInfoChangeListener \
	XCommandInfoChangeNotifier \
	XCommandProcessor \
	XCommandProcessor2 \
	XContent \
	XContentAccess \
	XContentCreator \
	XContentEventListener \
	XContentIdentifier \
	XContentIdentifierFactory \
	XContentIdentifierMapping \
	XContentProvider \
	XContentProviderFactory \
	XContentProviderManager \
	XContentProviderSupplier \
	XContentTransmitter \
	XDataContainer \
	XDynamicResultSet \
	XDynamicResultSetListener \
	XFetchProvider \
	XFetchProviderForContentAccess \
	XFileIdentifierConverter \
	XInteractionHandlerSupplier \
	XInteractionReplaceExistingData \
	XInteractionSupplyAuthentication \
	XInteractionSupplyAuthentication2 \
	XInteractionSupplyName \
	XParameterizedContentProvider \
	XPersistentPropertySet \
	XProgressHandler \
	XPropertyMatcher \
	XPropertyMatcherFactory \
	XPropertySetRegistry \
	XPropertySetRegistryFactory \
	XRecycler \
	XRemoteContentProviderAcceptor \
	XRemoteContentProviderActivator \
	XRemoteContentProviderChangeListener \
	XRemoteContentProviderChangeNotifier \
	XRemoteContentProviderConnectionControl \
	XRemoteContentProviderDistributor \
	XRemoteContentProviderDoneListener \
	XRemoteContentProviderSupplier \
	XSimpleFileAccess \
	XSimpleFileAccess2 \
	XSimpleFileAccess3 \
	XSortedDynamicResultSetFactory \
	XSourceInitialization \
	XUniversalContentBroker \
	XWebDAVCommandEnvironment \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/ui,\
	ActionTriggerSeparatorType \
	ConfigurationEvent \
	ContextChangeEventMultiplexer \
	ContextChangeEventObject \
	ContextMenuExecuteEvent \
	ContextMenuInterceptorAction \
	DockingArea \
	ImageManager \
	ImageType \
	ItemStyle \
	ItemType \
	LayoutSize \
	UIElementType \
	XAcceleratorConfiguration \
	XContextChangeEventListener \
	XContextChangeEventMultiplexer \
	XContextMenuInterception \
	XContextMenuInterceptor \
	XDockingAreaAcceptor \
	XImageManager \
	XModuleUIConfigurationManager \
	XModuleUIConfigurationManager2 \
	XModuleUIConfigurationManagerSupplier \
	XSidebar \
	XSidebarPanel \
	XStatusbarItem \
	XToolPanel \
	XUIConfiguration \
	XUIConfigurationListener \
	XUIConfigurationManager \
	XUIConfigurationManager2 \
	XUIConfigurationManagerSupplier \
	XUIConfigurationPersistence \
	XUIConfigurationStorage \
	XUIElement \
	XUIElementFactory \
    XUIElementFactoryManager \
	XUIElementFactoryRegistration \
	XUIElementSettings \
	XUIFunctionListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/ui/dialogs,\
	CommonFilePickerElementIds \
	ControlActions \
	DialogClosedEvent \
	ExecutableDialogException \
	ExecutableDialogResults \
	ExtendedFilePickerElementIds \
	FilePickerEvent \
	FilePreviewImageFormats \
	ListboxControlActions \
	TemplateDescription \
	WizardButton \
	WizardTravelType \
	XAsynchronousExecutableDialog \
	XControlAccess \
	XControlInformation \
	XDialogClosedListener \
	XExecutableDialog \
	XFilePicker \
	XFilePicker2 \
	XFilePicker3 \
	XFilePickerControlAccess \
	XFilePickerListener \
	XFilePickerNotifier \
	XFilePreview \
	XFilterGroupManager \
	XFilterManager \
	XFolderPicker \
	XFolderPicker2 \
	XWizard \
	XWizardController \
	XWizardPage \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/util,\
	AliasProgrammaticPair \
	AtomClassRequest \
	AtomDescription \
	CellProtection \
	ChangesEvent \
	ChangesSet \
	CloseVetoException \
	Color \
	DataEditorEvent \
	DataEditorEventType \
	Date \
	DateWithTimezone \
	DateTime \
	DateTimeWithTimezone \
	DateTimeRange \
	Duration \
	ElementChange \
	Endianness \
	InvalidStateException \
	Language \
	MalformedNumberFormatException \
	MeasureUnit \
	ModeChangeEvent \
	NotLockedException \
	NotNumericException \
	NumberFormat \
	RevisionTag \
	SearchAlgorithms \
	SearchFlags \
	SearchOptions \
	SearchResult \
	SortField \
	SortFieldType \
	Time \
	TimeWithTimezone \
	TriState \
	URL \
	VetoException \
	XAtomServer \
	XBroadcaster \
	XCancellable \
	XChainable \
	XChangesBatch \
	XChangesListener \
	XChangesNotifier \
	XChangesSet \
	XCloneable \
	XCloseBroadcaster \
	XCloseListener \
	XCloseable \
	XDataEditor \
	XDataEditorListener \
	XFlushListener \
	XFlushable \
	XImportable \
	XIndent \
	XJobManager \
	XLinkUpdate \
	XLocalizedAliases \
	XLockable \
	XMergeable \
	XModeChangeApproveListener \
	XModeChangeBroadcaster \
	XModeChangeListener \
	XModeSelector \
	XModifiable \
	XModifiable2 \
	XModifyBroadcaster \
	XModifyListener \
	XNumberFormatPreviewer \
	XNumberFormatTypes \
	XNumberFormats \
	XNumberFormatsSupplier \
	XNumberFormatter \
	XNumberFormatter2 \
	XOfficeInstallationDirectories \
	XPathSettings \
	XPropertyReplace \
	XProtectable \
	XRefreshListener \
	XRefreshable \
	XReplaceDescriptor \
	XReplaceable \
	XSearchDescriptor \
	XSearchable \
	XSortable \
	XStringAbbreviation \
	XStringEscape \
	XStringMapping \
	XStringSubstitution \
	XStringWidth \
	XTextSearch \
	XTimeStamped \
	XURLTransformer \
	XUniqueIDFactory \
	XUpdatable \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/view,\
	DocumentZoomType \
	DuplexMode \
	PaperFormat \
	PaperOrientation \
	PrintJobEvent \
	PrintableState \
	PrintableStateEvent \
	SelectionType \
	XControlAccess \
	XFormLayerAccess \
	XLineCursor \
	XMultiSelectionSupplier \
	XPrintJob \
	XPrintJobBroadcaster \
	XPrintJobListener \
	XPrintSettingsSupplier \
	XPrintable \
	XPrintableBroadcaster \
	XPrintableListener \
	XRenderable \
	XScreenCursor \
	XSelectionChangeListener \
	XSelectionSupplier \
	XViewCursor \
	XViewSettingsSupplier \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xforms,\
	InvalidDataOnSubmitException \
	XDataTypeRepository \
	XFormsEvent \
	XFormsSupplier \
	XFormsUIHelper1 \
	XModel \
	XModel2 \
	XSubmission \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml,\
	Attribute \
	AttributeData \
	FastAttribute \
	XExportFilter \
	XImportFilter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/crypto,\
	CipherID \
	DigestID \
	SecurityOperationStatus \
	XCipherContext \
	XCipherContextSupplier \
	XDigestContext \
	XDigestContextSupplier \
	XMLEncryptionException \
	XMLSignatureException \
	XNSSInitializer \
	XSEInitializer \
	XSecurityEnvironment \
	XUriBinding \
	XXMLEncryption \
	XXMLEncryptionTemplate \
	XXMLSecurityContext \
	XXMLSecurityTemplate \
	XXMLSignature \
	XXMLSignatureTemplate \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/crypto/sax,\
	ConstOfSecurityId \
	ElementMarkPriority \
	ElementMarkType \
	ElementStackItem \
	XBlockerMonitor \
	XDecryptionResultBroadcaster \
	XDecryptionResultListener \
	XElementStackKeeper \
	XEncryptionResultBroadcaster \
	XEncryptionResultListener \
	XKeyCollector \
	XMissionTaker \
	XReferenceCollector \
	XReferenceResolvedBroadcaster \
	XReferenceResolvedListener \
	XSAXEventKeeper \
	XSAXEventKeeperStatusChangeBroadcaster \
	XSAXEventKeeperStatusChangeListener \
	XSecurityController \
	XSecuritySAXEventKeeper \
	XSignatureCreationResultBroadcaster \
	XSignatureCreationResultListener \
	XSignatureVerifyResultBroadcaster \
	XSignatureVerifyResultListener \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/csax,\
	XCompressedDocumentHandler \
	XMLAttribute \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/dom,\
	DOMException \
	DOMExceptionType \
	NodeType \
	SAXDocumentBuilderState \
	XAttr \
	XCDATASection \
	XCharacterData \
	XComment \
	XDOMImplementation \
	XDocument \
	XDocumentBuilder \
	XDocumentFragment \
	XDocumentType \
	XElement \
	XEntity \
	XEntityReference \
	XNamedNodeMap \
	XNode \
	XNodeList \
	XNotation \
	XProcessingInstruction \
	XSAXDocumentBuilder \
	XSAXDocumentBuilder2 \
	XText \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/dom/events,\
	AttrChangeType \
	EventException \
	EventType \
	PhaseType \
	XDocumentEvent \
	XEvent \
	XEventListener \
	XEventTarget \
	XMouseEvent \
	XMutationEvent \
	XUIEvent \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/dom/views,\
	XAbstractView \
	XDocumentView \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/input,\
	XAttributes \
	XElement \
	XNamespaceMapping \
	XRoot \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/sax,\
	FastToken \
	InputSource \
	SAXException \
	SAXInvalidCharacterException \
	SAXParseException \
	XAttributeList \
	XDTDHandler \
	XDocumentHandler \
	XEntityResolver \
	XErrorHandler \
	XExtendedDocumentHandler \
	XFastAttributeList \
	XFastContextHandler \
	XFastDocumentHandler \
	XFastParser \
	XFastSAXSerializable \
	XFastShapeContextHandler \
	XFastTokenHandler \
	XLocator \
	XParser \
	XSAXSerializable \
	XWriter \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/wrapper,\
	XXMLDocumentWrapper \
	XXMLElementWrapper \
))
$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/xpath,\
	Libxml2ExtensionHandle \
	XPathException \
	XPathObjectType \
	XXPathAPI \
	XXPathExtension \
	XXPathObject \
))

$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xml/xslt,\
	XXSLTTransformer \
))

$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/com/sun/star/xsd,\
	DataTypeClass \
	WhiteSpaceTreatment \
	XDataType \
))

$(eval $(call gb_UnoApi_add_idlfiles,offapi,offapi/org/freedesktop/PackageKit,\
    XSyncDbusSessionHelper \
    XModify \
    XQuery \
))
$(eval $(call gb_UnoApi_add_idlfiles_nohdl,offapi,offapi/org/freedesktop/PackageKit,\
    SyncDbusSessionHelper \
))

$(eval $(call gb_UnoApi_set_reference_rdbfile,offapi,offapi/type_reference/offapi))

# vim: set noet sw=4 ts=4:
