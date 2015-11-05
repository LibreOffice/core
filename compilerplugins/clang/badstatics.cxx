/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "plugin.hxx"

// FIXME debug
#include <iostream>
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

    static std::pair<bool, FieldDecl const*> isBadStaticType(
            QualType const& rType, FieldDecl const*const pCurrentFieldDecl)
    {
        QualType const pCanonical(rType.getUnqualifiedType().getCanonicalType());
        RecordType const*const pRecordType(pCanonical->getAs<RecordType>());
        if (!pRecordType) {
            return std::make_pair(false, nullptr);
        }
        auto const type(pCanonical.getAsString());
        std::cerr << "XXX type: " << type << std::endl;
        if (   type == "class Image"
            || type == "class Bitmap"
            || type == "class BitmapEx"
           )
        {
            return std::make_pair(true, pCurrentFieldDecl);
        }
        // FIXME no definition if its a "class std::weak_ptr<class SvtUserOptions::Impl>"
//        RecordDecl const*const pRDecl(pRecordType->getDecl()->getDefinition());
//fail        assert(pRDecl);
        RecordDecl const*const pCDecl(dyn_cast<RecordDecl>(pRecordType->getDecl()->getCanonicalDecl()));
//        assert(pCDecl);
        CXXRecordDecl const*const pDecl1(dyn_cast<CXXRecordDecl>(pCDecl));
        ClassTemplateDecl const* pTemplateDecl(pDecl1->getDescribedClassTemplate());
        CXXRecordDecl const*pDecl;
        if (pTemplateDecl)
            pDecl = pTemplateDecl->getTemplatedDecl()->getDefinition();
        else
            pDecl = pDecl1;

        std::cerr << "XXX " << __LINE__ << std::endl;
        assert(pDecl);
        for (auto it = pDecl->field_begin(); it != pDecl->field_end(); ++it) {
            std::cerr << "XXX " << __LINE__ << std::endl;
            auto const ret(isBadStaticType((*it)->getType(), *it));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->bases_begin(); it != pDecl->bases_end(); ++it) {
            std::cerr << "XXX " << __LINE__ << std::endl;
            auto const ret(isBadStaticType((*it).getType(), pCurrentFieldDecl));
            if (ret.first) {
                return ret;
            }
        }
        for (auto it = pDecl->vbases_begin(); it != pDecl->vbases_end(); ++it) {
            std::cerr << "XXX " << __LINE__ << std::endl;
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

        if (pVarDecl->hasGlobalStorage()) {
            auto const ret(isBadStaticType(pVarDecl->getType(), nullptr));
            if (ret.first) {
                report(DiagnosticsEngine::Warning,
                        "bad static variable causes crash on shutdown",
                        pVarDecl->getLocation())
                    << pVarDecl->getSourceRange();
                if (ret.second != nullptr) {
                    report(DiagnosticsEngine::Warning,
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
