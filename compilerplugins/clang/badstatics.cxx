/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "check.hxx"
#include "plugin.hxx"

namespace {

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
        QualType const pCanonical(rpType.getUnqualifiedType().getCanonicalType());
        if (pCanonical->isPointerType() || pCanonical->isReferenceType()) {
            QualType const pPointee(pCanonical->getPointeeType().getUnqualifiedType().getCanonicalType());
            auto const iter(std::find(rParents.begin(), rParents.end(), pPointee));
            if (iter == rParents.end())
            {
                std::vector<QualType> copy(rParents);
                copy.push_back(pCanonical);
                return isBadStaticType(pPointee, chain, copy);
            } else {
                return std::make_pair(false, std::vector<FieldDecl const*>());
            }
        }
        RecordType const*const pRecordType(pCanonical->getAs<RecordType>());
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
        RecordDecl const*const pDefinition(pRecordType->getDecl()->getDefinition());
        if (!pDefinition) { // maybe no definition if it's a pointer/reference
            return std::make_pair(false, std::vector<FieldDecl const*>());
        }
        if (   type.Class("DeleteOnDeinit").Namespace("vcl").GlobalNamespace()
            || type.Class("weak_ptr").StdNamespace() // not owning
            || type.Class("ImplWallpaper").GlobalNamespace() // very odd static instance here
            || type.Class("Application").GlobalNamespace() // numerous odd subclasses in vclmain::createApplication()
            || type.Class("DemoMtfApp").GlobalNamespace() // one of these Application with own VclPtr
           )
        {
            return std::make_pair(false, std::vector<FieldDecl const*>());
        }
        std::vector<QualType> copy(rParents);
        copy.push_back(pCanonical);
        CXXRecordDecl const*const pDecl(dyn_cast<CXXRecordDecl>(pDefinition));
        assert(pDecl);
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
                || name == "s_ImplImageTree"
                    // ImplImageTree::get(), ImplImageTree::shutDown()
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
                for (auto i: ret.second) {
                    report(DiagnosticsEngine::Note,
                            "... due to this member of %0",
                            i->getLocation())
                        << i->getParent() << i->getSourceRange();
                }
            }
        }

        return true;
    }

};

loplugin::Plugin::Registration<BadStatics> X("badstatics");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
