/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

namespace {

static bool startsWith(const std::string& s, const char* other)
{
    return s.compare(0, strlen(other), other) == 0;
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

    static std::pair<bool, FieldDecl const*> isBadStaticType(
            QualType const& rpType, FieldDecl const*const pCurrentFieldDecl,
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
                return isBadStaticType(pPointee, pCurrentFieldDecl, copy);
            } else {
                return std::make_pair(false, nullptr);
            }
        }
        RecordType const*const pRecordType(pCanonical->getAs<RecordType>());
        if (!pRecordType) {
            return std::make_pair(false, nullptr);
        }
        auto const type(pCanonical.getAsString());
        if (   type == "class Image"
            || type == "class Bitmap"
            || type == "class BitmapEx"
           )
        {
            return std::make_pair(true, pCurrentFieldDecl);
        }
        RecordDecl const*const pDefinition(pRecordType->getDecl()->getDefinition());
        if (!pDefinition) { // maybe no definition if it's a pointer/reference
            return std::make_pair(false, nullptr);
        }
        if (   startsWith(type, "class vcl::DeleteOnDeinit")
            || startsWith(type, "class std::weak_ptr") // not owning
            || type == "class ImplWallpaper" // very odd static instance here
            || type == "class Application" // numerous odd subclasses in vclmain::createApplication()
            || type == "class DemoMtfApp" // one of these Application with own VclPtr
           )
        {
            return std::make_pair(false, nullptr);
        }
        std::vector<QualType> copy(rParents);
        copy.push_back(pCanonical);
        CXXRecordDecl const*const pDecl(dyn_cast<CXXRecordDecl>(pDefinition));
        assert(pDecl);
        for (auto it = pDecl->field_begin(); it != pDecl->field_end(); ++it) {
            auto const ret(isBadStaticType((*it)->getType(), *it, copy));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->bases_begin(); it != pDecl->bases_end(); ++it) {
            auto const ret(isBadStaticType((*it).getType(), pCurrentFieldDecl, copy));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->vbases_begin(); it != pDecl->vbases_end(); ++it) {
            auto const ret(isBadStaticType((*it).getType(), pCurrentFieldDecl, copy));
            if (ret.first) {
                return ret;
            }
        }
        return std::make_pair(false, nullptr);
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
                || name == "pStaticThesSubMenu" // wtf is this nonsense
                || name == "s_pGallery" // this is not entirely clear but apparently the GalleryThemeCacheEntry are deleted by GalleryBrowser2::SelectTheme() or GalleryBrowser2::dispose()
                || name == "s_ExtMgr" // TheExtensionManager::disposing()
                || name == "s_pDocLockedInsertingLinks" // not owning
                || name == "s_pVout" // _FrmFinit()
                || name == "s_pPaintQueue" // SwPaintQueue::Remove()
                || name == "gProp" // only owned (VclPtr) member cleared again
                || name == "g_pColumnCacheLastTabFrm" // not owning
                || name == "g_pColumnCacheLastCellFrm" // not owning
                || name == "g_pRowCacheLastTabFrm" // not owning
                || name == "g_pRowCacheLastCellFrm" // not owning
                || name == "g_OszCtrl" // SwCrsrOszControl::Exit()
                || name == "g_pSpellIter" // SwEditShell::SpellEnd()
                || name == "g_pConvIter" // SwEditShell::SpellEnd()
                || name == "g_pHyphIter" // SwEditShell::HyphEnd()
                || name == "pFieldEditEngine" // ScGlobal::Clear()
                || name == "xDrawClipDocShellRef" // ScGlobal::Clear()
               ) // these variables appear unproblematic
            {
                return true;
            }
            auto const ret(isBadStaticType(pVarDecl->getType(), nullptr,
                            std::vector<QualType>()));
            if (ret.first) {
                report(DiagnosticsEngine::Warning,
                        "bad static variable causes crash on shutdown",
                        pVarDecl->getLocation())
                    << pVarDecl->getSourceRange();
                if (ret.second != nullptr) {
                    report(DiagnosticsEngine::Remark,
                            "... due to this member",
                            ret.second->getLocation())
                        << ret.second->getSourceRange();
                }
            }
        }

        return true;
    }

};

loplugin::Plugin::Registration<BadStatics> X("badstatics");

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
