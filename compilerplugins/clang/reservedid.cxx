/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <algorithm>
#include <cassert>
#include <limits>
#include <set>
#include <string>

#include "clang/AST/Attr.h"

#include "plugin.hxx"

namespace {

bool isJniFunction(NamedDecl const * decl) {
    auto const fdecl = dyn_cast<FunctionDecl>(decl);
    if (fdecl == nullptr
        || !(decl->getDeclContext()->getDeclKind() == Decl::LinkageSpec
             && decl->getDeclContext()->getParent()->isTranslationUnit())
        || !fdecl->isExternC())
    {
        return false;
    }
    auto const id = decl->getIdentifier();
    return id != nullptr && id->getName().startswith("Java_");
}

class ReservedId:
    public RecursiveASTVisitor<ReservedId>, public loplugin::Plugin
{
public:
    explicit ReservedId(InstantiationData const & data): Plugin(data) {}

    void run() override;

    bool VisitNamedDecl(NamedDecl const * decl);

private:
    enum class Kind {
        Ok, DoubleUnderscore, UnderscoreUppercase, UnderscoreLowercase };

    Kind determineKind(llvm::StringRef const & id);

    bool isInLokIncludeFile(SourceLocation spellingLocation) const;

    bool isApi(NamedDecl const * decl);
};

void ReservedId::run() {
    //TODO: Rules for C?
    if (TraverseDecl(compiler.getASTContext().getTranslationUnitDecl())
        && compiler.hasPreprocessor())
    {
#if CLANG_VERSION >= 30700
        auto & prep = compiler.getPreprocessor();
        for (auto const & m: prep.macros(false)) {
            auto id = m.first->getName();
            if (determineKind(id) != Kind::Ok && id != "_GLIBCXX_CDTOR_CALLABI"
                && id != "_POSIX_SOURCE"
                && id != "__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES"
                && id != "__ORCUS_STATIC_LIB" && id != "__USE_GNU"
                && id != "_MAX_PATH") //TODO: win32
            {
                auto d = prep.getLocalMacroDirectiveHistory(m.first);
                for (;;) {
                    if (d->getKind() == MacroDirective::MD_Define) {
                        auto loc = d->getLocation();
                        if (loc.isValid() && !ignoreLocation(loc)) {
                            auto file = compiler.getSourceManager()
                                .getFilename(loc);
                            if (file != SRCDIR "/include/cppuhelper/implbase_ex_post.hxx"
                                && file != SRCDIR "/include/cppuhelper/implbase_ex_pre.hxx")
                            {
                                report(
                                    DiagnosticsEngine::Warning,
                                    "reserved macro identifier", loc);
                            }
                        }
                    }
                    d = d->getPrevious();
                    if (d == nullptr) {
                        break;
                    }
                }
            }
        }
#endif
    }
}

bool ReservedId::VisitNamedDecl(NamedDecl const * decl) {
    auto spelLoc = compiler.getSourceManager().getSpellingLoc(
        decl->getLocation());
    if (ignoreLocation(spelLoc)) {
        return true;
    }
    auto filename = compiler.getSourceManager().getFilename(spelLoc);
    if (filename.startswith(SRCDIR "/bridges/source/cpp_uno/")
        && filename.endswith("share.hxx"))
    {
        return true;
    }
    auto const id = decl->getIdentifier();
    if (id == nullptr) {
        return true;
    }
    auto const s = id->getName();
    switch (determineKind(s)) {
    case Kind::Ok:
        break;
    case Kind::DoubleUnderscore:
/*TODO*/if(s=="BIFF__5"||s=="XML__COLON"||s=="XML__EMPTY"||s=="XML__UNKNOWN_")break;
        if (!(isApi(decl) || isJniFunction(decl))
            && s != "__CERT_DecodeDERCertificate"
                // xmlsecurity/source/xmlsec/nss/nssrenam.h
            && s != "__CERT_NewTempCertificate"
                // xmlsecurity/source/xmlsec/nss/nssrenam.h
            && s != "__CTFont"
                // vcl/source/window/cairo_cairo.cxx -> include/vcl/sysdata.hxx
            && s != "__GLXcontextRec" // vcl/unx/glxtest.cxx
            && s != "__GLXFBConfigRec" // vcl/unx/glxtest.cxx
            && s != "__PK11_GetKeyData"
                // xmlsecurity/source/xmlsec/nss/nssrenam.h
            && s != "__data_start" // sal/osl/unx/system.cxx
            && s != "__lxstat64" // setup_native/scripts/source/getuid.c
            && s != "__lxstat") // setup_native/scripts/source/getuid.c
        {
            report(
                DiagnosticsEngine::Warning,
                "identifier %select{beginning with|containing}0 a double"
                    " underscore is reserved",
                decl->getLocation())
                << compiler.getLangOpts().CPlusPlus << decl->getSourceRange();
        }
        break;
    case Kind::UnderscoreUppercase:
        if (!isApi(decl)
            && s != "_FcPattern" // vcl/inc/unx/fc_fontoptions.hxx
            && s != "_GdkDisplay"
                // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GdkEvent" // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GdkScreen" // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GdkWindow"
                // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GstVideoOverlay"
                // avmedia/source/gstreamer/gstplayer.hxx
            && s != "_XRegion" // vcl/unx/generic/gdi/x11cairotextrender.cxx
            && s != "_XTrap") // vcl/unx/generic/gdi/xrender_peer.hxx
        {
            report(
                DiagnosticsEngine::Warning,
                "identifier beginning with an underscore followed by an"
                    " uppercase letter is reserved",
                decl->getLocation())
                << decl->getSourceRange();
        }
        break;
    case Kind::UnderscoreLowercase:
        if (decl->getDeclContext()->isTranslationUnit()
            && !isa<ParmVarDecl>(decl) && !isApi(decl)
            && s != "_cairo" && s != "_cairo_surface"
                // tools/source/ref/errinf.cxx -> include/vcl/window.hxx ->
                // include/vcl/outdev.hxx -> include/vcl/cairo.hxx
            && s != "_cairo_font_options"
                // vcl/source/window/accessibility.cxx -> vcl/inc/salinst.hxx
            && s != "_cairo_user_data_key"
                // vcl/headless/svpbmp.cxx -> vcl/inc/headless/svpgdi.hxx
            && s != "_end" // sal/osl/unx/system.cxx
            && s != "_rtl_Locale"
                // i18nlangtag/source/isolang/mslangid.cxx ->
                // include/i18nlangtag/languagetag.hxx
            && s != "_uno_ExtEnvironment"
                // cppu/source/threadpool/threadident.cxx ->
                // threadpool/current.hxx
            && s != "_xmlTextWriter") // include/svl/poolitem.hxx
        {
            report(
                DiagnosticsEngine::Warning,
                "identifier beginning with an underscore followed by a"
                    " lowercase letter is reserved in the global namespace",
                decl->getLocation())
                << decl->getSourceRange();
        }
        break;
    }
    return true;
}

ReservedId::Kind ReservedId::determineKind(llvm::StringRef const & id) {
    if (compiler.getLangOpts().CPlusPlus
        && id.find("__") != llvm::StringRef::npos)
    {
        return Kind::DoubleUnderscore;
    }
    if (id.size() >= 2 && id[0] == '_') {
        auto c = id[1];
        if (c == '_') {
            return Kind::DoubleUnderscore;
        }
        if (c >= 'A' && c <= 'Z') {
            return Kind::UnderscoreUppercase;
        }
        if (c >= 'a' && c <= 'z') {
            return Kind::UnderscoreLowercase;
        }
    }
    return Kind::Ok;
}

bool ReservedId::isInLokIncludeFile(SourceLocation spellingLocation) const {
    return compiler.getSourceManager().getFilename(spellingLocation).startswith(
        SRCDIR "/include/LibreOfficeKit/");
}

bool ReservedId::isApi(NamedDecl const * decl) {
    auto const fdecl = dyn_cast<FunctionDecl>(decl);
    if (fdecl != nullptr) {
        decl = fdecl->getCanonicalDecl();
    } else {
        auto const tdecl = dyn_cast<TagDecl>(decl);
        if (tdecl != nullptr) {
            decl = tdecl->getCanonicalDecl();
        }
    }
    auto const loc = compiler.getSourceManager().getSpellingLoc(
        decl->getLocation());
    if (!(isInUnoIncludeFile(loc) || isInLokIncludeFile(loc))
        || isa<ParmVarDecl>(decl))
    {
        return false;
    }
    auto const ctx = decl->getDeclContext();
    if (ctx->isTranslationUnit()
        || (ctx->getDeclKind() == Decl::LinkageSpec
            && ctx->getParent()->isTranslationUnit()))
    {
        return true;
    }
    if (ctx->isNamespace()) {
        auto const id = dyn_cast<NamespaceDecl>(ctx)->getIdentifier();
        return !(id == nullptr || id->getName() == "detail");
    }
    return false;
}

loplugin::Plugin::Registration<ReservedId> X("reservedid");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
