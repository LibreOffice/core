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

    /*static*/ std::pair<bool, FieldDecl const*> isBadStaticType(
            QualType const& rType, FieldDecl const*const pCurrentFieldDecl)
    {
        QualType const pCanonical(rType.getUnqualifiedType().getCanonicalType());
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
#if (__clang_major__ == 3 && __clang_minor__ > 5) || __clang_major__ > 3
        // TODO not sure if it works with clang 3.6, trunk is known to work
        assert(pDefinition);
#else
        if (!pDefinition) {
            // no definition if its a template, e.g. "class std::weak_ptr<class SvtUserOptions::Impl>"
            report(DiagnosticsEngine::Remark,
                    "old clang version unable to find definition of this class",
                    pRecordType->getDecl()->getLocation())
                << pRecordType->getDecl()->getSourceRange();
            return std::make_pair(false, nullptr);
        }
#endif
        CXXRecordDecl const*const pDecl(dyn_cast<CXXRecordDecl>(pDefinition));
        assert(pDecl);
        for (auto it = pDecl->field_begin(); it != pDecl->field_end(); ++it) {
            auto const ret(isBadStaticType((*it)->getType(), *it));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->bases_begin(); it != pDecl->bases_end(); ++it) {
            auto const ret(isBadStaticType((*it).getType(), pCurrentFieldDecl));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->vbases_begin(); it != pDecl->vbases_end(); ++it) {
            auto const ret(isBadStaticType((*it).getType(), pCurrentFieldDecl));
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
            auto const ret(isBadStaticType(pVarDecl->getType(), nullptr));
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
