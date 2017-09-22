/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>

#include "check.hxx"
#include "plugin.hxx"

namespace {

bool startsWith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}

class BadStatics
    : public clang::RecursiveASTVisitor<BadStatics>
    , public loplugin::Plugin
{

public:
    explicit BadStatics(InstantiationData const& rData) : Plugin(rData) {}

    void run() override {
        if (compiler.getLangOpts().CPlusPlus) { // no non-trivial dtors in C
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
        }
    }

    static std::pair<bool, std::vector<FieldDecl const*>> isBadStaticType(
            QualType const& rpType, std::vector<FieldDecl const*> & chain,
            std::vector<QualType> const& rParents)
    {
        QualType pt;
#if 0
        if (rpType->isAnyPointerType()) {
            pt = rpType->getPointeeType();
        } else if (auto at = rpType->getAsArrayTypeUnsafe()) {
            pt = at->getElementType();
        } else if (auto rt = rpType->getAs<ReferenceType>()) {
            pt = rt->getPointeeType();
        }
#endif
        if (auto at = rpType->getAsArrayTypeUnsafe()) {
            pt = at->getElementType();
        }
        if (!pt.isNull()) {
            QualType const pPointee(pt.getUnqualifiedType().getCanonicalType());
            auto const iter(std::find(rParents.begin(), rParents.end(), pPointee));
            if (iter == rParents.end())
            {
                std::vector<QualType> copy(rParents);
                copy.push_back(rpType.getUnqualifiedType().getCanonicalType());
                return isBadStaticType(pt, chain, copy);
            } else {
                return std::make_pair(false, std::vector<FieldDecl const*>());
            }
        }
        RecordType const*const pRecordType(rpType->getAs<RecordType>());
        if (!pRecordType) {
            return std::make_pair(false, std::vector<FieldDecl const*>());
        }
        auto const type = loplugin::TypeCheck(rpType);
        if (   type.Class("Image").GlobalNamespace()
            || type.Class("Bitmap").GlobalNamespace()
            || type.Class("BitmapEx").GlobalNamespace()
            || type.Class("VclPtr").GlobalNamespace()
           )
        {
            return std::make_pair(true, chain);
        }
        if (type.Class("array").StdNamespace()
            || type.Class("unique_ptr").StdNamespace()
            || type.Class("shared_ptr").StdNamespace()
            || type.Class("Reference").Namespace("rtl").GlobalNamespace()
            || type.Class("Sequence").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace()
            || type.Class("deque").StdNamespace()
            || type.Class("forward_list").StdNamespace()
            || type.Class("initializer_list").StdNamespace()
            || type.Class("list").StdNamespace()
            || type.Class("multiset").StdNamespace()
            || type.Class("set").StdNamespace()
            || type.Class("unordered_multiset").StdNamespace()
            || type.Class("unordered_set").StdNamespace()
            || type.Class("vector").StdNamespace())
        {
            std::vector<QualType> copy(rParents);
            copy.push_back(rpType.getUnqualifiedType().getCanonicalType());
            auto ctsd = dyn_cast<ClassTemplateSpecializationDecl>(
                pRecordType->getDecl());
            assert(ctsd != nullptr);
            auto const & args = ctsd->getTemplateArgs();
            assert(args.size() >= 1);
            return isBadStaticType(args.get(0).getAsType(), chain, copy);
        }
        if (type.Class("map").StdNamespace()
            || type.Class("multimap").StdNamespace()
            || type.Class("unordered_map").StdNamespace()
            || type.Class("unordered_multimap").StdNamespace())
        {
            std::vector<QualType> copy(rParents);
            copy.push_back(rpType.getUnqualifiedType().getCanonicalType());
            auto ctsd = dyn_cast<ClassTemplateSpecializationDecl>(
                pRecordType->getDecl());
            assert(ctsd != nullptr);
            auto const & args = ctsd->getTemplateArgs();
            assert(args.size() >= 2);
            auto ret = isBadStaticType(args.get(0).getAsType(), chain, copy);
            if (ret.first) {
                return ret;
            }
            return isBadStaticType(args.get(1).getAsType(), chain, copy);
        }
        RecordDecl const*const pDefinition(pRecordType->getDecl()->getDefinition());
        if (!pDefinition) { // maybe no definition if it's a pointer/reference
            return std::make_pair(false, std::vector<FieldDecl const*>());
        }
        if (   type.Class("DeleteOnDeinit").Namespace("vcl").GlobalNamespace()
            || type.Class("DeleteUnoReferenceOnDeinit").Namespace("vcl").GlobalNamespace()
            || type.Class("OString").Namespace("rtl").GlobalNamespace()
            || type.Class("OUString").Namespace("rtl").GlobalNamespace()
            || type.Class("OStringBuffer").Namespace("rtl").GlobalNamespace()
            || type.Class("OUStringBuffer").Namespace("rtl").GlobalNamespace()
            || type.Class("Mutex").Namespace("osl").GlobalNamespace()
            || type.Class("Condition").Namespace("osl").GlobalNamespace()
            || type.Class("WeakReference").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace() // not owning
            || type.Class("weak_ptr").StdNamespace() // not owning
            || type.Class("basic_string").StdNamespace()
            || type.Class("TestPlugInDefaultImpl").Namespace("CppUnit").GlobalNamespace() // CppUnit
            || type.Class("TestNamer").Namespace("CppUnit").GlobalNamespace() // CppUnit
            || type.Class("LanguageTag").GlobalNamespace() // probably okay currently, unless lt_tag_unref does something stupid
            || type.Class("Thread").Namespace("salhelper").GlobalNamespace() // salhelper::Thread dtor hidden
            || type.Class("Type").Namespace("uno").Namespace("star").Namespace("sun").Namespace("com").GlobalNamespace() // FIXME is this ok?
            || type.Class("OImplementationId").Namespace("cppu").GlobalNamespace() // just a Sequence
            || type.Class("B2DHomMatrix").Namespace("basegfx").GlobalNamespace()
//            || type.Class("Allocator").Class("FileHandle_Impl").GlobalNamespace() // can't clear that?
//            || type.Class("Allocator").GlobalNamespace() // can't clear that?
            || type.Class("UnicodeToTextConverter_Impl").AnonymousNamespace().GlobalNamespace() // dtor is no-op
            || type.Class("TextToUnicodeConverter_Impl").AnonymousNamespace().GlobalNamespace() // dtor is no-op
            || type.Class("ImplWallpaper").GlobalNamespace() // very odd static instance here
            || type.Class("Application").GlobalNamespace() // numerous odd subclasses in vclmain::createApplication()
            || type.Class("DemoMtfApp").GlobalNamespace() // one of these Application with own VclPtr
            || type.Class("GlobalObject").GlobalNamespace() // sal unit test
           )
        {
            return std::make_pair(false, std::vector<FieldDecl const*>());
        }
        std::vector<QualType> copy(rParents);
        copy.push_back(rpType.getUnqualifiedType().getCanonicalType());
        CXXRecordDecl const*const pDecl(dyn_cast<CXXRecordDecl>(pDefinition));
        assert(pDecl);
        // not hasNonTrivialDestructor() because if it's a default one
        // we want to recursively check the members, if they're all good it's ok
        if (/*pDecl->hasNonTrivialDestructor() &&*/ pDecl->hasUserDeclaredDestructor())
        {
            auto const*const pDtor(pDecl->getDestructor());
            if (auto const*const pDtorDef = pDtor->getDefinition())
            {
                auto const pBody(pDtorDef->getBody());
                auto const pCompound(dyn_cast_or_null<CompoundStmt>(pBody));
                if (pCompound && pCompound->size() > 0) {
                    return std::make_pair(true, chain);
                }
                // else: empty dtor impl.: check the members
            }
            else // assume the worst of unavailable dtors
            {
                if (!pDtor->isPure())
                {
                    return std::make_pair(true, chain);
                }
                // else: empty dtor impl.: check the members
            }
        }
        for (auto it = pDecl->field_begin(); it != pDecl->field_end(); ++it) {
            chain.push_back(*it);
            auto const ret(isBadStaticType((*it)->getType(), chain, copy));
            chain.pop_back();
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->bases_begin(); it != pDecl->bases_end(); ++it) {
            auto const ret(isBadStaticType((*it).getType(), chain, copy));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->vbases_begin(); it != pDecl->vbases_end(); ++it) {
            auto const ret(isBadStaticType((*it).getType(), chain, copy));
            if (ret.first) {
                return ret;
            }
        }
        return std::make_pair(false, std::vector<FieldDecl const*>());
    }

    bool VisitVarDecl(VarDecl const*const pVarDecl)
    {
        if (ignoreLocation(pVarDecl)) {
            return true;
        }

        if (pVarDecl->hasGlobalStorage()
            && pVarDecl->isThisDeclarationADefinition())
        {
            auto const name(pVarDecl->getName());
            if (   name == "g_pI18NStatusInstance" // I18NStatus::free()
                || name == "s_pPreviousView" // not a owning pointer
                || name == "s_pDefCollapsed" // SvImpLBox::~SvImpLBox()
                || name == "s_pDefExpanded"  // SvImpLBox::~SvImpLBox()
                || name == "g_pDDSource" // SvTreeListBox::dispose()
                || name == "g_pDDTarget" // SvTreeListBox::dispose()
                || name == "g_pSfxApplication" // SfxApplication::~SfxApplication()
                || name == "s_SidebarResourceManagerInstance" // ResourceManager::disposeDecks()
                || name == "s_pGallery" // this is not entirely clear but apparently the GalleryThemeCacheEntry are deleted by GalleryBrowser2::SelectTheme() or GalleryBrowser2::dispose()
                || name == "s_ExtMgr" // TheExtensionManager::disposing()
                || name == "s_pDocLockedInsertingLinks" // not owning
                || name == "s_pVout" // FrameFinit()
                || name == "s_pPaintQueue" // SwPaintQueue::Remove()
                || name == "gProp" // only owned (VclPtr) member cleared again
                || name == "g_pColumnCacheLastTabFrame" // not owning
                || name == "g_pColumnCacheLastCellFrame" // not owning
                || name == "g_pRowCacheLastTabFrame" // not owning
                || name == "g_pRowCacheLastCellFrame" // not owning
                || name == "g_OszCtrl" // SwCrsrOszControl::Exit()
                || name == "g_pSpellIter" // SwEditShell::SpellEnd()
                || name == "g_pConvIter" // SwEditShell::SpellEnd()
                || name == "g_pHyphIter" // SwEditShell::HyphEnd()
                || name == "pFieldEditEngine" // ScGlobal::Clear()
                || name == "xDrawClipDocShellRef" // ScGlobal::Clear()
                || name == "s_ImageTree"
                    // ImageTree::get(), ImageTree::shutDown()
                || name == "s_pMouseFrame"
                    // vcl/osx/salframeview.mm, mouseEntered/Exited, not owning
                || name == "pCurrentMenuBar"
                    // vcl/osx/salmenu.cxx, AquaSalMenu::set/unsetMainMenu, not
                    // owning
                || name == "s_pCaptureFrame" // vcl/osx/salframe.cxx, not owning
                || name == "pBlink"
                    // sw/source/core/text/blink.cxx, _TextFinit()
                || name == "s_pIconCache"
                    // sd/source/ui/tools/IconCache.cxx, leaked
                || name == "maInstanceMap"
                    // sd/source/ui/framework/tools/FrameworkHelper.cxx, would
                    // leak ViewShellBase* keys if that map is not empty at exit
                || name == "theAddInAsyncTbl"
                    // sc/source/core/tool/adiasync.cxx, would leak
                    // ScAddInAsync* keys if that set is not empty at exit
                || name == "g_aWindowList"
                    //vcl/unx/gtk/a11y/atkutil.cxx, asserted empty at exit
                || name == "aLogger" // FormulaLogger& FormulaLogger::get() in sc/source/core/tool/formulalogger.cxx
                || name == "m_aUncommitedRegistrations" // sw/source/uibase/dbui/dbmgr.cxx
                || (loplugin::DeclCheck(pVarDecl).Var("aAllListeners")
                    .Class("ScAddInListener").GlobalNamespace()) // not owning
                || name == "g_pDefaultImplFontCharMap" // only trivial members
                || startsWith(name, "autoRegisterRegistry__") // CppUnit
                || name == "g_aBufferAllocator" // can't clear that?
               ) // these variables appear unproblematic
            {
                return true;
            }
            // these two are fairly harmless because they're both empty objects
            if (   name == "s_xEmptyController" // svx/source/fmcomp/gridcell.cxx
                || name == "xCell"              // svx/source/table/svdotable.cxx
               )
            {
                return true;
            }
            // ignore pointers, nothing happens to them on shutdown
            QualType const pCanonical(pVarDecl->getType().getUnqualifiedType().getCanonicalType());
            if (pCanonical->isPointerType()) {
                return true;
            }
            std::vector<FieldDecl const*> pad;
            auto const ret(isBadStaticType(pVarDecl->getType(), pad,
                            std::vector<QualType>()));
            if (ret.first) {
                report(DiagnosticsEngine::Warning,
                        "bad static variable causes crash on shutdown",
                        pVarDecl->getLocation())
                    << pVarDecl->getSourceRange();
                if (!isUnitTestMode())
                {
                    for (auto i: ret.second) {
                        report(DiagnosticsEngine::Note,
                                "... due to this member of %0",
                                i->getLocation())
                            << i->getParent() << i->getSourceRange();
                    }
                }
            }
        }

        return true;
    }

};

loplugin::Plugin::Registration<BadStatics> X("badstatics");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
