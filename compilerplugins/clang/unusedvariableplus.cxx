/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */

#ifndef LO_CLANG_SHARED_PLUGINS

#include "plugin.hxx"
#include <unordered_set>

/*
 * Very aggressive unused variable checker, we allowlist types that are known
 * good when unused.
*/

namespace
{
static bool startswith(const std::string& rStr, const char* pSubStr)
{
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class UnusedVariablePlus : public loplugin::FilteringPlugin<UnusedVariablePlus>
{
public:
    explicit UnusedVariablePlus(loplugin::InstantiationData const& data)
        : FilteringPlugin(data)
    {
    }

    virtual bool preRun() override
    {
        std::string fn(handler.getMainFileName());
        loplugin::normalizeDotDotInFilePath(fn);
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/canvas/workben/"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/source/uipreviewer/previewer.cxx"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/vcl/workben/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/vcl/qa/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/vcl/backendtest/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/lotuswordpro/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/salhelper/qa/"))
            return false;
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/sal/qa/"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/idl/source/prj/svidl.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sot/source/unoolestorage/xolesimplestorage.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/core/tool/interpr7.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/ui/vba/vbaapplication.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/doc/doccomp.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/core/swg/SwXMLTextBlocks.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sw/source/filter/ww8/wrtw8esh.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR
                                     "/shell/source/sessioninstall/SyncDbusSessionHelper.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svl/qa/unit/items/test_IndexedStyleSheets.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sd/qa/unit/export-tests-ooxml2.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/qa/unit/subsequent_export-test.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/vcl/skia/SkiaHelper.cxx"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/svx/source/dialog/dlgctrl.cxx"))
            return false;

        // clang has a bug here, with vars in destructing assignments
        if (loplugin::hasPathnamePrefix(fn, SRCDIR "/binaryurp/"))
            return false;
        if (loplugin::isSamePathname(fn, SRCDIR "/sc/source/filter/excel/xestring.cxx"))
            return false;
        return true;
    }

    virtual void run() override
    {
        if (preRun())
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    }

    bool VisitVarDecl(const VarDecl* var)
    {
        if (ignoreLocation(var))
            return true;
        if (var->isReferenced() || var->isUsed())
            return true;
        if (var->isDefinedOutsideFunctionOrMethod())
            return true;
        if (var->isExceptionVariable()) // not interesting
            return true;

        auto type = var->getType().getCanonicalType().getUnqualifiedType();
        auto typeName = type.getAsString();
        if (typeName.compare(0, 7, "struct ") == 0)
            typeName = typeName.substr(7);
        if (typeName.compare(0, 6, "class ") == 0)
            typeName = typeName.substr(6);
        if (typeName.compare(0, 2, "::") == 0)
            typeName = typeName.substr(2);
        if (typeName.compare(0, 23, "(anonymous namespace)::") == 0)
            typeName = typeName.substr(23);
        static std::unordered_set<std::string> ignoreClassNamesSet{
            "apphelper::NegativeGuard<class osl::Mutex>",
            "avmedia::gstreamer::(anonymous namespace)::FlagGuard",
            "BoolEnv_Impl",
            "BoolResetter",
            "boost::io::basic_ios_all_saver<char, struct std::char_traits<char> >",
            "BorderLinesGuard",
            "BroadcastRecalcOnRefMoveGuard",
            "CacheLockGuard",
            "cc_reset",
            "ViewCallback",
            "chart::ControllerLockGuard",
            "chart::ControllerLockGuardUNO",
            "chart::HiddenUndoContext",
            "chart::sidebar::(anonymous namespace)::PreventUpdate",
            "chart::TimerTriggeredControllerLock",
            "chart::TrueGuard",
            "ClearableClipRegion",
            "osl::ClearableGuard<class osl::Mutex>",
            "rptui::ColorChanger",
            "SortRefUpdateSetter",
            "SortRefNoUpdateSetter",
            "com::sun::star::uno::ContextLayer",
            "comphelper::FlagGuard",
            "comphelper::FlagRestorationGuard",
            "comphelper::ValueRestorationGuard<_Bool>",
            "comphelper::(anonymous namespace)::ProfileZone",
            "comphelper::ProfileZone",
            "comphelper::ORelease<class osl::Mutex>",
            "comphelper::OStreamSection",
            "ConfigurationController::Lock",
            "SortTypeSetter",
            "SkiaZone",
            "MockMetadatable",
            "connectivity::(anonymous namespace)::ForbidQueryName",
            "connectivity::calc::OCalcConnection::ODocHolder",
            "connectivity::writer::OWriterConnection::ODocHolder",
            "connectivity::jdbc::ContextClassLoaderScope",
            "CurrShell",
            "dbaccess::ModifyLock",
            "dbaccess::NameChangeNotifier",
            "dbaccess::OFilteredContainer::EnsureReset",
            "dbaccess::OQuery::OAutoActionReset",
            "dbaccess::OQueryContainer::OAutoActionReset",
            "dbaccess::(anonymous namespace)::LayoutManagerLock",
            "dbaccess::(anonymous namespace)::LockModifiable",
            "dbaccess::(anonymous namespace)::OExecuteImpl",
            "dbaccess::(anonymous namespace)::PreserveVisualAreaSize",
            "dbaccess::(anonymous namespace)::ProtectFlag",
            "dbaui::BrowserViewStatusDisplay",
            "dbaui::SbaXDataBrowserController::FormErrorHelper",
            "dbaui::(anonymous namespace)::SelectionGuard",
            "dbaxml::(anonymous namespace)::FocusWindowWaitGuard",
            "DBG_Model",
            "DeactivateUpdateMode",
            "desktop::Desktop",
            "desktop::(anonymous namespace)::ConditionSetGuard",
            "desktop::(anonymous namespace)::RefClearGuard<class "
            "com::sun::star::uno::Reference<class com::sun::star::frame::XSynchronousDispatch> "
            ">",
            "DetachCurrentThread",
            "DialogReleaseGuard",
            "DisableCallbackAction",
            "DisableCallbacks",
            "DisableGetPivotData",
            "DispatchMutexLock_Impl",
            "DocTemplLocker_Impl",
            "DocumentSettingsGuard",
            "DocxTableExportContext",
            "dp_misc::AbortChannel::Chain",
            "dp_misc::ProgressLevel",
            "E3DModifySceneSnapRectUpdater",
            "E3dObjFactory",
            "ErrorHdlResetter",
            "EscherExAtom",
            "EscherExContainer",
            "ExportDataSaveRestore",
            "ExtensionRemoveGuard",
            "frm::(anonymous namespace)::FieldChangeNotifier",
            "frm::(anonymous namespace)::DocumentModifyGuard",
            "FieldDeletionModify",
            "FieldDeletionListener",
            "FileHandle_Impl::Guard",
            "FlowFrameJoinLockGuard",
            "FmXFormShell::SuspendPropertyTracking",
            "FocusWindowWaitCursor",
            "formula::(anonymous namespace)::OpCodeList",
            "formula::(anonymous namespace)::FormulaCompilerRecursionGuard",
            "FontCacheGuard",
            "FontLockGuard",
            "FormatLevel",
            "FormulaGrammarSwitch",
            "framework::DocumentUndoGuard",
            "framework::ShareGuard",
            "framework::TransactionGuard",
            "framework::(anonymous namespace)::QuickstartSuppressor",
            "GalApp",
            "GalleryProgress",
            "GlibThreadDefaultMainContextScope",
            "Guard",
            "HandleResetAttrAtTextNode",
            "HandleSetAttrAtTextNode",
            "HelpParser",
            "HtmlExport",
            "HTMLSaveData",
            "IMapCompat",
            "jni_uno::JLocalAutoRef",
            "LoadMediumGuard",
            "LockGuard",
            "MacroInterpretIncrementer",
            "MailMergeExecuteFinalizer",
            "ModifyBlocker_Impl",
            "MutexRelease",
            "MutexType",
            "NewTextListsHelper",
            "OAutoRegistration",
            "rptui::GeometryHandler::OBlocker",
            "rptui::OXReportControllerObserver::OEnvLock",
            "oglcanvas::TransformationPreserver",
            "io_acceptor::(anonymous namespace)::BeingInAccept",
            "desktop::LibLibreOffice_Impl",
            "ToolbarUnoDispatcher",
            "ooo::vba::excel::(anonymous namespace)::PasteCellsWarningReseter",
            "oox::drawingml::(anonymous namespace)::ActionLockGuard",
            "oox::dump::IndentGuard",
            "oox::dump::ItemGuard",
            "oox::dump::MultiItemsGuard",
            "oox::dump::TableGuard",
            "OpenCLInitialZone",
            "OpenCLZone",
            "OpenGLVCLContextZone",
            "OpenGLZone",
            "osl::MutexGuard",
            "rptui::OXUndoEnvironment::OUndoMode",
            "rptui::OXUndoEnvironment::OUndoEnvLock",
            "PaMIntoCursorShellRing",
            "ParserCleanup",
            "BorderTest",
            "pcr::ComposedUIAutoFireGuard",
            "writerperfect::(anonymous namespace)::PositionHolder",
            "pq_sdbc_driver::DisposeGuard",
            "PropertyChangeNotifier",
            "ProtectFormulaGroupContext",
            "PreventUpdate",
            "pyuno::PyThreadAttach",
            "pyuno::PyThreadDetach",
            "pyuno_loader::(anonymous namespace)::PythonInit",
            "RecursionCounter",
            "RefGuard",
            "icu_65::RegexMatcher",
            "RestoreMapMode",
            "Runner",
            "pyuno::Runtime",
            "salhelper::ConditionModifier",
            "salhelper::ConditionWaiter",
            "SaveRunState",
            "SbiExpression",
            "sc::AutoCalcSwitch",
            "sc::DelayFormulaGroupingSwitch",
            "sc::IdleSwitch",
            "sc::UndoSwitch",
            "ScBulkBroadcast",
            "ScChartLockGuard",
            "ScCompiler",
            "ScDocument::NumFmtMergeHandler",
            "ScDocShellModificator",
            "ScDocShell::PrepareSaveGuard",
            "ScExternalRefManager::ApiGuard",
            "ScFormulaGroupCycleCheckGuard",
            "ScFormulaGroupDependencyComputeGuard",
            "SchedulerGuard",
            "ScMutationDisable",
            "ScNoteCaptionCreator",
            "ScValidationRegisteredDlg",
            "ScopedAntialiasing",
            "ScRefreshTimerProtector",
            "ScWaitCursorOff",
            "ScXMLImport::MutexGuard",
            "SdIOCompat",
            "sd::slidesorter::controller::FocusManager::FocusHider",
            "sd::slidesorter::controller::SlideSorterController::ModelChangeLock",
            "sd::slidesorter::controller::PageSelector::BroadcastLock",
            "sd::slidesorter::view::SlideSorterView::DrawLock",
            "sd::slidesorter::controller::PageSelector::UpdateLock",
            "sd::ViewShellManager::Implementation::UpdateLock",
            "sd::(anonymous namespace)::LockUI",
            "sd::ToolBarManager::UpdateLock",
            "sd::ViewShellManager::UpdateLock",
            "sd::OutlineViewPageChangesGuard",
            "sd::framework::ConfigurationController::Lock",
            "sd::slidesorter::controller::VisibleAreaManager::TemporaryDisabler",
            "sd::slidesorter::controller::SelectionObserver::Context",
            "sd::ToolBarManager::Implementation::UpdateLockImplementation",
            "sd::OutlineViewModelChangeGuard",
            "sd::slidesorter::controller::(anonymous "
            "namespace)::TemporarySlideTrackingDeactivator",
            "sd::ModifyGuard",
            "sd::OutlineViewModelChangeGuard",
            "setFastDocumentHandlerGuard",
            "SfxErrorContext",
            "SfxObjectShellLock",
            "SfxProgress",
            "SfxSaveGuard",
            "SfxStack",
            "ShellMoveCursor",
            "SkAutoCanvasRestore",
            "SolarMutexGuard",
            "SolarMutexReleaser",
            "StackHack",
            "std::scoped_lock<class std::mutex>",
            "std::unique_ptr<class com::sun::star::uno::ContextLayer, struct "
            "std::default_delete<class com::sun::star::uno::ContextLayer> >",
            "std::unique_ptr<class weld::WaitObject, struct std::default_delete<class "
            "weld::WaitObject> >",
            "std::unique_ptr<class ClearableClipRegion, struct o3tl::default_delete<class "
            "ClearableClipRegion> >",
            "std::unique_ptr<class SwDocShell::LockAllViewsGuard, struct "
            "std::default_delete<class "
            "SwDocShell::LockAllViewsGuard> >",
            "std::unique_ptr<class SwSaveFootnoteHeight, struct std::default_delete<class "
            "SwSaveFootnoteHeight> >",
            "std::unique_ptr<class SwModelTestBase::Resetter, struct std::default_delete<class "
            "SwModelTestBase::Resetter> >",
            "StreamExceptionsEnabler",
            "SvAddressParser_Impl",
            "svl::undo::impl::LockGuard",
            "svt::table::(anonymous namespace)::SuppressCursor",
            "svx::(anonymous namespace)::FontSwitch",
            "SvXMLElementExport",
            "svxform::(anonymous namespace)::QuitGuard",
            "sw::DrawUndoGuard",
            "sw::UndoGuard",
            "sw::GroupUndoGuard",
            "sw::(anonymous namespace)::CursorGuard",
            "SwActContext",
            "SwAutoFormat",
            "SwCallLink",
            "SwContentNotify",
            "SwCursorSaveState",
            "SwCSS1OutMode",
            "SwDataChanged",
            "SwDigitModeModifier",
            "SwDrawViewSave",
            "SwDropSave",
            "SwEnhancedPDFExportHelper",
            "SwEnterLeave",
            "SwFieldSlot",
            "SwFilterOptions",
            "SwFntAccess",
            "SwFontSave",
            "SwFootnoteSave",
            "SwFrameDeleteGuard",
            "SwModelTestBase::Resetter",
            "std::unique_ptr<class ScTokenArray, struct std::default_delete<class "
            "ScTokenArray> >", // ScCompiler::CompileString has nasty semantics
            "Resetter",
            "SwFrameSwapper",
            "SwFlyNotify",
            "SwForbidFollowFormat",
            "SwHandleAnchorNodeChg",
            "SwHookOut",
            "SwImplShellAction",
            "SwKeepConversionDirectionStateContext",
            "SwLayIdle",
            "SwLayNotify",
            "SwLayoutModeModifier",
            "SwMvContext",
            "SwNotifyAccAboutInvalidTextSelections",
            "SwObjPositioningInProgress",
            "SwParaSelection",
            "SwPauseThreadStarting",
            "SwPosNotify",
            "SwRedlineItr", // ???
            "SwRegHistory",
            "SwSaveFootnoteHeight",
            "SwSaveSetLRUOfst",
            "SwStyleBase_Impl::ItemSetOverrider",
            "SwSwapIfNotSwapped",
            "SwSwapIfSwapped",
            "SwTableNumFormatMerge",
            "SwTaggedPDFHelper",
            "SwTestFormat",
            "SwTextCursorSave",
            "SwTextSlot",
            "SwTrnsfrActionAndUndo",
            "SwWait",
            "SwXDispatchProviderInterceptor::DispatchMutexLock_Impl",
            "TableWait",
            "TargetStateControl_Impl",
            "TempErrorHandler",
            "TemporaryCellGroupMaker",
            "TemporaryRedlineUpdater",
            "TextFrameLockGuard",
            "TimerContext",
            "TimerTriggeredControllerLock",
            "ToggleSaveToModule",
            "toolkit::(anonymous namespace)::ResetFlagOnExit",
            "TravelSuspension",
            "TreeUpdateSwitch",
            "rptui::UndoContext",
            "rptui::UndoSuppressor",
            "UndoRedoRedlineGuard",
            "UnoActionRemoveContext",
            "UnoActionContext",
            "UpdateFontsGuard",
            "utl::CloseableComponent",
            "utl::DisposableComponent",
            "ValueCounter_Impl",
            "VclListenerLock",
            "vclcanvas::tools::OutDevStateKeeper",
            "vcl::PaintBufferGuard",
            "vcl::RoadmapWizardTravelSuspension",
            "vcl::ScopedAntialiasing",
            "vcl::WizardTravelSuspension",
            "VerbExecutionControllerGuard",
            "VersionCompatRead",
            "VersionCompatWrite",
            "SlideShowImpl::WaitSymbolLock",
            "weld::WaitObject",
            "writerfilter::ooxml::(anonymous namespace)::StatusIndicatorGuard",
            "WriterSpecificAutoFormatBlock",
            "xmloff::OOfficeFormsExport",
            "xmlscript::(anonymous namespace)::MGuard",
            "XMLTextCharStyleNamesElementExport",
        };
        if (ignoreClassNamesSet.find(typeName) != ignoreClassNamesSet.end())
            return true;
        if (startswith(typeName, "comphelper::ScopeGuard<"))
            return true;
        if (startswith(typeName, "comphelper::ValueRestorationGuard<"))
            return true;
        if (startswith(typeName, "osl::Guard<"))
            return true;
        if (startswith(typeName, "dbaui::OMultiInstanceAutoRegistration<"))
            return true;
        if (startswith(typeName, "pcr::OAutoRegistration<"))
            return true;

        if (var->getIdentifier())
        {
            auto name = var->getName();
            if (name == "aBroadcastGuard" || name == "aDeleteRef" || name == "aGuard"
                || name == "aGuard2" || name == "aHoldSelf" || name == "aKeepDoc"
                || name == "aAutoRegistration" || name == "aLoadContentIfExists"
                || name == "aOwnRef" || name == "createImpl" || name == "flyHolder"
                || name == "guard" || name == "ensureDelete" || name == "s_xTerminateListener"
                || name == "pThis" || name == "pOldViewShell" || name == "self"
                || name == "xDocStor" || name == "xDeleteUponLeaving" || name == "xDeleteRef"
                || name == "xHoldAlive" || name == "xHolder" || name == "xHoldRefForMethodAlive"
                || name == "xLock" || name == "xMutexGuard" || name == "xKeepAlive"
                || name == "xKeepContentHolderAlive" || name == "xKeepDocAlive"
                || name == "xKeepMeAlive" || name == "xKeepProviderAlive"
                || name == "xOperationHold" || name == "xPreventDelete" || name == "xSelf"
                || name == "xSelfHold" || name == "xTempHold" || name == "xDisposeAfterNewOne"
                || name == "xThis" || name == "xThisPackage" || name == "xTriggerInit"
                || name == "xLifeCycle" || name == "xAnotherLifeCycle")
                return true;
        }

        if (isa<ParmVarDecl>(var))
            return true;

        if (typeName.find("Reference") != std::string::npos && var->getInit())
        {
            if (auto cxxConstructExpr
                = dyn_cast<CXXConstructExpr>(var->getInit()->IgnoreImplicit()))
                if (cxxConstructExpr->getNumArgs() == 2)
                    if (auto param1 = dyn_cast<DeclRefExpr>(cxxConstructExpr->getArg(1)))
                        if (auto enumDecl = dyn_cast<EnumConstantDecl>(param1->getDecl()))
                            if (enumDecl->getName() == "UNO_QUERY_THROW"
                                || enumDecl->getName() == "UNO_SET_THROW")
                                return true;
        }

        report(DiagnosticsEngine::Warning, "unused variable %0 of type %1", var->getLocation())
            << var->getDeclName() << typeName;

        return true;
    }
};

loplugin::Plugin::Registration<UnusedVariablePlus> unusedvariableplus("unusedvariableplus", false);

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
