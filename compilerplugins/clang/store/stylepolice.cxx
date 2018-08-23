/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <memory>
#include <regex>
#include <string>
#include <set>

#include "plugin.hxx"

// Check for some basic naming mismatches which make the code harder to read
//
// This plugin is deliberately fairly lax, and only targets the most egregeriously faulty code,
// since we have a broad range of styles in our code and we don't want to generate unnecessary
// churn.

namespace {

class StylePolice :
    public loplugin::FilteringPlugin<StylePolice>
{
public:
    explicit StylePolice(InstantiationData const & data): FilteringPlugin(data) {}

    virtual void run() override { TraverseDecl(compiler.getASTContext().getTranslationUnitDecl()); }

    bool VisitVarDecl(const VarDecl *);
private:
    StringRef getFilename(SourceLocation loc);
};

StringRef StylePolice::getFilename(SourceLocation loc)
{
    SourceLocation spellingLocation = compiler.getSourceManager().getSpellingLoc(loc);
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name;
}

bool startswith(const std::string& rStr, const char* pSubStr) {
    return rStr.compare(0, strlen(pSubStr), pSubStr) == 0;
}
bool isUpperLetter(char c) {
    return c >= 'A' && c <= 'Z';
}
bool isLowerLetter(char c) {
    return c >= 'a' && c <= 'z';
}
bool isIdentifierLetter(char c) {
    return isUpperLetter(c) || isLowerLetter(c);
}
bool matchPointerVar(const std::string& s) {
    return s.size() > 2 && s[0] == 'p' && isUpperLetter(s[1]);
}
bool matchRefCountedPointerVar(const std::string& s) {
    return s.size() > 2 && s[0] == 'x' && isUpperLetter(s[1]);
}
bool matchMember(const std::string& s) {
    return s.size() > 3 && s[0] == 'm'
        && (   ( strchr("abnprsx", s[1]) && isUpperLetter(s[2]) )
            || ( s[1] == '_' && isIdentifierLetter(s[2]) ) );
}

bool StylePolice::VisitVarDecl(const VarDecl * varDecl)
{
    if (ignoreLocation(varDecl)) {
        return true;
    }
    StringRef aFileName = getFilename(varDecl->getLocStart());
    std::string name = varDecl->getName();

    if (!varDecl->isLocalVarDecl()) {
        return true;
    }

    if (matchMember(name))
    {
        // these names appear to be taken from some scientific paper
        if (aFileName == SRCDIR "/scaddins/source/analysis/bessel.cxx" ) {
        }
        // lots of places where we are storing a "method id" here
        else if (aFileName.startswith(SRCDIR "/connectivity/source/drivers/jdbc") && name.compare(0,3,"mID") == 0) {
        }
        else {
            report(
                DiagnosticsEngine::Warning,
                "this local variable follows our member field naming convention, which is confusing",
                varDecl->getLocation())
                 << varDecl->getType() << varDecl->getSourceRange();
        }
    }

    QualType qt = varDecl->getType().getDesugaredType(compiler.getASTContext()).getCanonicalType();
    qt = qt.getNonReferenceType();
    std::string typeName = qt.getAsString();
    if (startswith(typeName, "const "))
        typeName = typeName.substr(6);
    if (startswith(typeName, "class "))
        typeName = typeName.substr(6);
    if (startswith(typeName, "struct "))
        typeName = typeName.substr(7);
    std::string aOriginalTypeName = varDecl->getType().getAsString();
    if (startswith(aOriginalTypeName, "const "))
        aOriginalTypeName = aOriginalTypeName.substr(6);

    if (!qt->isPointerType() && !qt->isArrayType() && !qt->isFunctionPointerType() && !qt->isMemberPointerType()
        && matchPointerVar(name)
        && !startswith(typeName, "boost::intrusive_ptr")
        && !startswith(typeName, "boost::optional")
        && !startswith(typeName, "boost::shared_ptr")
        && !startswith(typeName, "com::sun::star::uno::Reference")
        && !startswith(typeName, "cppu::OInterfaceIteratorHelper")
        && !startswith(typeName, "formula::FormulaCompiler::CurrentFactor")
        && aOriginalTypeName != "GLXPixmap"
        && !startswith(typeName, "rtl::Reference")
        && !startswith(typeName, "ScopedVclPtr")
        && typeName.find("::mem_fun") == std::string::npos
        && typeName.find("shared_ptr") == std::string::npos
        && typeName.find("unique_ptr") == std::string::npos
        && typeName.find("::weak_ptr") == std::string::npos
        && !startswith(typeName, "_LOKDocViewPrivate")
        && !startswith(typeName, "sw::UnoCursorPointer")
        && !startswith(typeName, "tools::SvRef")
        && !startswith(typeName, "VclPtr")
        && !startswith(typeName, "vcl::ScopedBitmapAccess")
        // lots of the code seems to regard iterator objects as being "pointer-like"
        && typeName.find("iterator<") == std::string::npos
        && typeName.find("iter<") == std::string::npos
            // libc++ std::__1::__wrap_iter<...>
        && aOriginalTypeName != "sal_IntPtr" )
    {
        if (aFileName.startswith(SRCDIR "/bridges/") ) {
        } else if (aFileName.startswith(SRCDIR "/vcl/source/fontsubset/sft.cxx") ) {
        } else {
            report(
                DiagnosticsEngine::Warning,
                "this local variable of type '%0' follows our pointer naming convention, but it is not a pointer, %1",
                varDecl->getLocation())
                 << typeName << aOriginalTypeName << varDecl->getSourceRange();
        }
    }


    if (matchRefCountedPointerVar(name)
        && !startswith(typeName, "boost::intrusive_ptr")
        && !startswith(typeName, "com::sun::star::uno::Reference")
        && !startswith(typeName, "com::sun::star::uno::Sequence")
        && !startswith(typeName, "com::sun::star::uno::WeakReference")
        && !startswith(typeName, "drawinglayer::primitive2d::Primitive2DContainer")
        && !startswith(typeName, "drawinglayer::primitive3d::Primitive3DContainer")
        && !startswith(typeName, "jfw::CXPathObjectPtr")
        && !startswith(typeName, "_LOKDocViewPrivate")
        && !startswith(typeName, "oox::dump::BinaryInputStreamRef")
        && !startswith(typeName, "oox::drawingml::chart::ModelRef")
        && !startswith(typeName, "rtl::Reference")
        && !startswith(typeName, "Reference")
        && !startswith(typeName, "SfxObjectShellLock")
        && !startswith(typeName, "store::PageHolderObject")
        && !startswith(typeName, "store::ResourceHolder")
        && !startswith(typeName, "store::OStoreHandle")
        && typeName.find("unique_ptr") == std::string::npos
        && typeName.find("shared_ptr") == std::string::npos
        && !startswith(typeName, "ScopedVclPtr")
        && !startswith(typeName, "svt::EmbeddedObjectRef")
        && !startswith(typeName, "tools::SvRef")
        && !startswith(typeName, "tools::WeakReference")
        && !startswith(typeName, "utl::SharedUNOComponent")
        && !startswith(typeName, "VclPtr")
        && !startswith(typeName, "vcl::DeleteOnDeinit")
        && !startswith(typeName, "vcl::DeleteUnoReferenceOnDeinit")
        // there are lots of coordinate/position vars that start with "x"
        && !qt->isArithmeticType()
        && !startswith(typeName, "float [")
        )
    {
            report(
                DiagnosticsEngine::Warning,
                "this local variable of type '%0' follows our ref-counted-pointer naming convention, but it is not a ref-counted-pointer, %1",
                varDecl->getLocation())
                 << typeName << aOriginalTypeName << varDecl->getSourceRange();
    }


    return true;
}

loplugin::Plugin::Registration< StylePolice > X("stylepolice");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
