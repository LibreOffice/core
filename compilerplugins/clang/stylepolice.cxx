/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <regex>
#include <string>
#include <set>

#include "compat.hxx"
#include "plugin.hxx"

// Check for some basic naming mismatches which make the code harder to read

namespace {

class StylePolice :
    public RecursiveASTVisitor<StylePolice>, public loplugin::Plugin
{
public:
    explicit StylePolice(InstantiationData const & data): Plugin(data) {}

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
    std::string aOriginalTypeName = varDecl->getType().getAsString();
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
        && !startswith(typeName, "std::mem_fun")
        && !startswith(typeName, "std::shared_ptr")
        && !startswith(typeName, "shared_ptr") // weird issue in slideshow
        && !startswith(typeName, "std::unique_ptr")
        && !startswith(typeName, "unique_ptr") // weird issue in include/vcl/threadex.hxx
        && !startswith(typeName, "std::weak_ptr")
        && !startswith(typeName, "struct _LOKDocViewPrivate")
        && !startswith(typeName, "sw::UnoCursorPointer")
        && !startswith(typeName, "tools::SvRef")
        && !startswith(typeName, "VclPtr")
        && !startswith(typeName, "vcl::ScopedBitmapAccess")
        // lots of the code seems to regard iterator objects as being "pointer-like"
        && typeName.find("iterator<") == std::string::npos
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
    return true;
}

loplugin::Plugin::Registration< StylePolice > X("stylepolice");

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
