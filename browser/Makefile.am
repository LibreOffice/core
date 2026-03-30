MAKEFLAGS = --no-builtin-rules
CTAGS = ctags
BUILT_SOURCES = node_modules

L10N_PO = $(if $(filter true,$(ENABLE_ANDROIDAPP)),$(filter-out %-ca-valencia.po,$(wildcard $(srcdir)/po/*.po)),$(wildcard $(srcdir)/po/*.po))
# See https://issuetracker.google.com/issues/293294923 for why we need to filter out ca-valencian
IS_BUNDLE = $(if $(filter true,$(ENABLE_BUNDLE)),true,)
IS_SEPARATE = $(if $(filter false,$(ENABLE_BUNDLE)),true,)

DIST_FOLDER ?= $(builddir)/dist
TYPESCRIPT_JS_DIR = $(builddir)/typescript_js

export NODE_PATH=$(abs_builddir)/node_module

if !ENABLE_MOBILEAPP
L10N_JSON = $(patsubst $(srcdir)/po/help-%.po,$(DIST_FOLDER)/l10n/help-%.json,$(L10N_PO))
L10N_JSON += $(patsubst $(srcdir)/po/ui-%.po,$(DIST_FOLDER)/l10n/ui-%.json,$(L10N_PO))
else
L10N_ALL_JS = $(DIST_FOLDER)/l10n-all.js
L10N_JSON = $(L10N_ALL_JS)
L10N_ALL_JS_ENTRY = l10n-all.js

$(L10N_ALL_JS) : $(wildcard $(srcdir)/po/ui-*.po) $(wildcard $(srcdir)/po/help-*.po) $(shell find $(srcdir)/l10n -name '*.*') $(srcdir)/util/create-l10n-all-js.py
	find $(srcdir)/po -name 'ui-*.po' -exec $(srcdir)/util/po2json.py {} -o {}.json \;
	find $(srcdir)/po -name 'help-*.po' -exec $(srcdir)/util/po2json.py {} -o {}.json \;
	@mkdir -p $(dir $@)
	python3 $(srcdir)/util/create-l10n-all-js.py >$@
	find $(srcdir)/po -name 'ui-*.po.json' -delete
	find $(srcdir)/po -name 'help-*.po.json' -delete
endif

QUIET_TSC = $(QUIET_TSC_$(V))
QUIET_TSC_ = $(QUIET_TSC_$(AM_DEFAULT_VERBOSITY))
QUIET_TSC_0 = @echo "  TSC     " $@;

QUIET_ESLINT = $(QUIET_ESLINT_$(V))
QUIET_ESLINT_ = $(QUIET_ESLINT_$(AM_DEFAULT_VERBOSITY))
QUIET_ESLINT_0 = @echo "  ESLINT  " $@;

QUIET_PRETTIER = $(QUIET_PRETTIER_$(V))
QUIET_PRETTIER_ = $(QUIET_PRETTIER_$(AM_DEFAULT_VERBOSITY))
QUIET_PRETTIER_0 = @echo "  PRETTIER" $@;

QUIET_M4 = $(QUIET_M4_$(V))
QUIET_M4_ = $(QUIET_M4_$(AM_DEFAULT_VERBOSITY))
QUIET_M4_0 = @echo "  M4      " $@;

QUIET_ECHO = $(QUIET_ECHO_$(V))
QUIET_ECHO_ = $(QUIET_ECHO_$(AM_DEFAULT_VERBOSITY))
QUIET_ECHO_0 = @echo "  ECHO    " $@;

QUIET_CMP = $(QUIET_CMP_$(V))
QUIET_CMP_ = $(QUIET_CMP_$(AM_DEFAULT_VERBOSITY))
QUIET_CMP_0 = @echo "  CMP     " $@;

QUIET_JSCPCD = $(QUIET_JSCPCD_$(V))
QUIET_JSCPCD_ = $(QUIET_JSCPCD_$(AM_DEFAULT_VERBOSITY))
QUIET_JSCPCD_0 = @echo "  JSCPCD  " $@;

QUIET_STYLELINT = $(QUIET_STYLELINT_$(V))
QUIET_STYLELINT_ = $(QUIET_STYLELINT_$(AM_DEFAULT_VERBOSITY))
QUIET_STYLELINT_0 = @echo "  STYLELINT" $@;

QUIET_UNOCOMMANDS = $(QUIET_UNOCOMMANDS_$(V))
QUIET_UNOCOMMANDS_ = $(QUIET_UNOCOMMANDS_$(AM_DEFAULT_VERBOSITY))
QUIET_UNOCOMMANDS_0 = @echo "  UNOCOMMANDS " $@;
QUIET_UNOSHORTCUTS = $(QUIET_UNOSHORTCUTS_$(V))
QUIET_UNOSHORTCUTS_ = $(QUIET_UNOSHORTCUTS_$(AM_DEFAULT_VERBOSITY))
QUIET_UNOSHORTCUTS_0 = @echo "  UNOSHORTCUTS " $@;

MOCHA_DIR = $(srcdir)/mocha_tests
MOCHA_JS_DIR = $(MOCHA_DIR)/workdir
# tsc can report a failure, but still create a .js and a second make attempt will succeed,
# so if this returns an error then delete the target
$(MOCHA_JS_DIR)/main.js: $(MOCHA_DIR)/main.ts
	$(QUIET_TSC) $(builddir)/node_modules/typescript/bin/tsc $< --outfile $@ --module none --moduleResolution node --lib dom,es2016 --target ES5 --downlevelIteration --allowJs || (rm -f $@ && false)

JQUERY_LIGHTNESS_IMAGE_PATH = $(srcdir)/images/jquery-ui-lightness
JQUERY_LIGHTNESS_IMAGES = $(wildcard $(JQUERY_LIGHTNESS_IMAGE_PATH)/*.png)
JQUERY_LIGHTNESS_DIST_IMAGES = $(patsubst $(JQUERY_LIGHTNESS_IMAGE_PATH)/%.png,$(DIST_FOLDER)/images/%.png,$(JQUERY_LIGHTNESS_IMAGES))

JQUERY_MINIFIED_IMAGE_PATH = node_modules/jquery-ui/themes/base/images
JQUERY_MINIFIED_IMAGES = $(wildcard $(JQUERY_MINIFIED_IMAGE_PATH)/*.png)
JQUERY_MINIFIED_DIST_IMAGES = $(patsubst $(JQUERY_MINIFIED_IMAGE_PATH)/%.png,$(DIST_FOLDER)/images/%.png,$(JQUERY_MINIFIED_IMAGES))

COOL_IMAGES_SRC = $(shell find $(srcdir)/images \( -name '*.png' -o -name '*.svg' -o -name '*.gif' \))
COOL_IMAGES_DST = $(patsubst $(srcdir)/%,$(DIST_FOLDER)/%,$(COOL_IMAGES_SRC))
COOL_IMAGES_CUSTOM_SRC = $(shell if test -n "$(CUSTOM_ICONS_DIRECTORY)"; then find $(CUSTOM_ICONS_DIRECTORY) -name '*.*'; fi)
COOL_IMAGES_CUSTOM_DST = $(patsubst $(CUSTOM_ICONS_DIRECTORY)/%,$(DIST_FOLDER)/images/%,$(COOL_IMAGES_CUSTOM_SRC))
COOL_L10N_SRC = $(shell find $(srcdir)/l10n -name '*.json')
if !ENABLE_MOBILEAPP
COOL_L10N_DST =  $(patsubst $(srcdir)/l10n/%,$(DIST_FOLDER)/l10n/%,$(COOL_L10N_SRC))
endif

COOL_HTML_SRC = $(shell find $(srcdir)/html -name '*.html')
COOL_HTML_DST = $(patsubst $(srcdir)/html/%.html,$(DIST_FOLDER)/%.html,$(COOL_HTML_SRC))

if ENABLE_WELCOME_MESSAGE
COOL_WELCOME_SRC = $(shell find $(srcdir)/welcome -name '*')
COOL_WELCOME_DST = $(patsubst $(srcdir)/welcome/%,$(DIST_FOLDER)/welcome/%,$(COOL_WELCOME_SRC))
endif

COOL_ADMIN_SRC = $(shell find $(srcdir)/admin -name '*.html' -or -name '*.css' -or -name '*.ttf' -or -name 'OFL.txt' -or -name '*.svg')
COOL_ADMIN_ALL = $(shell find $(srcdir)/admin -name '*')
COOL_ADMIN_DST = $(patsubst $(srcdir)/admin/%,$(DIST_FOLDER)/admin/%,$(COOL_ADMIN_SRC))

define file_target
$(1): $(2)
	@mkdir -p $$(dir $$@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		`cp $$< $$@`; \
	else \
		`ln -sf ../$$< $$@`; \
	fi

endef

define file_targets
$(foreach file,$(1),$(call file_target,\
	$(DIST_FOLDER)/$(notdir $(file)),\
	$(file)))
endef

define npm_source
	for package in $(1); do \
		npm pack $${package}; \
		fname=$$(ls *.tgz); \
		pdir=$${fname%%.tgz}; \
		mkdir -p $${pdir}; \
		tar -xzf $${fname} -C $${pdir} --strip-components=1; \
		rm $${fname}; \
	done
endef

# SVG icons
icons_dir = $(srcdir)/images

# Generate into the source tree so tsc --rootDir ./src sees it
icons_js = $(DIST_FOLDER)/src/control/Control.BackstageSVGIcons.js

# Backstage icons have to be embedded, so that the coloring works on the DOM
BACKSTAGE_SVG_NAMES = \
	lc_home.svg \
	lc_newdoc.svg \
	lc_open.svg \
	lc_shareas.svg \
	lc_printpreview.svg \
	lc_save.svg \
	lc_saveacopy.svg \
	lc_printlayout.svg \
	lc_exportto.svg

BACKSTAGE_SVGS = $(addprefix $(icons_dir)/,$(BACKSTAGE_SVG_NAMES))

$(icons_js): Makefile.am Makefile $(BACKSTAGE_SVGS)
	@mkdir -p "$(dir $@)"
	@{ \
	  echo "// AUTO-GENERATED. DO NOT EDIT."; \
	  echo "var BackstageSVGIcons = {"; \
	  for f in $(BACKSTAGE_SVGS); do \
	    name=$$(basename "$$f"); \
	    echo "  \"$$name\": \"" | tr -d '\n'; \
	    sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/\r//g' "$$f" | tr -d '\n'; \
	    echo "\","; \
	  done; \
	  echo "};"; \
	} > "$@"

COOL_ADMIN_TS = \
	admin/src/ModalDialogCreator.ts \
	admin/src/integrator/AdminIntegratorSettings.ts \
	admin/src/integrator/WordBook.ts \
	admin/src/integrator/Xcu.ts

COOL_ADMIN_TS_JS = $(patsubst %.ts,$(TYPESCRIPT_JS_DIR)/%.js,$(COOL_ADMIN_TS))

COOL_ADMIN_JS =\
	admin/src/Base.js \
	admin/src/Admin.js \
	admin/src/AdminSocketBase.js \
	admin/src/Util.js \
	admin/src/AdminSocketOverview.js \
	admin/src/AdminSocketAnalytics.js \
	admin/src/AdminSocketSettings.js \
	admin/src/AdminSocketHistory.js \
	admin/src/AdminSocketLog.js \
	admin/src/AdminClusterOverview.js \
	admin/src/AdminClusterOverviewAbout.js

NODE_MODULES_SRC =\
	autolinker@3.14.1 \
	dompurify@3.3.2 \
	json-js@1.1.2 \
	l10n-for-node@0.0.1 \
	@braintree/sanitize-url@6.0.0

COOL_CSS =\
	$(srcdir)/css/leaflet.css \
	$(srcdir)/css/leaflet-spinner.css \
	$(srcdir)/css/selectionMarkers.css \
	$(srcdir)/css/color-palette.css \
	$(srcdir)/css/color-palette-dark.css \
	$(srcdir)/css/cool.css \
	$(srcdir)/css/toolbar.css \
	$(srcdir)/css/partsPreviewControl.css \
	$(srcdir)/css/searchControl.css \
	$(srcdir)/css/impress.css \
	$(srcdir)/css/impress-mobile.css \
	$(srcdir)/css/spreadsheet.css \
	$(srcdir)/css/writer-mobile.css \
	$(srcdir)/css/writer.css \
	$(srcdir)/css/iframedialog.css \
	$(builddir)/node_modules/smartmenus/dist/css/sm-core-css.css \
	$(builddir)/node_modules/smartmenus/dist/css/sm-simple/sm-simple.css \
	$(srcdir)/css/welcome.css \
	$(srcdir)/css/menubar.css \
	$(srcdir)/css/mobilewizard.css \
	$(srcdir)/css/jsdialogs.css \
	$(srcdir)/css/helpdialog.css \
	$(srcdir)/css/btns.css \
	$(srcdir)/css/notebookbar.css \
	$(srcdir)/css/backstage.css \
	$(srcdir)/css/jssidebar.css \
	$(srcdir)/css/forced-colors.css \
	$(srcdir)/css/aichat-sidebar.css \
	$(srcdir)/css/override-smartmenus.css \
	$(srcdir)/css/jquery-ui-lightness.css \
	$(srcdir)/css/infobar.css \
	$(srcdir)/css/tooltip.css \
	$(srcdir)/css/progressbar.css \
	$(srcdir)/css/settings.css

COOL_CSS_DST = $(foreach file,$(COOL_CSS),$(DIST_FOLDER)/$(notdir $(file)))
COOL_CSS_M4 = $(strip $(foreach file,$(COOL_CSS),$(notdir $(file))))

$(eval $(call file_targets,$(COOL_CSS)))

WORKER_NODE_MODULES_JS =\
	node_modules/fzstd/umd/index.js

$(DIST_FOLDER)/src/app/TaskWorker.js: \
	$(srcdir)/src/app/TaskWorker.js.m4 \
	$(srcdir)/src/app/TaskWorkerSrc.js \
	$(DIST_FOLDER)/src/layer/tile/CanvasTileUtils.js \
	$(WORKER_NODE_MODULES_JS)

	$(QUIET_M4) m4 -PE -DDEBUG=$(ENABLE_DEBUG) \
		-DMOBILEAPP=$(ENABLE_MOBILEAPP) \
		-DNODE_MODULES_JS=$(subst $(SPACE),$(COMMA),$(WORKER_NODE_MODULES_JS)) \
		-DTILEUTILS_JS=$(DIST_FOLDER)/src/layer/tile/CanvasTileUtils.js \
		-DWORKER_JS=$(srcdir)/src/app/TaskWorkerSrc.js \
		$(srcdir)/src/app/TaskWorker.js.m4 > $@

NODE_MODULES_JS =\
	$(if $(IS_SEPARATE),node_modules/hammerjs/hammer.js,node_modules/hammerjs/hammer.min.js) \
	node_modules/dompurify/dist/purify.min.js \
	node_modules/jquery/dist/jquery.js \
	node_modules/jquery-ui/dist/jquery-ui.js \
	$(shell find node_modules/jquery-ui/ui/i18n -name '*.js' 2>/dev/null) \
	node_modules/smartmenus/dist/jquery.smartmenus.js \
	node_modules/fzstd/umd/index.js \
	node_modules/marked/marked.min.js \
	node_modules/gl-matrix/gl-matrix-min.js

COOL_LIBS_JS =\
	Autolinker.js \
	sanitize-url.js

if !ENABLE_MOBILEAPP
COOL_LIBS_JS +=\
	l10n.js
endif

NODE_MODULES_JS_SRC = $(patsubst %.js,$(builddir)/%.js,$(NODE_MODULES_JS))
NODE_MODULES_JS_DST = $(patsubst %.js,$(DIST_FOLDER)/%.js,$(NODE_MODULES_JS))

COOL_LIBS_JS_SRC = $(patsubst %.js,$(srcdir)/js/%.js,$(COOL_LIBS_JS))
COOL_LIBS_JS_DST = $(patsubst %.js,$(DIST_FOLDER)/%.js,$(COOL_LIBS_JS))

# the ordering of this is delicate to get JS initialization right,
# this list includes both Javascript and Typescript generated files
# in a rather precise order.
COOL_JS_LST =\
	src/app/DocUtil.ts \
	src/app/LocaleService.ts \
	src/app/LayoutingService.ts \
	src/app/ServerConnectionService.ts \
	src/app/DocEvents.ts \
	src/docstate.ts \
	src/docstatefunctions.js \
	src/docdispatcher.ts \
	src/app/TableStylesService.ts \
	src/core/geometry.ts \
	src/core/TimerRegistry.ts \
	src/app/GraphicSelectionMiddleware.ts \
	src/app/TextSelectionMiddleware.ts \
	src/app/CellSelectionMarkerMiddleware.ts \
	src/app/TableMiddleware.ts \
	src/app/ImpressSelectionMiddleware.ts \
	src/app/ViewLayout.ts \
	src/app/ViewLayoutNewBase.ts \
	src/app/ViewLayoutWriter.ts \
	src/app/ViewLayoutMultiPage.ts \
	src/app/ViewLayoutCompareChanges.ts \
	src/app/DocumentBase.ts \
	src/app/DocumentView.ts \
	src/app/TileCoordData.ts \
	src/app/PaneBorder.ts \
	src/app/Tile.ts \
	src/app/BitmapTileManager.ts \
	src/app/TilesMiddleware.ts \
	src/app/SearchService.ts \
	src/app/BaseClass.ts \
	src/app/Events.ts \
	src/app/Rectangle.ts \
	src/app/Util.ts \
	src/app/LOUtil.ts \
	src/app/IconUtil.ts \
	src/app/iface/DocLayer.Interface.ts \
	src/app/iface/Map.Interface.ts \
	src/app/iface/Control.Interface.ts \
	src/app/Log.ts \
	src/app/Debug.ts \
	src/app/A11yValidator.ts \
	src/app/Socket.ts \
	src/app/TracingEvents.ts \
	src/app/ServerCommand.ts \
	src/Leaflet.js \
	src/errormessages.js \
	src/unocommands.js \
	src/unoshortcuts.js \
	src/UNO/Key.js \
	src/core/Util.js \
	src/core/Class.js \
	src/core/Events.js \
	src/geometry/Point.ts \
	src/geometry/Bounds.ts \
	src/geometry/Transformation.ts \
	src/app/dom/DomUtil.ts \
	src/geo/LatLng.js \
	src/geo/LatLngBounds.js \
	src/geo/projection/Projection.LonLat.js \
	src/geo/crs/CRS.js \
	src/control/IFrameDialog.js \
	src/map/Map.js \
	src/control/Control.IdleHandler.ts \
	src/map/Clipboard.js \
	src/layer/tile/SheetSwitch.ts \
	src/layer/Layer.js \
	src/layer/tile/SheetGeometry.ts \
	src/layer/tile/CanvasTileUtils.ts \
	src/canvas/CanvasSectionProps.js \
	src/canvas/CanvasSectionContainer.ts \
	src/canvas/CanvasSectionObject.ts \
	src/canvas/sections/HTMLObjectSection.ts \
	src/canvas/sections/RulerSpacerSection.ts \
	src/canvas/sections/CompareChangesLabelSection.ts \
	src/canvas/sections/CommentMarkerSubSection.ts \
	src/canvas/sections/CommentSection.ts \
	src/canvas/sections/CommentListSection.ts \
	src/canvas/sections/CalcGridSection.ts \
	src/canvas/sections/SplitSection.ts \
	src/canvas/sections/SplitterLinesSection.ts \
	src/canvas/sections/PixelGridSection.ts \
	src/canvas/sections/DebugOverlaySection.ts \
	src/canvas/sections/PreloadMapSection.ts \
	src/canvas/sections/TilesSection.ts \
	src/canvas/sections/AutoFillBaseSection.ts \
	src/canvas/sections/CellFillMarkerSection.ts \
	src/canvas/sections/ImpressSelectionRectangle.ts \
	src/canvas/sections/TableFillMarkerSection.ts \
	src/canvas/sections/TableResizeMarkerSection.ts \
	src/canvas/sections/TableSelectMarkerSection.ts \
	src/canvas/sections/FocusCellSection.ts \
	src/canvas/sections/URLPopUpSection.ts \
	src/canvas/sections/InvalidationRectangleSection.ts \
	src/canvas/sections/ShapeHandlesSection.ts \
	src/canvas/sections/ShapeHandleCustomSubSection.ts \
	src/canvas/sections/ShapeHandlePolySubSection.ts \
	src/canvas/sections/ShapeHandleGluePointSubSection.ts \
	src/canvas/sections/ShapeHandleAnchorSubSection.ts \
	src/canvas/sections/ShapeHandleRotationSubSection.ts \
	src/canvas/sections/ShapeHandleDiagramSubSection.ts \
	src/canvas/sections/ShapeHandleScalingSubSection.ts \
	src/canvas/sections/ValidityInputHelpSection.ts \
	src/canvas/sections/CellCursorSection.ts \
	src/canvas/sections/CellSelectionHandleSection.ts \
	src/canvas/sections/CalcValidityDropDownSection.ts \
	src/canvas/sections/FormulaErrorHelpSection.ts \
	src/canvas/sections/TextSelectionHandleSection.ts \
	src/canvas/sections/TextSelectionSection.ts \
	src/canvas/sections/OtherViewGraphicSelectionSection.ts \
	src/canvas/sections/OtherViewCellCursorSection.ts \
	src/canvas/sections/FormFieldButton.ts \
	src/canvas/sections/OtherViewCursorSection.ts \
	src/layer/vector/CEventsHandler.ts \
	src/layer/vector/CPointSet.ts \
	src/layer/vector/CPath.ts \
	src/layer/vector/CLineUtil.ts \
	src/layer/vector/CPolyline.ts \
	src/layer/vector/CPolyUtil.ts \
	src/layer/vector/CPolygon.ts \
	src/layer/vector/CRectangle.ts \
	src/layer/marker/Cursor.ts \
	src/layer/vector/CanvasOverlay.ts \
	src/canvas/sections/ScrollSection.ts \
	src/canvas/sections/MouseControl.ts \
	src/app/LayerUtil.ts \
	src/layer/tile/CanvasTileLayer.js \
	src/layer/vector/CDarkOverlay.ts \
	src/layer/SplitPanesContext.ts \
	src/layer/CalcSplitPanesContext.ts \
	src/layer/tile/WriterTileLayer.js \
	src/layer/tile/ImpressTileLayer.js \
	src/layer/tile/CalcTileLayer.js \
	src/canvas/sections/CursorHeaderSection.ts \
	src/canvas/sections/CursorHandler.ts \
	src/canvas/sections/TableInsertMarkerSection.ts \
	src/canvas/sections/GenericButtonSection.ts \
	src/canvas/sections/ChartContextButtonSection.ts \
	src/canvas/sections/DiagramButtonSection.ts \
	src/canvas/sections/ContentControlDropdownSubSection.ts \
	src/canvas/sections/ContentControlSection.ts \
	src/canvas/sections/TooltipAnchorSection.ts \
	src/layer/marker/ProgressOverlay.js \
	src/layer/marker/TextInput.js \
	src/layer/marker/A11yTextInput.js \
	src/geometry/LineUtil.js \
	src/core/Handler.js \
	src/dom/DomEvent.js \
	src/dom/Draggable.js \
	src/dom/NotebookbarAccessibilityDefinitions.js \
	src/dom/NotebookbarAccessibility.js \
	src/map/handler/Map.Scroll.js \
	src/dom/DomEvent.Pointer.js \
	src/map/handler/Map.Keyboard.js \
	src/map/handler/Map.KeyboardShortcuts.ts \
	src/map/handler/Map.Welcome.js \
	src/map/handler/Map.Feedback.js \
	src/map/handler/Map.Settings.ts \
	src/map/handler/Map.VersionBar.js \
	src/dom/DomEvent.MultiClick.js \
	src/map/handler/Map.Print.js \
	src/map/handler/Map.SlideShow.js \
	src/map/handler/Map.FileInserter.js \
	src/map/handler/Map.StateChanges.js \
	src/map/handler/Map.WOPI.js \
	src/control/jsdialog/Util.UniqueId.ts \
	src/control/jsdialog/Definitions.MenuCommands.ts \
	src/control/jsdialog/Control.ts \
	src/control/jsdialog/Control.ContextMenu.ts \
	src/control/Control.AboutDialog.ts \
	src/control/Control.Toolbar.js \
	src/control/Control.Command.js \
	src/control/Control.js \
	src/control/Control.PartsPreview.js \
	src/control/Control.GroupBase.ts \
	src/control/Control.CornerGroup.ts \
	src/control/Control.RowGroup.ts \
	src/control/Control.ColumnGroup.ts \
	src/control/Control.CornerHeader.ts \
	src/control/Control.Header.ts \
	src/control/Control.ColumnHeader.ts \
	src/control/Control.RowHeader.ts \
	src/control/Control.DocumentRepair.js \
	src/control/Control.DownloadProgress.js \
	src/control/Control.Menubar.ts \
	src/control/Control.Tabs.js \
	src/control/Control.Permission.js \
	src/control/Control.Selection.js \
	src/control/Control.ServerAuditDialog.ts \
	src/control/Control.LokDialog.js \
	src/control/Control.AlertDialog.js \
	src/control/Control.Tooltip.js \
	src/control/Control.ESignature.ts \
	src/control/Control.ESignatureDialog.ts \
	src/control/Control.AIChatSidebar.ts \
	src/control/ColorPicker.ts \
	src/control/jsdialog/Util.Shortcuts.ts \
	src/control/jsdialog/Util.ModelState.ts \
	src/control/jsdialog/Component.Base.ts \
	src/control/jsdialog/Component.Toolbar.ts \
	src/control/jsdialog/Definitions.Menu.ts \
	src/control/jsdialog/Definitions.DialogModification.ts \
	src/control/jsdialog/Definitions.WindowId.ts \
	src/control/jsdialog/Util.Accessibility.ts \
	src/control/jsdialog/Util.ButtonType.ts \
	src/control/jsdialog/Util.Dropdown.ts \
	src/control/jsdialog/Util.GenerateTableIcon.ts \
	src/control/jsdialog/Util.Events.ts \
	src/control/jsdialog/Util.KeyCodes.ts \
	src/control/jsdialog/Util.KeyboardTabNavigation.ts \
	src/control/jsdialog/Util.KeyboardGridNavigation.ts \
	src/control/jsdialog/Util.KeyboardListNavigation.ts \
	src/control/jsdialog/Util.KeyboardRadioGroupNavigation.ts \
	src/control/jsdialog/Util.FocusCycle.js \
	src/control/jsdialog/Util.FormulaBarSelectionHandle.ts \
	src/control/jsdialog/Util.MessageRouter.ts \
	src/control/jsdialog/Util.ModalHelper.js \
	src/control/jsdialog/Util.OnDemandRenderer.ts \
	src/control/jsdialog/Util.ScrollableBar.ts \
	src/control/jsdialog/Util.ScrollIntoView.ts \
	src/control/jsdialog/Util.SnackbarController.ts \
	src/control/jsdialog/Util.StateChange.ts \
	src/control/jsdialog/Widget.ButtonBox.ts \
	src/control/jsdialog/Widget.Checkbox.ts \
	src/control/jsdialog/Widget.Calendar.js \
	src/control/jsdialog/Widget.ColorPickerButton.js \
	src/control/jsdialog/Widget.ColorPicker.ts \
	src/control/jsdialog/Widget.PageMarginEntry.ts \
	src/control/jsdialog/Widget.PageSizeEntry.ts \
	src/control/jsdialog/Widget.Combobox.js \
	src/control/jsdialog/Widget.Containers.ts \
	src/control/jsdialog/Widget.DropDown.ts \
	src/control/jsdialog/Widget.DrawingArea.js \
	src/control/jsdialog/Widget.Edit.ts \
	src/control/jsdialog/Widget.FormulabarEdit.js \
	src/control/jsdialog/Widget.Frame.js \
	src/control/jsdialog/Widget.HTMLContent.ts \
	src/control/jsdialog/Widget.IconView.ts \
	src/control/jsdialog/Widget.NotebookbarIconView.ts \
	src/control/jsdialog/Widget.MenuButton.js \
	src/control/jsdialog/Widget.Listbox.ts \
	src/control/jsdialog/Widget.MobileBorderSelector.js \
	src/control/jsdialog/Widget.MobileTabControl.js \
	src/control/jsdialog/Widget.MultilineEdit.js \
	src/control/jsdialog/Widget.NewSlideLayoutEntry.ts \
	src/control/jsdialog/Widget.OverflowGroup.ts \
	src/control/jsdialog/Widget.OverflowManager.ts \
	src/control/jsdialog/Widget.Progressbar.js \
	src/control/jsdialog/Widget.RadioButton.ts \
	src/control/jsdialog/Widget.PushButton.ts \
	src/control/jsdialog/Widget.Comment.ts \
	src/control/jsdialog/Widget.SpinField.ts \
	src/control/jsdialog/Widget.FixedTextControl.ts \
	src/control/jsdialog/Widget.ScrolledWindow.js \
	src/control/jsdialog/Widget.SearchEdit.ts \
	src/control/jsdialog/Widget.SidebarContainers.ts \
	src/control/jsdialog/Widget.Toolitem.ts \
	src/control/jsdialog/Widget.Text.ts \
	src/control/jsdialog/Widget.Timefield.js \
	src/control/jsdialog/Widget.TreeView.ts \
	src/control/Control.JSDialog.js \
	src/control/Control.JSDialogBuilder.js \
	src/control/Control.MobileWizardBuilder.js \
	src/control/Control.MobileWizardWindow.js \
	src/control/Control.MobileWizard.js \
	src/control/Control.LanguageDialog.js \
	src/control/Control.MobileSlide.js \
	src/control/Control.StatusBar.js \
	src/control/Control.MobileSearchBar.ts \
	src/control/Control.MobileTopBar.ts \
	src/control/Control.MobileBottomBar.js \
	src/control/Control.UserList.ts \
	src/control/Control.FormulaBarJSDialog.js \
	src/control/Control.SheetsBar.js \
	src/control/Control.PresentationBar.js \
	src/control/Control.TopToolbar.js \
	src/control/Control.SaveState.ts \
	src/control/Control.NotebookbarBase.ts \
	src/control/Control.UIManager.ts \
	src/control/Control.DocumentNameInput.js \
	src/control/Notebookbar.WriterReferencesTab.ts \
	src/control/Control.Notebookbar.js \
	src/control/Control.NotebookbarWriter.js \
	src/control/Control.NotebookbarCalc.js \
	src/control/Control.NotebookbarImpress.js \
	src/control/Control.NotebookbarDraw.js \
	src/control/Control.NotebookbarBuilder.js \
	$(icons_js) \
	src/control/Control.BackstageView.ts \
	src/control/Control.ContextToolbar.ts \
	src/control/Control.SidebarBase.ts \
	src/control/Control.Sidebar.ts \
	src/control/Notebookbar.CalcTableTab.ts \
	src/control/Notebookbar.ImpressAnimationTab.ts \
	src/control/Notebookbar.ImpressTransitionTab.ts \
	src/control/Sidebar.FromNotebookbarPanel.ts \
	src/control/Control.NavigatorPanel.ts \
	src/control/Control.QuickFindPanel.ts \
	src/control/AutoCompletePopup.ts \
	src/control/Control.Mention.ts \
	src/control/Control.FormulaUsagePopup.ts \
	src/control/Control.FormulaAutoCompletePopup.ts \
	src/control/Control.AutoFillPreviewTooltip.ts \
	src/control/Control.AddressInputField.ts \
	src/control/Control.Zotero.js \
	src/control/Permission.js \
	src/control/Toolbar.js \
	src/control/Signing.js \
	src/control/Parts.js \
	src/control/Styles.js \
	src/control/Ruler.ts \
	src/control/VRuler.ts \
	src/control/HRuler.ts \
	src/control/PermissionMode.ts \
	src/slideshow/SlideCache.ts \
	src/slideshow/SlideCompositor.ts \
	src/slideshow/LayerRenderer.ts \
	src/slideshow/LayerDrawing.ts \
	src/slideshow/LayersCompositor.ts \
	src/slideshow/RenderContext.ts \
	src/slideshow/VideoRenderer.ts \
	src/slideshow/SlideRenderer.ts \
	src/slideshow/SlideshowMiddleware.ts \
	src/slideshow/SlideShowPresenter.ts \
	src/slideshow/PresenterConsole.js \
	src/slideshow/SimpleTextureRenderer.ts \
	src/slideshow/StaticTextRenderer.ts \
	src/slideshow/PauseTimer.ts \
	src/slideshow/TransitionFactory.ts \
	src/slideshow/CanvasLoader.ts \
	src/slideshow/Operation.ts \
	src/slideshow/Primitive.ts \
	src/slideshow/engine/ActivityFactory.ts \
    src/slideshow/engine/ActivityQueue.ts \
    src/slideshow/engine/AnimationFactory.ts \
    src/slideshow/engine/Duration.ts \
    src/slideshow/engine/Effect.ts \
    src/slideshow/engine/ElapsedTime.ts \
    src/slideshow/engine/EventMultiplexer.ts \
    src/slideshow/engine/Event.ts \
    src/slideshow/engine/FrameSynchronization.ts \
    src/slideshow/engine/InteractiveAnimationSequence.ts \
    src/slideshow/engine/MetaPresentation.ts \
    src/slideshow/engine/MetaSlide.ts \
    src/slideshow/engine/NextEffectEventArray.ts \
    src/slideshow/engine/SlideAnimations.ts \
    src/slideshow/engine/SlideChangeBase.ts \
    src/slideshow/engine/SlideShowHandler.ts \
    src/slideshow/engine/SlideShowNavigator.ts \
    src/slideshow/engine/SlideTransition.ts \
    src/slideshow/engine/SourceEventElement.ts \
    src/slideshow/engine/TimerEventQueue.ts \
    src/slideshow/engine/Timing.ts \
    src/slideshow/engine/tools.ts \
    src/slideshow/engine/activities/AnimationActivity.ts \
    src/slideshow/engine/activities/ActivityBase.ts \
    src/slideshow/engine/activities/SimpleContinuousActivityBase.ts \
    src/slideshow/engine/activities/ContinuousActivityBase.ts \
    src/slideshow/engine/activities/ContinuousKeyTimeActivityBase.ts \
    src/slideshow/engine/activities/DiscreteActivityBase.ts \
    src/slideshow/engine/activities/FromToByActivity.ts \
    src/slideshow/engine/activities/SetActivity.ts \
    src/slideshow/engine/activities/SimpleActivity.ts \
    src/slideshow/engine/activities/ValueListActivity.ts \
    src/slideshow/engine/animationnodes/BaseNode.ts \
    src/slideshow/engine/animationnodes/AnimationBaseNode.ts \
    src/slideshow/engine/animationnodes/AnimationColorNode.ts \
    src/slideshow/engine/animationnodes/AnimationNodeInfo.ts \
    src/slideshow/engine/animationnodes/AnimationPathMotionNode.ts \
    src/slideshow/engine/animationnodes/AnimationSetNode.ts \
    src/slideshow/engine/animationnodes/AnimationTransformNode.ts \
    src/slideshow/engine/animationnodes/AnimationTransitionFilterNode.ts \
    src/slideshow/engine/animationnodes/BaseContainerNode.ts \
    src/slideshow/engine/animationnodes/NodeContext.ts \
    src/slideshow/engine/animationnodes/PropertyAnimationNode.ts \
    src/slideshow/engine/AnimatedElement.ts \
    src/slideshow/TransitionsInfo.ts \
    src/slideshow/Transition2d.ts \
    src/slideshow/Transition3d.ts \
    src/slideshow/transition/ClippingTransition.ts \
	src/slideshow/transition/FadeTransition.ts \
	src/slideshow/transition/PlusTransition.ts \
	src/slideshow/transition/CircleTransition.ts \
	src/slideshow/transition/VenetianTransition.ts \
	src/slideshow/transition/WedgeTransition.ts \
	src/slideshow/transition/OvalTransition.ts \
	src/slideshow/transition/BoxTransition.ts \
	src/slideshow/transition/DiamondTransition.ts\
	src/slideshow/transition/CheckersTransition.ts \
	src/slideshow/transition/PushTransition.ts \
	src/slideshow/transition/SimpleDissolveTransition.ts \
	src/slideshow/transition/BarsTransition.ts \
	src/slideshow/transition/CutTransition.ts \
	src/slideshow/transition/WheelTransition.ts \
	src/slideshow/transition/UncoverTransition.ts \
	src/slideshow/transition/CoverTransition.ts \
	src/slideshow/transition/SplitTransition.ts \
	src/slideshow/transition/WipeTransition.ts \
	src/slideshow/transition/IrisWipeTransition.ts \
	src/slideshow/transition/EllipseWipeTransition.ts \
	src/slideshow/transition/SlideWipeTransition.ts \
	src/slideshow/transition/BarWipeTransition.ts \
	src/slideshow/transition/DiagonalTransition.ts \
	src/slideshow/transition/PushWipeTransition.ts \
	src/slideshow/transition/CombTransition.ts \
	src/slideshow/transition/NoTransition.ts \
	src/slideshow/transition3d/SimpleTransition.ts \
	src/slideshow/transition3d/CubeTransition.ts \
	src/slideshow/transition3d/HelixTransition.ts \
	src/slideshow/transition3d/FallTransition.ts \
	src/slideshow/transition3d/ReflectionTransition.ts \
	src/slideshow/transition3d/RochadeTransition.ts \
	src/slideshow/transition3d/TurnDownTransition.ts \
	src/slideshow/transition3d/Venetian3dTransition.ts \
	src/slideshow/transition3d/TurnAroundTransition.ts \
	src/slideshow/transition3d/PermTextureTransition.ts \
	src/slideshow/transition3d/RippleTransition.ts \
	src/slideshow/transition3d/NewsFlashTransition.ts \
	src/slideshow/transition3d/DissolveTransition3d.ts \
	src/slideshow/transition3d/FlipTilesTransition.ts \
	src/slideshow/transition3d/StaticNoiseTransition.ts \
	src/slideshow/transition3d/MicsShapeWipeTransition.ts \
	src/slideshow/operation/RotateAndScaleDepth.ts \
	src/slideshow/operation/SEllipseTranslate.ts \
	src/slideshow/operation/SRotate.ts \
	src/slideshow/operation/SScale.ts \
	src/slideshow/operation/STranslate.ts \
	src/control/Control.Scroll.Annotation.js \
	src/main.js

COOL_JS_SRC = $(filter %.js,$(COOL_JS_LST))
COOL_TS_SRC = $(filter %.ts,$(COOL_JS_LST))

# beware these are not helpfully ordered
COOL_TS_JS_DST = $(patsubst src/%.ts,$(DIST_FOLDER)/src/%.js,$(COOL_TS_SRC))
COOL_JS_DST = $(patsubst src/%.js,$(DIST_FOLDER)/src/%.js,$(COOL_JS_SRC)) $(COOL_TS_JS_DST) $(DIST_FOLDER)/src/app/TaskWorker.js
# should be no lingering / obsolete files:
COOL_ASSERT_INTERSECT = $(filter $(COOL_JS_SRC), $(patsubst %.ts,%.js,$(COOL_TS_SRC)))

COMMA := ,
EMPTY :=
SPACE := $(EMPTY) $(EMPTY)
COOL_VERSION = $(shell cd $(srcdir) && git log -1 --format=%h --abbrev=10)
INTERMEDIATE_DIR ?= $(if $(IS_SEPARATE),$(abs_builddir)/debug,$(abs_builddir)/release)

# Don't use EXTRA_DIST with $(shell find) as it causes "Argument list too long" errors
# Instead, use dist-hook to copy files
EXTRA_DIST =

dist-hook:
	cd $(srcdir) && find . -type f \
		-not -path './.git/*' \
		-not -path './node_modules/*' \
		-not -path './l10n/*' \
		-not -path './images/*' \
		-not -path './dist/*' \
		-not -path './debug/*' \
		-not -path './release/*' \
		-exec cp -p --parents {} $(distdir) \;

SUBDIRS = l10n images

COOL_JS_BUNDLE = $(patsubst %.ts,%.js,$(COOL_JS_LST))

define bundle_cool
	$(if $(IS_SEPARATE),\
		@touch $@,
		$(QUIET_M4) m4 -PE -DIOSAPP=$(ENABLE_IOSAPP) \
			-DMACOSAPP=$(ENABLE_MACOSAPP) \
			-DQTAPP=$(ENABLE_QTAPP) \
			-DWINDOWSAPP=$(ENABLE_WINDOWSAPP) \
			-DANDROIDAPP=$(ENABLE_ANDROIDAPP) \
			-DMOBILEAPPNAME="$(APP_NAME)" \
			-DVERSION=$(COOL_VERSION) \
			-DCOPYRIGHT=$(srcdir)/src/copyright.js \
			-DCOOL_JS=$(subst $(SPACE),$(COMMA),$(patsubst src/%,$(DIST_FOLDER)/src/%,$(COOL_JS_BUNDLE))) \
			$(srcdir)/cool-src.js.m4 > $@)
endef

define prereq_cool
	$(if $(IS_SEPARATE),$(COOL_JS_DST),$(COOL_JS_SRC))
endef

define bundle_css
	$(if $(IS_SEPARATE),\
		@touch $@,\
		@echo "Uglify cool css files..."
		@$(NODE) node_modules/uglifycss/uglifycss $(COOL_CSS) > $@)
endef

define prereq_css
	$(if $(IS_SEPARATE),$(COOL_CSS_DST),$(COOL_CSS))
endef

define bundle_all
	$(if $(IS_SEPARATE),\
		@touch $@,\
		$(QUIET_M4) m4 -PE -DL10N_ALL_JS=$(L10N_ALL_JS) \
			-DNODE_MODULES_JS=$(subst $(SPACE),$(COMMA),$(NODE_MODULES_JS)) \
			-DCOOL_LIBS_JS=$(subst $(SPACE),$(COMMA),$(COOL_LIBS_JS_SRC)) \
			-DCOOL_JS=$(INTERMEDIATE_DIR)/cool-src.js \
			$(srcdir)/bundle.js.m4 > $(INTERMEDIATE_DIR)/bundle.js
		@echo "Uglify cool js files..."
		@$(NODE) node_modules/uglify-js/bin/uglifyjs \
			-O max_line_len=100 \
			$(INTERMEDIATE_DIR)/bundle.js \
			--output $@)
endef

define prereq_all
	$(if $(IS_SEPARATE),$(NODE_MODULES_JS_DST) $(COOL_LIBS_JS_DST),\
		$(NODE_MODULES_JS_SRC) $(COOL_LIBS_JS_SRC))
endef

define global_file
	$(if $(IS_SEPARATE),
		@cp $< $@,
		@echo "Uglify global.js file..."
		@$(NODE) node_modules/uglify-js/bin/uglifyjs $< --output $@)
endef

define run_eslint_check
	$(QUIET_ESLINT) $(NODE) node_modules/eslint/bin/eslint.js \
		--resolve-plugins-relative-to $(abs_builddir) \
		--ignore-path $(srcdir)/.eslintignore \
		--config .eslintrc \
		--no-eslintrc $(1)
endef

define run_prettier_check
	$(QUIET_PRETTIER) $(NODE) node_modules/.bin/prettier \
		--ignore-path $(srcdir)/.prettierignore \
		--ignore-path $(srcdir)/.beforeprettier \
		--log-level warn \
		$(1)
endef

.PHONY: build-cool

all-local: build-cool

ADMIN_BUNDLE = $(DIST_FOLDER)/admin-bundle.js

if ENABLE_WASM
$(DIST_FOLDER)/emscripten-module.js: ../wasm/emscripten-module.js
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/emscripten-zetajs-example.js: $(abs_top_srcdir)/wasm/emscripten-zetajs-example.js
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/online%: ../wasm/online%
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/soffice%: ../wasm/soffice%
	@mkdir -p $(dir $@)
	@cp $< $@

WASM_FILES= \
	$(DIST_FOLDER)/emscripten-module.js \
	$(DIST_FOLDER)/online.js \
	$(DIST_FOLDER)/online.wasm \
	$(DIST_FOLDER)/soffice.data \
	$(DIST_FOLDER)/soffice.data.js.metadata

if ENABLE_WASM_ADDITIONAL_FILES
WASM_FILES += \
	$(DIST_FOLDER)/online.data
endif

if ENABLE_DEBUG
WASM_FILES += \
	$(DIST_FOLDER)/online.wasm.debug.wasm \
	$(DIST_FOLDER)/online.wasm.debug.wasm.dwp
endif

if ENABLE_WASM_ZETAJS

$(DIST_FOLDER)/smoketest.js: $(WITH_WASM_ZETAJS)/test/smoketest.js
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/zeta.js: $(WITH_WASM_ZETAJS)/source/zeta.js
	@mkdir -p $(dir $@)
	@cp $< $@

WASM_FILES += $(DIST_FOLDER)/zeta.js

if ENABLE_WASM_EMBINDTEST
WASM_FILES += $(DIST_FOLDER)/smoketest.js
endif

endif

if ENABLE_WASM_ZETAJS_EXAMPLE
WASM_FILES += $(DIST_FOLDER)/emscripten-zetajs-example.js
endif

endif

WASM_FALLBACK_FILES =
if ENABLE_WASM_FALLBACK
WASM_FALLBACK_FILES += $(DIST_FOLDER)/wasm
endif

TEMPLATE_MANIFEST = $(DIST_FOLDER)/templates/templates.js

$(TYPESCRIPT_JS_DIR)/%.js: $(srcdir)/%.ts
	@mkdir -p $(dir $@)
	$(QUIET_TSC) $(builddir)/node_modules/typescript/bin/tsc --outFile $@ $<

build-cool: \
	$(COOL_L10N_DST) \
	$(L10N_JSON) \
	$(COOL_IMAGES_DST) \
	$(COOL_IMAGES_CUSTOM_DST) \
	$(COOL_TEMPLATES_DST) \
	$(JQUERY_LIGHTNESS_DIST_IMAGES) \
	$(JQUERY_MINIFIED_DIST_IMAGES) \
	$(ADMIN_BUNDLE) \
	$(DIST_FOLDER)/bundle.css \
	$(DIST_FOLDER)/device-mobile.css \
	$(DIST_FOLDER)/device-tablet.css \
	$(DIST_FOLDER)/device-desktop.css \
	$(DIST_FOLDER)/progressbar.css \
	$(DIST_FOLDER)/settings.css \
	$(DIST_FOLDER)/color-palette.css \
	$(DIST_FOLDER)/color-palette-dark.css \
	$(DIST_FOLDER)/bundle.js \
	$(DIST_FOLDER)/cool.html \
	$(WASM_FILES) \
	$(WASM_FALLBACK_FILES) \
	$(TEMPLATE_MANIFEST) \
	adminIntegratorSettings
	@echo "build cool completed"
if APP_HAS_BRANDING
	@if test -d "$(APP_BRANDING_DIR)" ; then cp -a "$(APP_BRANDING_DIR)"/branding* $(DIST_FOLDER)/ ; fi
	@if test -d "$(APP_BRANDING_DIR)" ; then cp -a "$(APP_BRANDING_DIR)"/images/*.svg $(DIST_FOLDER)/images/ ; fi
	@if test -d "$(APP_BRANDING_DIR)" ; then cp -a "$(APP_BRANDING_DIR)/images/toolbar-bg-logo.svg" $(DIST_FOLDER)/images/toolbar-bg.svg ; fi
if ENABLE_MACOS
	@if test -d "$(APP_BRANDING_DIR)" ; then cp -a "$(APP_BRANDING_DIR)/welcome" "$(DIST_FOLDER)/" ; fi
endif
	@echo
	@echo "JS, HTML and CSS has been updated ($(DIST_FOLDER))."
endif

if ENABLE_CYPRESS
	cp -a  "$(srcdir)/../cypress_test/data/multiuser/cypress-multiuser.html" "$(DIST_FOLDER)/cypress-multiuser.html"
	@echo "Added HTML file for multiuser tests"
endif

# Stands in for the whole templates subdirectory.
# Just copy all of it to dist/templates.
$(TEMPLATE_MANIFEST):
	mkdir -p $(dir $@)
	cp -r $(srcdir)/templates $(DIST_FOLDER)

$(DIST_FOLDER)/images/templates/preview/%.png: $(srcdir)/images/templates/preview/%.png
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/admin-bundle.js: $(COOL_ADMIN_DST) \
	$(INTERMEDIATE_DIR)/admin-src.js
	@NODE_PATH=$(abs_builddir)/node_modules:$(INTERMEDIATE_DIR) $(NODE) node_modules/browserify/bin/cmd.js -g browserify-css $(if $(IS_SEPARATE),--debug,-g uglifyify) -o $@ $(srcdir)/admin/main-admin.js

$(INTERMEDIATE_DIR)/admin-src.js: $(COOL_ADMIN_TS_JS) $(COOL_ADMIN_JS)
	@mkdir -p $(dir $@)
	$(call run_eslint_check,--ignore-pattern $(COOL_ADMIN_TS_JS) $(srcdir)/admin/src)
	$(call run_prettier_check,--debug-check $(srcdir)/admin/src)
	@awk 'FNR == 1 {print ""} 1' $(COOL_ADMIN_TS_JS) $(patsubst %.js,$(srcdir)/%.js,$(COOL_ADMIN_JS)) > $@

$(INTERMEDIATE_DIR)/cool-src.js: tscompile.done $(call prereq_cool) $(COOL_JS_DST) $(abs_top_srcdir)/scripts/unocommands.py $(abs_top_srcdir)/scripts/unoshortcuts.py $(srcdir)/cool-src.js.m4
	@mkdir -p $(dir $@)
	$(QUIET_UNOCOMMANDS) $(abs_top_srcdir)/scripts/unocommands.py --check $(abs_top_srcdir)
	$(QUIET_UNOSHORTCUTS) $(abs_top_srcdir)/scripts/unoshortcuts.py --check $(abs_top_srcdir)
	@printf "Checking for obsolete JS build intermediates in this makefile... "
	@if ! test "z$(COOL_ASSERT_INTERSECT)" = "z"; then echo; echo "Error: please remove obsolete files:"; echo "$(COOL_ASSERT_INTERSECT)"; exit 1; else echo "clean"; fi
	$(call run_eslint_check,$(srcdir)/src $(srcdir)/js)
	$(call run_prettier_check,--debug-check $(srcdir)/src $(srcdir)/js/global.js)
	@echo "Bundling cool..."
	$(call bundle_cool)

$(DIST_FOLDER)/bundle.css: $(call prereq_css)
	@mkdir -p $(dir $@)
	$(QUIET_STYLELINT) $(NODE) node_modules/stylelint/bin/stylelint.js --config $(srcdir)/.stylelintrc.json $(srcdir)/css/*.css --fix
	$(call bundle_css)

$(DIST_FOLDER)/bundle.js: $(INTERMEDIATE_DIR)/cool-src.js $(call prereq_all)
	@mkdir -p $(dir $@)
	$(call bundle_all)

$(DIST_FOLDER)/global.js: $(srcdir)/js/global.js
	@mkdir -p $(dir $@)
	$(call global_file)

$(INTERMEDIATE_DIR)/COOL_JS.m4.jslist: $(COOL_JS_DST)
	@mkdir -p $(INTERMEDIATE_DIR)
	@rm -f $@.tmp
	@for file in $^; do echo "$$file" >> $@.tmp; done
	$(QUIET_CMP) if cmp -s $@.tmp $@; then rm $@.tmp; else mv $@.tmp $@; fi

$(INTERMEDIATE_DIR)/COOL_JS.m4: $(INTERMEDIATE_DIR)/COOL_JS.m4.jslist
	$(foreach file,$(L10N_ALL_JS_ENTRY) $(NODE_MODULES_JS) $(COOL_LIBS_JS) $(patsubst %.ts,%.js,$(patsubst $(DIST_FOLDER)/src/%,src/%,$(COOL_JS_LST))), $(shell echo -n $(file), >> $(INTERMEDIATE_DIR)/COOL_JS.m4.tmp))
	@truncate $(if $(filter Darwin FreeBSD,$(shell uname -s)),-s -1,--size=-1) $(INTERMEDIATE_DIR)/COOL_JS.m4.tmp
	@mv $(INTERMEDIATE_DIR)/COOL_JS.m4.tmp $(INTERMEDIATE_DIR)/COOL_JS.m4

$(DIST_FOLDER)/cool.html: $(srcdir)/html/cool.html.m4 \
	$(COOL_HTML_DST) \
	$(COOL_WELCOME_DST) \
	$(DIST_FOLDER)/bundle.css \
	$(DIST_FOLDER)/global.js \
	$(INTERMEDIATE_DIR)/COOL_JS.m4 \
	| $(DIST_FOLDER)/bundle.js

	$(QUIET_M4) m4 -PE -I $(INTERMEDIATE_DIR) \
	    -DBUNDLE=$(IS_BUNDLE) \
		-DIOSAPP=$(ENABLE_IOSAPP) \
		-DMACOSAPP=$(ENABLE_MACOSAPP) \
		-DQTAPP=$(ENABLE_QTAPP) \
		-DDEBUG=$(ENABLE_DEBUG) \
		-DWINDOWSAPP=$(ENABLE_WINDOWSAPP) \
		-DANDROIDAPP=$(ENABLE_ANDROIDAPP) \
		-DEMSCRIPTENAPP=$(ENABLE_EMSCRIPTENAPP) \
		-DMOBILEAPPNAME="$(APP_NAME)" \
		-DCOOL_CSS="$(subst $(SPACE),$(COMMA),$(COOL_CSS_M4))" \
		-DGLOBAL_JS="$(DIST_FOLDER)/global.js" \
		-DVENDOR="$(VENDOR)" \
		-DINFO_URL="$(INFO_URL)" \
		$(srcdir)/html/cool.html.m4 > $@

.PHONY: adminIntegratorSettings
adminIntegratorSettings:
	@mkdir -p $(DIST_FOLDER)
	$(QUIET_M4) m4 -PE -I $(INTERMEDIATE_DIR) \
		-DIOSAPP=$(ENABLE_IOSAPP) \
		-DMACOSAPP=$(ENABLE_MACOSAPP) \
		-DQTAPP=$(ENABLE_QTAPP) \
		-DDEBUG=$(ENABLE_DEBUG) \
		-DWINDOWSAPP=$(ENABLE_WINDOWSAPP) \
		-DANDROIDAPP=$(ENABLE_ANDROIDAPP) \
		-DEMSCRIPTENAPP=$(ENABLE_EMSCRIPTENAPP) \
		-DMOBILEAPPNAME="$(APP_NAME)" \
		$(srcdir)/admin/adminIntegratorSettings.html.m4 \
		> $(DIST_FOLDER)/adminIntegratorSettings.html

node_modules: package.json node_shrinkpack
	@UV_USE_IO_URING=0 npm ci --offline
	@touch node_modules

update-shrinkpack:
	npm update
	npm shrinkwrap
	npx shrinkpack
	cp npm-shrinkwrap.json npm-shrinkwrap.json.in
	@echo "Done. Remember to commit node_shrinkpack/ and npm-shrinkwrap.json.in"

$(DIST_FOLDER)/device-%.css: $(srcdir)/css/device-%.css
	@mkdir -p $(dir $@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		$(NODE) node_modules/uglifycss/uglifycss $< > $@ ; \
	else \
		ln -sf $(abs_srcdir)/$< $@ ; \
	fi

$(DIST_FOLDER)/plugins/%.js: $(srcdir)/plugins/%.js
	@mkdir -p $(dir $@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		`cp $< $@`; \
	else \
		`ln -sf $(abs_srcdir)/$< $@`; \
	fi

$(DIST_FOLDER)/images/%: $(CUSTOM_ICONS_DIRECTORY)/%
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/images/%: $(srcdir)/images/%
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/%.html: $(srcdir)/html/%.html
	@mkdir -p $(dir $@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		`cp $< $@`; \
	else \
		`ln -sf $(abs_srcdir)/$< $@`; \
	fi

if ENABLE_WELCOME_MESSAGE
$(DIST_FOLDER)/welcome/%: $(srcdir)/welcome/%
	@mkdir -p $(dir $@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		`cp $< $@`; \
	else \
		`ln -sf $(abs_srcdir)/$< $@`; \
	fi
endif

if ENABLE_DEBUG
SourceMap=--allowJs true --sourceMap true --inlineSources true
endif

tscompile.done: $(COOL_JS_SRC) $(COOL_TS_SRC)
	$(QUIET_TSC) $(builddir)/node_modules/typescript/bin/tsc --typeRoots $(builddir)/node_modules/@types --outDir $(DIST_FOLDER)/src --rootDir $(srcdir)/src --checkJs false --project $(srcdir)/tsconfig.json $(SourceMap)
	$(QUIET_TSC) $(builddir)/node_modules/.bin/tsc-strict --typeRoots $(builddir)/node_modules/@types --outDir $(DIST_FOLDER)/src --rootDir $(srcdir)/src --checkJs false --project $(srcdir)/tsconfig.json $(SourceMap)
	@touch $@

$(COOL_TS_JS_DST): tscompile.done
	@:

$(DIST_FOLDER)/src/%.js: $(srcdir)/src/%.js
	@mkdir -p $(dir $@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		`cp $< $@`; \
	else \
		`echo $< $@`; \
		`ln -sf $(abs_srcdir)/$< $@`; \
	fi

$(DIST_FOLDER)/%.js: $(srcdir)/js/%.js
	@mkdir -p $(dir $@)
	@if test -z '$(ENABLE_BROWSERSYNC)'; then \
		`cp $< $@`; \
	else \
		`ln -sf $(abs_srcdir)/$< $@`; \
	fi

$(DIST_FOLDER)/node_modules/%.js: $(builddir)/node_modules/%.js
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/$(JQUERY_LIGHTNESS_IMAGE_PATH)/%.png: $(JQUERY_LIGHTNESS_IMAGE_PATH)/%.png
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/images/%.png: $(JQUERY_LIGHTNESS_IMAGE_PATH)/%.png
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/$(JQUERY_MINIFIED_IMAGE_PATH)/%.png: $(JQUERY_MINIFIED_IMAGE_PATH)/%.png
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/images/%.png: $(JQUERY_MINIFIED_IMAGE_PATH)/%.png
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/l10n/%: $(srcdir)/l10n/%
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/l10n/help-%.json: $(srcdir)/po/help-%.po
	@$(srcdir)/util/po2json.py --quiet $< -o $@

$(DIST_FOLDER)/l10n/ui-%.json: $(srcdir)/po/ui-%.po
	@$(srcdir)/util/po2json.py --quiet $< -o $@

$(DIST_FOLDER)/admin/%: $(srcdir)/admin/%
	@mkdir -p $(dir $@)
	@cp $< $@

$(DIST_FOLDER)/wasm:
	@mkdir -p $(dir $@)
	ln -s @WASM_BUILDDIR@/browser/dist $@

if ENABLE_DEBUG
install-data-hook:
	mkdir -p $(DESTDIR)$(pkgdatadir)/browser; \
	cp -a dist/ $(DESTDIR)$(pkgdatadir)/browser/;
else
EXCLUDES = \
        --exclude='src' \
        --exclude='debug.html' \
        --exclude='framed.doc.html' \
        --exclude='framed.html' \
        --exclude='load.doc.html' \
        --exclude='multidocs.html' \
        --exclude='tsconfig.tsbuildinfo'

if ENABLE_MOBILEAPP
EXCLUDES += --exclude='l10n-all.js'
endif

if !ENABLE_WASM
EXCLUDES += --exclude='wasm.html'
WASM_BROTLI_CMD =
else
WASM_BROTLI_CMD = find $(DESTDIR)$(pkgdatadir)/browser/dist -type f ! -iname "*.png" -exec brotli --force {} \;
endif

install-data-hook:
	mkdir -p $(DESTDIR)$(pkgdatadir)/browser
	rsync -a $(EXCLUDES) dist $(DESTDIR)$(pkgdatadir)/browser/
	mkdir -p $(DESTDIR)$(pkgdatadir)/browser/dist/src/app
	mkdir -p $(DESTDIR)$(pkgdatadir)/browser/dist/src/layer/tile
	cp -a dist/src/layer/tile/CanvasTileUtils.js $(DESTDIR)$(pkgdatadir)/browser/dist/src/layer/tile/CanvasTileUtils.js
	cp -a dist/src/app/TaskWorker.js $(DESTDIR)$(pkgdatadir)/browser/dist/src/app/TaskWorker.js
	$(WASM_BROTLI_CMD)
endif

libs:
	@mkdir -p $(abs_srcdir)/libs
	@cd $(abs_srcdir)/libs \
	&& $(call npm_source,$(NODE_MODULES_SRC))

pot:
	xgettext --from-code=UTF-8 --language=JavaScript --keyword=_ --output=po/templates/cool-ui.pot \
		admin/admin.strings.js \
		admin/src/AdminClusterOverview.js \
		admin/src/AdminSocketAnalytics.js \
		admin/src/AdminSocketBase.js \
		admin/src/AdminSocketOverview.js \
		admin/src/AdminSocketSettings.js \
		admin/src/integrator/AdminIntegratorSettings.ts \
		admin/src/integrator/WordBook.ts \
		admin/src/integrator/Xcu.ts \
		admin/src/Util.js \
		js/global.js \
		src/app/Debug.ts \
		src/app/IconUtil.ts \
		src/app/TableStylesService.ts \
		src/canvas/sections/CommentListSection.ts \
		src/canvas/sections/CommentSection.ts \
		src/canvas/sections/CompareChangesLabelSection.ts \
		src/canvas/sections/ShapeHandlesSection.ts \
		src/canvas/sections/URLPopUpSection.ts \
		src/canvas/sections/TableInsertMarkerSection.ts \
		src/canvas/sections/GenericButtonSection.ts \
		src/canvas/sections/ChartContextButtonSection.ts \
		src/canvas/sections/DiagramButtonSection.ts \
		src/control/jsdialog/Definitions.MenuCommands.ts \
		src/control/ColorPicker.ts \
		src/control/Control.AboutDialog.ts \
		src/control/Control.AddressInputField.ts \
		src/control/Control.AlertDialog.js \
		src/control/Control.BackstageView.ts \
		src/control/Control.Command.js \
		src/control/Control.DocumentNameInput.js \
		src/control/Control.DocumentRepair.js \
		src/control/Control.DownloadProgress.js \
		src/control/Control.ESignature.ts \
		src/control/Control.ESignatureDialog.ts \
		src/control/Control.FormulaBarJSDialog.js \
		src/control/Control.IdleHandler.ts \
		src/control/Control.JSDialog.js \
		src/control/Control.JSDialogBuilder.js \
		src/control/Control.LanguageDialog.js \
		src/control/Control.Mention.ts \
		src/control/Control.Menubar.ts \
		src/control/Control.MobileBottomBar.js \
		src/control/Control.MobileSearchBar.ts \
		src/control/Control.MobileTopBar.ts \
		src/control/Control.MobileWizardBuilder.js \
		src/control/Control.NavigatorPanel.ts \
		src/control/Control.QuickFindPanel.ts \
		src/control/Notebookbar.CalcTableTab.ts \
		src/control/Notebookbar.ImpressAnimationTab.ts \
		src/control/Notebookbar.ImpressTransitionTab.ts \
		src/control/Notebookbar.WriterReferencesTab.ts \
		src/control/Sidebar.FromNotebookbarPanel.ts \
		src/control/Control.Notebookbar.js \
		src/control/Control.NotebookbarBuilder.js \
		src/control/Control.NotebookbarCalc.js \
		src/control/Control.NotebookbarDraw.js \
		src/control/Control.NotebookbarImpress.js \
		src/control/Control.NotebookbarWriter.js \
		src/control/Control.ContextToolbar.ts \
		src/control/Control.PartsPreview.js \
		src/control/Control.PresentationBar.js \
		src/control/Control.SaveState.ts \
		src/control/Control.Scroll.Annotation.js \
		src/control/Control.ServerAuditDialog.ts \
		src/control/Control.SheetsBar.js \
		src/control/Control.StatusBar.js \
		src/control/Control.Tabs.js \
		src/control/Control.Toolbar.js \
		src/control/Control.TopToolbar.js \
		src/control/Control.UIManager.ts \
		src/control/Control.UserList.ts \
		src/control/Control.Zotero.js \
		src/control/HRuler.ts \
		src/control/Parts.js \
		src/control/Permission.js \
		src/control/Signing.js \
		src/control/Toolbar.js \
		src/control/VRuler.ts \
		src/control/PermissionMode.ts \
		src/control/jsdialog/Definitions.Menu.ts \
		src/control/jsdialog/Util.SnackbarController.ts \
		src/control/jsdialog/Widget.ColorPicker.ts \
		src/control/jsdialog/Widget.PageMarginEntry.ts \
		src/control/jsdialog/Widget.PageSizeEntry.ts \
		src/control/jsdialog/Widget.HTMLContent.ts \
		src/control/jsdialog/Widget.MobileBorderSelector.js \
		src/control/jsdialog/Widget.NewSlideLayoutEntry.ts \
		src/control/jsdialog/Widget.RadioButton.ts \
		src/control/jsdialog/Widget.PushButton.ts \
		src/control/jsdialog/Widget.Comment.ts \
		src/control/jsdialog/Widget.SpinField.ts \
		src/control/jsdialog/Widget.FixedTextControl.ts \
		src/control/jsdialog/Widget.SidebarContainers.ts \
		src/control/jsdialog/Widget.TreeView.ts \
		src/app/Socket.ts \
		src/app/TracingEvents.ts \
		src/app/ServerCommand.ts \
		src/docdispatcher.ts \
		src/docstate.ts \
		src/docstatefunctions.js \
		src/dom/NotebookbarAccessibility.js \
		src/errormessages.js \
		src/layer/marker/A11yTextInput.js \
		src/layer/marker/TextInput.js \
		src/layer/tile/CanvasTileLayer.js \
		src/main.js \
		src/map/Clipboard.js \
		src/map/Map.js \
		src/map/handler/Map.Feedback.js \
		src/map/handler/Map.FileInserter.js \
		src/map/handler/Map.Keyboard.js \
		src/map/handler/Map.Print.js \
		src/map/handler/Map.SlideShow.js \
		src/map/handler/Map.VersionBar.js \
		src/map/handler/Map.WOPI.js \
		src/map/handler/Map.Welcome.js \
		src/map/handler/Map.Settings.ts \
		src/slideshow/PauseTimer.ts \
		src/slideshow/PresenterConsole.js \
		src/slideshow/SlideShowPresenter.ts \
		templates/templates.js

	html2po --pot --input=html/cool-help.html --output=po/templates/cool-help.pot --duplicates=merge
	html2po --pot --input=descriptions/google-play.html --output=po/templates/descriptions.pot --duplicates=merge --keepcomments
	html2po --pot --input=welcome/welcome.html --output=po/templates/welcome.pot --duplicates=merge
	$(srcdir)/util/appendwelcome.py po/templates/welcome.pot po/templates/cool-ui.pot
	rm po/templates/welcome.pot
	msguniq -w 78 -t UTF-8 po/templates/cool-ui.pot > po/templates/cool-ui.pot.new; mv po/templates/cool-ui.pot.new po/templates/cool-ui.pot

l10n: pot
	for i in po/ui-*.po; do pot2po --input=po/templates/cool-ui.pot --template=$$i --output=$$i.new; mv $$i.new $$i;done
	for i in po/help-*.po; do pot2po --input=po/templates/cool-help.pot --template=$$i --output=$$i.new; mv $$i.new $$i;done
	for i in po/descriptions-*.po; do pot2po --input=po/templates/descriptions.pot --template=$$i --output=$$i.new; mv $$i.new $$i;done

clean-local:
	@rm -rf $(DIST_FOLDER)
	@rm -rf $(INTERMEDIATE_DIR)
	@rm -rf node_modules
	@rm -f $(abs_srcdir)/jsconfig.json
	@rm -f $(abs_srcdir)/admin/jsconfig.json

CLEANFILES = tscompile.done $(INTERMEDIATE_DIR)/cool-src.js $(DIST_FOLDER)/bundle.css $(DIST_FOLDER)/bundle.js $(INTERMEDIATE_DIR)/cool-src.js admin/adminIntegratorSettings.html

$(abs_srcdir)/jsconfig.json:
	@printf "{\n \
	\"compilerOptions\" : {},\n \
	\"include\" : [\"src/**/*\", \"js/**/*\"],\n \
	\"exclude\" : [\"node_modules\", \"admin/**/*\"]\n}" > $@

$(abs_srcdir)/admin/jsconfig.json:
	@printf "{\n \
	\"compilerOptions\" : {},\n \
	\"include\" : [\"./**/*\"],\n \
	\"exclude\" : [\"node_modules\"]\n}" > $@

jsconfig: $(abs_srcdir)/jsconfig.json $(abs_srcdir)/admin/jsconfig.json

build-tests: $(MOCHA_JS_DIR)/main.js
	@echo "Build of mocha test finished."

eslint: $(COOL_ADMIN_TS_JS)
	$(call run_eslint_check,--max-warnings 0 \
		--ignore-pattern $(COOL_ADMIN_TS_JS) \
		$(srcdir)/src \
		$(srcdir)/js \
		$(srcdir)/admin/src)

prettier:
	$(call run_prettier_check,--check \
		$(srcdir)/src \
		$(srcdir)/js \
		$(srcdir)/admin/src)

prettier-write:
	$(call run_prettier_check,--write \
		$(srcdir)/src \
		$(srcdir)/js \
		$(srcdir)/admin/src)

MOCHA_TEST_LOG = "`pwd`/mocha.log"
DUPLICATION_TEST_LOG = "`pwd`/duplication.log"
check-local: $(MOCHA_JS_DIR)/main.js
	@echo "Running mocha tests..."
	@echo > ${MOCHA_TEST_LOG}
	@if (cd $(srcdir) && PATH="$(abs_builddir)/node_modules/.bin:$$PATH" NODE_PATH="$(abs_builddir)/node_modules" npm test) 2> ${MOCHA_TEST_LOG} 1>&2; then \
		echo "Mocha tests finished successfully. Results in ${MOCHA_TEST_LOG}"; \
	else \
		cat ${MOCHA_TEST_LOG}; \
		exit 1; \
	fi

	@echo > ${DUPLICATION_TEST_LOG}
	$(QUIET_JSCPCD) if (cd $(srcdir) && PATH="$(abs_builddir)/node_modules/.bin:$$PATH" NODE_PATH="$(abs_builddir)/node_modules" npm run duplication) 2> ${DUPLICATION_TEST_LOG} 1>&2; then \
		echo "Code duplication tests finished successfully."; \
	else \
		cat ${DUPLICATION_TEST_LOG}; \
		exit 1; \
	fi

# the delay (in milliseconds) between each emulated key stroke for each view
typing_speed=100
# The duration of the typing (in milliseconds)
typing_duration=60000
# By default 6 views are spawned in the document.
single_view="false"
# --inspect flag for the first view. Ideally should be used with single_view=true
node_inspect="false"
# Record tile stats and dump it into the file "test/tilestats.txt"
# Pipe it with | grep "tilestats"
record_stats="true"
perf-test:
	@echo "Running perf tests..."
	@mkdir -p $(abs_top_builddir)/test/data
	@if [ ! -f $(abs_top_srcdir)/test/data/perf-test-edit.odt -o "`md5sum <$(abs_top_srcdir)/test/data/perf-test.odt`" != "`md5sum <$(abs_top_srcdir)/test/data/perf-test-edit.odt`" ]; then \
		echo 'Making fresh copy of the perf-test-edit.odt document'; \
		cp $(abs_top_srcdir)/test/data/perf-test.odt $(abs_top_builddir)/test/data/perf-test-edit.odt; \
	fi
	$(NODE) $(abs_srcdir)/test/bootstrap.js $(abs_top_builddir) $(abs_srcdir) $(typing_speed) $(single_view) $(typing_duration) $(node_inspect) ${record_stats}
# for now - unwinding ts and js deps for different targets is a real pain.
.NOTPARALLEL:
