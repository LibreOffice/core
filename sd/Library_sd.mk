# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,sd))

$(eval $(call gb_SdiTarget_SdiTarget,sd/sdi/sdslots,sd/sdi/sdslots))

$(eval $(call gb_SdiTarget_set_include,sd/sdi/sdslots,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/sdi \
    $$(INCLUDE) \
))

$(eval $(call gb_SdiTarget_SdiTarget,sd/sdi/sdgslots,sd/sdi/sdgslots))

$(eval $(call gb_SdiTarget_set_include,sd/sdi/sdgslots,\
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/sdi \
    $$(INCLUDE) \
))

$(eval $(call gb_Library_add_sdi_headers,sd,\
    sd/sdi/sdgslots \
    sd/sdi/sdslots \
))

$(eval $(call gb_Library_use_packages,sd,\
	animations_inc \
	bluez_bluetooth_inc \
))

$(eval $(call gb_Library_set_include,sd,\
    $$(INCLUDE) \
    -I$(SRCDIR)/sd/inc \
    -I$(SRCDIR)/sd/source/ui/inc \
    -I$(SRCDIR)/sd/source/ui/slidesorter/inc \
    -I$(WORKDIR)/SdiTarget/sd/sdi \
))

$(eval $(call gb_Library_add_defs,sd,\
    -DSD_DLLIMPLEMENTATION \
))

ifneq ($(strip $(dbg_anim_log)$(DBG_ANIM_LOG)),)
$(eval $(call gb_Library_add_defs,sd,\
    $$(DEFS) \
    -DDBG_ANIM_LOG \
))
endif

$(eval $(call gb_Library_use_sdk_api,sd))

$(eval $(call gb_Library_use_libraries,sd,\
    avmedia \
    basegfx \
    canvastools \
    comphelper \
    cppcanvas \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    i18nisolang1 \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sfx \
    sot \
    svl \
    svt \
    svxcore \
    svx \
    tk \
    tl \
    ucbhelper \
    utl \
    vcl \
    xmlsecurity \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_use_externals,sd,\
 libxml2 \
 dbus \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Library_use_system_win32_libs,sd,\
	ws2_32 \
))
endif

$(eval $(call gb_Library_set_componentfile,sd,sd/util/sd))

$(eval $(call gb_Library_add_exception_objects,sd,\
    sd/source/core/CustomAnimationCloner \
    sd/source/core/CustomAnimationEffect \
    sd/source/core/CustomAnimationPreset \
    sd/source/core/EffectMigration \
    sd/source/core/PageListWatcher \
    sd/source/core/TransitionPreset \
    sd/source/core/anminfo \
    sd/source/core/annotations/Annotation \
    sd/source/core/annotations/AnnotationEnumeration \
    sd/source/core/cusshow \
    sd/source/core/drawdoc \
    sd/source/core/drawdoc2 \
    sd/source/core/drawdoc3 \
    sd/source/core/drawdoc4 \
    sd/source/core/drawdoc_animations \
    sd/source/core/pglink \
    sd/source/core/sdattr \
    sd/source/core/sdiocmpt \
    sd/source/core/sdobjfac \
    sd/source/core/sdpage \
    sd/source/core/sdpage2 \
    sd/source/core/sdpage_animations \
    sd/source/core/shapelist \
    sd/source/core/stlfamily \
    sd/source/core/stlpool \
    sd/source/core/stlsheet \
    sd/source/core/text/textapi \
    sd/source/core/typemap \
    sd/source/core/undo/undofactory \
    sd/source/core/undo/undomanager \
    sd/source/core/undo/undoobjects \
    sd/source/core/undoanim \
    sd/source/filter/cgm/sdcgmfilter \
    sd/source/filter/grf/sdgrffilter \
    sd/source/filter/html/HtmlOptionsDialog \
    sd/source/filter/html/buttonset \
    sd/source/filter/html/htmlex \
    sd/source/filter/html/sdhtmlfilter \
    sd/source/filter/sdfilter \
    sd/source/filter/sdpptwrp \
    sd/source/filter/xml/sdtransform \
    sd/source/filter/xml/sdxmlwrp \
    sd/source/helper/simplereferencecomponent \
    sd/source/ui/accessibility/AccessibleDocumentViewBase \
    sd/source/ui/accessibility/AccessibleDrawDocumentView \
    sd/source/ui/accessibility/AccessibleOutlineEditSource \
    sd/source/ui/accessibility/AccessibleOutlineView \
    sd/source/ui/accessibility/AccessiblePageShape \
    sd/source/ui/accessibility/AccessiblePresentationGraphicShape \
    sd/source/ui/accessibility/AccessiblePresentationOLEShape \
    sd/source/ui/accessibility/AccessiblePresentationShape \
    sd/source/ui/accessibility/AccessibleScrollPanel \
    sd/source/ui/accessibility/AccessibleSlideSorterObject \
    sd/source/ui/accessibility/AccessibleSlideSorterView \
    sd/source/ui/accessibility/AccessibleTreeNode \
    sd/source/ui/accessibility/AccessibleViewForwarder \
    sd/source/ui/accessibility/SdShapeTypes \
    sd/source/ui/animations/CustomAnimationCreateDialog \
    sd/source/ui/animations/CustomAnimationDialog \
    sd/source/ui/animations/CustomAnimationList \
    sd/source/ui/animations/CustomAnimationPane \
    sd/source/ui/animations/DialogListBox \
    sd/source/ui/animations/STLPropertySet \
    sd/source/ui/animations/SlideTransitionPane \
    sd/source/ui/animations/motionpathtag \
    sd/source/ui/annotations/annotationmanager \
    sd/source/ui/annotations/annotationtag \
    sd/source/ui/annotations/annotationwindow \
    sd/source/ui/app/sddll \
    sd/source/ui/app/sddll1 \
    sd/source/ui/app/sddll2 \
    sd/source/ui/app/sdmod \
    sd/source/ui/app/sdmod1 \
    sd/source/ui/app/sdmod2 \
    sd/source/ui/app/sdpopup \
    sd/source/ui/app/sdresid \
    sd/source/ui/app/sdxfer \
    sd/source/ui/app/tbxww \
    sd/source/ui/app/tmplctrl \
    sd/source/ui/controller/slidelayoutcontroller \
    sd/source/ui/dlg/AnimationChildWindow \
    sd/source/ui/dlg/LayerDialogChildWindow \
    sd/source/ui/dlg/LayerDialogContent \
    sd/source/ui/dlg/LayerTabBar \
    sd/source/ui/dlg/NavigatorChildWindow \
    sd/source/ui/dlg/PaneChildWindows \
    sd/source/ui/dlg/PaneDockingWindow \
    sd/source/ui/dlg/PaneShells \
    sd/source/ui/dlg/SpellDialogChildWindow \
    sd/source/ui/dlg/TemplateScanner \
    sd/source/ui/dlg/animobjs \
    sd/source/ui/dlg/assclass \
    sd/source/ui/dlg/diactrl \
    sd/source/ui/dlg/dlgctrls \
    sd/source/ui/dlg/docprev \
    sd/source/ui/dlg/filedlg \
    sd/source/ui/dlg/gluectrl \
    sd/source/ui/dlg/ins_paste \
    sd/source/ui/dlg/layeroptionsdlg \
    sd/source/ui/dlg/navigatr \
    sd/source/ui/dlg/sdabstdlg \
    sd/source/ui/dlg/sdtreelb \
    sd/source/ui/dlg/unchss \
    sd/source/ui/docshell/docshel2 \
    sd/source/ui/docshell/docshel3 \
    sd/source/ui/docshell/docshel4 \
    sd/source/ui/docshell/docshell \
    sd/source/ui/docshell/grdocsh \
    sd/source/ui/docshell/sdclient \
    sd/source/ui/framework/configuration/ChangeRequestQueue \
    sd/source/ui/framework/configuration/ChangeRequestQueueProcessor \
    sd/source/ui/framework/configuration/Configuration \
    sd/source/ui/framework/configuration/ConfigurationClassifier \
    sd/source/ui/framework/configuration/ConfigurationController \
    sd/source/ui/framework/configuration/ConfigurationControllerBroadcaster \
    sd/source/ui/framework/configuration/ConfigurationControllerResourceManager \
    sd/source/ui/framework/configuration/ConfigurationTracer \
    sd/source/ui/framework/configuration/ConfigurationUpdater \
    sd/source/ui/framework/configuration/GenericConfigurationChangeRequest \
    sd/source/ui/framework/configuration/ResourceFactoryManager \
    sd/source/ui/framework/configuration/ResourceId \
    sd/source/ui/framework/configuration/UpdateRequest \
    sd/source/ui/framework/factories/BasicPaneFactory \
    sd/source/ui/framework/factories/BasicToolBarFactory \
    sd/source/ui/framework/factories/BasicViewFactory \
    sd/source/ui/framework/factories/ChildWindowPane \
    sd/source/ui/framework/factories/FrameWindowPane \
    sd/source/ui/framework/factories/FullScreenPane \
    sd/source/ui/framework/factories/Pane \
    sd/source/ui/framework/factories/PresentationFactory \
    sd/source/ui/framework/factories/TaskPanelFactory \
    sd/source/ui/framework/factories/ViewShellWrapper \
    sd/source/ui/framework/module/CenterViewFocusModule \
    sd/source/ui/framework/module/DrawModule \
    sd/source/ui/framework/module/ImpressModule \
    sd/source/ui/framework/module/ModuleController \
    sd/source/ui/framework/module/PresentationModule \
    sd/source/ui/framework/module/ResourceManager \
    sd/source/ui/framework/module/ShellStackGuard \
    sd/source/ui/framework/module/SlideSorterModule \
    sd/source/ui/framework/module/ToolBarModule \
    sd/source/ui/framework/module/ToolPanelModule \
    sd/source/ui/framework/module/ViewTabBarModule \
    sd/source/ui/framework/tools/FrameworkHelper \
    sd/source/ui/func/bulmaper \
    sd/source/ui/func/fuarea \
    sd/source/ui/func/fubullet \
    sd/source/ui/func/fuchar \
    sd/source/ui/func/fucon3d \
    sd/source/ui/func/fuconarc \
    sd/source/ui/func/fuconbez \
    sd/source/ui/func/fuconcs \
    sd/source/ui/func/fuconnct \
    sd/source/ui/func/fuconrec \
    sd/source/ui/func/fuconstr \
    sd/source/ui/func/fuconuno \
    sd/source/ui/func/fucopy \
    sd/source/ui/func/fucushow \
    sd/source/ui/func/fudraw \
    sd/source/ui/func/fudspord \
    sd/source/ui/func/fuediglu \
    sd/source/ui/func/fuexpand \
    sd/source/ui/func/fuformatpaintbrush \
    sd/source/ui/func/fuhhconv \
    sd/source/ui/func/fuinsert \
    sd/source/ui/func/fuinsfil \
    sd/source/ui/func/fuline \
    sd/source/ui/func/fulinend \
    sd/source/ui/func/fulink \
    sd/source/ui/func/fumeasur \
    sd/source/ui/func/fumorph \
    sd/source/ui/func/fuoaprms \
    sd/source/ui/func/fuolbull \
    sd/source/ui/func/fuoltext \
    sd/source/ui/func/fuoutl \
    sd/source/ui/func/fupage \
    sd/source/ui/func/fuparagr \
    sd/source/ui/func/fupoor \
    sd/source/ui/func/fuprlout \
    sd/source/ui/func/fuprobjs \
    sd/source/ui/func/fuscale \
    sd/source/ui/func/fusearch \
    sd/source/ui/func/fusel \
    sd/source/ui/func/fusldlg \
    sd/source/ui/func/fusnapln \
    sd/source/ui/func/fusumry \
    sd/source/ui/func/futempl \
    sd/source/ui/func/futhes \
    sd/source/ui/func/futransf \
    sd/source/ui/func/futxtatt \
    sd/source/ui/func/fuvect \
    sd/source/ui/func/fuzoom \
    sd/source/ui/func/sdundo \
    sd/source/ui/func/sdundogr \
    sd/source/ui/func/smarttag \
    sd/source/ui/func/undoback \
    sd/source/ui/func/undoheaderfooter \
    sd/source/ui/func/undolayer \
    sd/source/ui/func/undopage \
    sd/source/ui/func/unmovss \
    sd/source/ui/func/unoaprms \
    sd/source/ui/func/unprlout \
    sd/source/ui/presenter/CanvasUpdateRequester \
    sd/source/ui/presenter/PresenterCanvas \
    sd/source/ui/presenter/PresenterHelper \
    sd/source/ui/presenter/PresenterPreviewCache \
    sd/source/ui/presenter/PresenterTextView \
    sd/source/ui/presenter/SlideRenderer \
    sd/source/ui/slideshow/PaneHider \
    sd/source/ui/slideshow/SlideShowRestarter \
    sd/source/ui/slideshow/showwin \
    sd/source/ui/slideshow/slideshow \
    sd/source/ui/slideshow/slideshowimpl \
    sd/source/ui/slideshow/slideshowviewimpl \
    sd/source/ui/slidesorter/cache/SlsBitmapCache \
    sd/source/ui/slidesorter/cache/SlsBitmapCompressor \
    sd/source/ui/slidesorter/cache/SlsBitmapFactory \
    sd/source/ui/slidesorter/cache/SlsCacheCompactor \
    sd/source/ui/slidesorter/cache/SlsCacheConfiguration \
    sd/source/ui/slidesorter/cache/SlsGenericPageCache \
    sd/source/ui/slidesorter/cache/SlsPageCache \
    sd/source/ui/slidesorter/cache/SlsPageCacheManager \
    sd/source/ui/slidesorter/cache/SlsQueueProcessor \
    sd/source/ui/slidesorter/cache/SlsRequestFactory \
    sd/source/ui/slidesorter/cache/SlsRequestQueue \
    sd/source/ui/slidesorter/controller/SlideSorterController \
    sd/source/ui/slidesorter/controller/SlsAnimationFunction \
    sd/source/ui/slidesorter/controller/SlsAnimator \
    sd/source/ui/slidesorter/controller/SlsClipboard \
    sd/source/ui/slidesorter/controller/SlsCurrentSlideManager \
    sd/source/ui/slidesorter/controller/SlsDragAndDropContext \
    sd/source/ui/slidesorter/controller/SlsFocusManager \
    sd/source/ui/slidesorter/controller/SlsInsertionIndicatorHandler \
    sd/source/ui/slidesorter/controller/SlsListener \
    sd/source/ui/slidesorter/controller/SlsPageSelector \
    sd/source/ui/slidesorter/controller/SlsProperties \
    sd/source/ui/slidesorter/controller/SlsScrollBarManager \
    sd/source/ui/slidesorter/controller/SlsSelectionFunction \
    sd/source/ui/slidesorter/controller/SlsSelectionManager \
    sd/source/ui/slidesorter/controller/SlsSelectionObserver \
    sd/source/ui/slidesorter/controller/SlsSlotManager \
    sd/source/ui/slidesorter/controller/SlsTransferable \
    sd/source/ui/slidesorter/controller/SlsVisibleAreaManager \
    sd/source/ui/slidesorter/model/SlideSorterModel \
    sd/source/ui/slidesorter/model/SlsPageDescriptor \
    sd/source/ui/slidesorter/model/SlsPageEnumeration \
    sd/source/ui/slidesorter/model/SlsPageEnumerationProvider \
    sd/source/ui/slidesorter/model/SlsVisualState \
    sd/source/ui/slidesorter/shell/SlideSorter \
    sd/source/ui/slidesorter/shell/SlideSorterService \
    sd/source/ui/slidesorter/shell/SlideSorterViewShell \
    sd/source/ui/slidesorter/view/SlideSorterView \
    sd/source/ui/slidesorter/view/SlsFontProvider \
    sd/source/ui/slidesorter/view/SlsFramePainter \
    sd/source/ui/slidesorter/view/SlsInsertAnimator \
    sd/source/ui/slidesorter/view/SlsInsertionIndicatorOverlay \
    sd/source/ui/slidesorter/view/SlsLayeredDevice \
    sd/source/ui/slidesorter/view/SlsLayouter \
    sd/source/ui/slidesorter/view/SlsPageObjectLayouter \
    sd/source/ui/slidesorter/view/SlsPageObjectPainter \
    sd/source/ui/slidesorter/view/SlsTheme \
    sd/source/ui/slidesorter/view/SlsToolTip \
    sd/source/ui/slidesorter/view/SlsViewCacheContext \
    sd/source/ui/table/TableDesignPane \
    sd/source/ui/table/tablefunction \
    sd/source/ui/table/tableobjectbar \
    sd/source/ui/toolpanel/ControlContainer \
    sd/source/ui/toolpanel/LayoutMenu \
    sd/source/ui/toolpanel/ScrollPanel \
    sd/source/ui/toolpanel/SlideSorterCacheDisplay \
    sd/source/ui/toolpanel/SubToolPanel \
    sd/source/ui/toolpanel/TaskPaneControlFactory \
    sd/source/ui/toolpanel/TaskPaneFocusManager \
    sd/source/ui/toolpanel/TaskPaneShellManager \
    sd/source/ui/toolpanel/TaskPaneTreeNode \
    sd/source/ui/toolpanel/TitleBar \
    sd/source/ui/toolpanel/TitledControl \
    sd/source/ui/toolpanel/ToolPanel \
    sd/source/ui/toolpanel/ToolPanelFactory \
    sd/source/ui/toolpanel/ToolPanelUIElement \
    sd/source/ui/toolpanel/ToolPanelViewShell \
    sd/source/ui/toolpanel/controls/AllMasterPagesSelector \
    sd/source/ui/toolpanel/controls/CurrentMasterPagesSelector \
    sd/source/ui/toolpanel/controls/CustomAnimationPanel \
    sd/source/ui/toolpanel/controls/DocumentHelper \
    sd/source/ui/toolpanel/controls/MasterPageContainer \
    sd/source/ui/toolpanel/controls/MasterPageContainerFiller \
    sd/source/ui/toolpanel/controls/MasterPageContainerProviders \
    sd/source/ui/toolpanel/controls/MasterPageContainerQueue \
    sd/source/ui/toolpanel/controls/MasterPageDescriptor \
    sd/source/ui/toolpanel/controls/MasterPageObserver \
    sd/source/ui/toolpanel/controls/MasterPagesPanel \
    sd/source/ui/toolpanel/controls/MasterPagesSelector \
    sd/source/ui/toolpanel/controls/PreviewValueSet \
    sd/source/ui/toolpanel/controls/RecentMasterPagesSelector \
    sd/source/ui/toolpanel/controls/RecentlyUsedMasterPages \
    sd/source/ui/toolpanel/controls/SlideTransitionPanel \
    sd/source/ui/toolpanel/controls/TableDesignPanel \
    sd/source/ui/tools/AsynchronousCall \
    sd/source/ui/tools/ConfigurationAccess \
    sd/source/ui/tools/EventMultiplexer \
    sd/source/ui/tools/IconCache \
    sd/source/ui/tools/IdleDetection \
    sd/source/ui/tools/PreviewRenderer \
    sd/source/ui/tools/PropertySet \
    sd/source/ui/tools/SdGlobalResourceContainer \
    sd/source/ui/tools/SlotStateListener \
    sd/source/ui/tools/TimerBasedTaskExecution \
    sd/source/ui/unoidl/DrawController \
    sd/source/ui/unoidl/SdUnoDrawView \
    sd/source/ui/unoidl/SdUnoOutlineView \
    sd/source/ui/unoidl/SdUnoSlideView \
    sd/source/ui/unoidl/UnoDocumentSettings \
    sd/source/ui/unoidl/facreg \
    sd/source/ui/unoidl/randomnode \
    sd/source/ui/unoidl/unocpres \
    sd/source/ui/unoidl/unodoc \
    sd/source/ui/unoidl/unolayer \
    sd/source/ui/unoidl/unomodel \
    sd/source/ui/unoidl/unomodule \
    sd/source/ui/unoidl/unoobj \
    sd/source/ui/unoidl/unopage \
    sd/source/ui/unoidl/unopback \
    sd/source/ui/unoidl/unopool \
    sd/source/ui/unoidl/unosrch \
    sd/source/ui/view/DocumentRenderer \
    sd/source/ui/view/FormShellManager \
    sd/source/ui/view/GraphicObjectBar \
    sd/source/ui/view/GraphicViewShellBase \
    sd/source/ui/view/ImpressViewShellBase \
    sd/source/ui/view/MediaObjectBar \
    sd/source/ui/view/OutlineViewShellBase \
    sd/source/ui/view/Outliner \
    sd/source/ui/view/OutlinerIterator \
    sd/source/ui/view/PresentationViewShellBase \
    sd/source/ui/view/SlideSorterViewShellBase \
    sd/source/ui/view/ToolBarManager \
    sd/source/ui/view/ViewClipboard \
    sd/source/ui/view/ViewShellBase \
    sd/source/ui/view/ViewShellHint \
    sd/source/ui/view/ViewShellImplementation \
    sd/source/ui/view/ViewShellManager \
    sd/source/ui/view/ViewTabBar \
    sd/source/ui/view/WindowUpdater \
    sd/source/ui/view/clview \
    sd/source/ui/view/drawview \
    sd/source/ui/view/drbezob \
    sd/source/ui/view/drtxtob \
    sd/source/ui/view/drtxtob1 \
    sd/source/ui/view/drviews1 \
    sd/source/ui/view/drviews2 \
    sd/source/ui/view/drviews3 \
    sd/source/ui/view/drviews4 \
    sd/source/ui/view/drviews5 \
    sd/source/ui/view/drviews6 \
    sd/source/ui/view/drviews7 \
    sd/source/ui/view/drviews8 \
    sd/source/ui/view/drviews9 \
    sd/source/ui/view/drviewsa \
    sd/source/ui/view/drviewsb \
    sd/source/ui/view/drviewsc \
    sd/source/ui/view/drviewsd \
    sd/source/ui/view/drviewse \
    sd/source/ui/view/drviewsf \
    sd/source/ui/view/drviewsg \
    sd/source/ui/view/drviewsh \
    sd/source/ui/view/drviewsi \
    sd/source/ui/view/drviewsj \
    sd/source/ui/view/drvwshrg \
    sd/source/ui/view/frmview \
    sd/source/ui/view/grviewsh \
    sd/source/ui/view/outlnvs2 \
    sd/source/ui/view/outlnvsh \
    sd/source/ui/view/outlview \
    sd/source/ui/view/presvish \
    sd/source/ui/view/sdruler \
    sd/source/ui/view/sdview \
    sd/source/ui/view/sdview2 \
    sd/source/ui/view/sdview3 \
    sd/source/ui/view/sdview4 \
    sd/source/ui/view/sdview5 \
    sd/source/ui/view/sdwindow \
    sd/source/ui/view/tabcontr \
    sd/source/ui/view/unmodpg \
    sd/source/ui/view/viewoverlaymanager \
    sd/source/ui/view/viewshe2 \
    sd/source/ui/view/viewshe3 \
    sd/source/ui/view/viewshel \
    sd/source/ui/view/zoomlist \
))

ifeq ($(ENABLE_SDREMOTE),YES)
$(eval $(call gb_Library_add_exception_objects,sd,\
    sd/source/ui/remotecontrol/BufferedStreamSocket \
    sd/source/ui/remotecontrol/Communicator \
    sd/source/ui/remotecontrol/DiscoveryService \
    sd/source/ui/remotecontrol/ImagePreparer \
    sd/source/ui/remotecontrol/Server \
    sd/source/ui/remotecontrol/Receiver \
    sd/source/ui/remotecontrol/Listener \
    sd/source/ui/remotecontrol/Transmitter \
))

$(eval $(call gb_Library_add_defs,sd,\
    -DENABLE_SDREMOTE \
))

ifeq ($(ENABLE_BLUETOOTH),YES)
$(eval $(call gb_Library_add_exception_objects,sd,\
    sd/source/ui/remotecontrol/BluetoothServer \
))

$(eval $(call gb_Library_add_defs,sd,\
    -DENABLE_BLUETOOTH \
))
endif

endif

ifeq ($(strip $(GUI)),WNT)
$(eval $(call gb_Library_add_cxxobjects,sd,\
    sd/source/ui/app/optsitem \
    sd/source/ui/func/futext \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,sd,\
    sd/source/ui/app/optsitem \
    sd/source/ui/func/futext \
))
endif

ifeq ($(strip $(COM)),GCC)
$(eval $(call gb_Library_add_cxxobjects,sd,\
    sd/source/ui/unoidl/unowcntr \
    , $(gb_COMPILERNOOPTFLAGS) $(gb_LinkTarget_EXCEPTIONFLAGS) \
))
else
$(eval $(call gb_Library_add_exception_objects,sd,\
    sd/source/ui/unoidl/unowcntr \
))
endif

# vim: set noet sw=4 ts=4:
