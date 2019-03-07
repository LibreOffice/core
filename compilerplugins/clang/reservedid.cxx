/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef LO_CLANG_SHARED_PLUGINS

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
    public loplugin::FilteringPlugin<ReservedId>
{
public:
    explicit ReservedId(loplugin::InstantiationData const & data): FilteringPlugin(data)
    {}

    void run() override;
    void postRun() override;

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
    if (TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()))
        postRun();
}

void ReservedId::postRun() {
    if( compiler.hasPreprocessor())
    {
        auto & prep = compiler.getPreprocessor();
        for (auto const & m: prep.macros(false)) {
            auto id = m.first->getName();
            if (determineKind(id) != Kind::Ok
                && id != "_ATL_APARTMENT_THREADED"
                    // extensions/source/activex/StdAfx2.h
                && id != "_ATL_STATIC_REGISTRY"
                    // extensions/source/activex/StdAfx2.h
                && id != "_GLIBCXX_CDTOR_CALLABI"
                && id != "_HAS_AUTO_PTR_ETC" // unotools/source/i18n/resmgr.cxx
                && id != "_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR" // unotools/source/i18n/resmgr.cxx
                && id != "_MAX_PATH" // Windows
                && id != "_POSIX_SOURCE"
                && id != "_USE_MATH_DEFINES" // include/sal/config.h, Windows
                && id != "_WIN32_DCOM" // embedserv/source/embed/esdll.cxx
                && id != "_WTL_NO_CSTRING"
                    // fpicker/source/win32/filepicker/platform_vista.h (TODO:
                    // needed?)
                && id != "__ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES"
                && id != "__Column_FWD_DEFINED__"
                    // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK
                    // adoctint.h
                && id != "__Group_FWD_DEFINED__"
                    // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK
                    // adoctint.h
                && id != "__Index_FWD_DEFINED__"
                    // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK
                    // adoctint.h
                && id != "__Key_FWD_DEFINED__"
                    // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK
                    // adoctint.h
                && id != "__ORCUS_STATIC_LIB"
                && id != "__Table_FWD_DEFINED__"
                    // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK
                    // adoctint.h
                && id != "__USE_GNU"
                && id != "__User_FWD_DEFINED__")
                    // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK
                    // adoctint.h
            {
                auto d = prep.getLocalMacroDirectiveHistory(m.first);
                for (;;) {
                    if (d->getKind() == MacroDirective::MD_Define) {
                        auto loc = d->getLocation();
                        if (loc.isValid() && !ignoreLocation(loc)) {
                            auto file = getFileNameOfSpellingLoc(loc);
                            if (!loplugin::isSamePathname(
                                    file,
                                    SRCDIR
                                        "/include/cppuhelper/implbase_ex_post.hxx")
                                && !loplugin::isSamePathname(
                                    file,
                                    SRCDIR
                                        "/include/cppuhelper/implbase_ex_pre.hxx"))
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
    }
}

bool ReservedId::VisitNamedDecl(NamedDecl const * decl) {
    auto spelLoc = compiler.getSourceManager().getSpellingLoc(
        decl->getLocation());
    if (ignoreLocation(spelLoc)) {
        return true;
    }
    auto filename = getFileNameOfSpellingLoc(spelLoc);
    if (loplugin::hasPathnamePrefix(filename, SRCDIR "/bridges/source/cpp_uno/")
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
            && s != "__CxxDetectRethrow"
                // bridges/source/cpp_uno/msvc_win32_x86-64/mscx.hxx
            && s != "__GLXcontextRec" // vcl/unx/glxtest.cxx
            && s != "__GLXFBConfigRec" // vcl/unx/glxtest.cxx
            && s != "__PK11_GetKeyData"
                // xmlsecurity/source/xmlsec/nss/nssrenam.h
            && s != "__current_exception" // bridges/inc/except.hxx, Windows
            && s != "__data_start") // sal/osl/unx/system.cxx
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
            && s != "_ADOColumn"
                // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK adoctint.h
            && s != "_ADOGroup"
                // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK adoctint.h
            && s != "_ADOIndex"
                // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK adoctint.h
            && s != "_ADOKey"
                // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK adoctint.h
            && s != "_ADOTable"
                // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK adoctint.h
            && s != "_ADOUser"
                // connectivity/source/inc/ado/Awrapadox.hxx, MS SDK adoctint.h
            && s != "_DllMainCRTStartup"
                // odk/source/unowinreg/win/unowinreg.cxx (TODO: needed?)
            && s != "_FcPattern" // vcl/inc/unx/fc_fontoptions.hxx
            && s != "_GdkDisplay"
                // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GdkEvent" // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GdkScreen" // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GdkWindow"
                // vcl/unx/gtk/xid_fullscreen_on_all_monitors.c
            && s != "_GstVideoOverlay"
                // avmedia/source/gstreamer/gstplayer.hxx
            && s != "_Module" // extensions/source/activex/StdAfx2.h, CComModule
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
    return loplugin::hasPathnamePrefix(
        getFileNameOfSpellingLoc(spellingLocation),
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

loplugin::Plugin::Registration<ReservedId> reservedid("reservedid");

}

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
