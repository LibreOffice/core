/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cassert>
#include <string>
#include <iostream>
#include <fstream>
#include <set>
#include "plugin.hxx"
#include "compat.hxx"

/**
This looks for unused enum constants

We search for 3 things
(a) constants that are declared but never used
(b) constants only used in a "read" fashion i.e. we compare stuff against them, but we never store a value anywhere
(c) constants only used in a "write" fashion i.e. we store a value, but never check for that value

(a) is fairly reliable but (b) and (c) will need some checking before acting on.

Be warned that it produces around 5G of log file.

The process goes something like this:
  $ make check
  $ make FORCE_COMPILE_ALL=1 COMPILER_PLUGIN_TOOL='unusedenumconstants' check
  $ ./compilerplugins/clang/unusedenumconstants.py

Note that the actual process may involve a fair amount of undoing, hand editing, and general messing around
to get it to work :-)

*/

namespace {

struct MyFieldInfo
{
    std::string parentClass;
    std::string fieldName;
    std::string sourceLocation;
};
bool operator < (const MyFieldInfo &lhs, const MyFieldInfo &rhs)
{
    return std::tie(lhs.parentClass, lhs.fieldName)
         < std::tie(rhs.parentClass, rhs.fieldName);
}


// try to limit the voluminous output a little
static std::set<MyFieldInfo> definitionSet;
static std::set<MyFieldInfo> writeSet;
static std::set<MyFieldInfo> readSet;


class UnusedEnumConstants:
    public RecursiveASTVisitor<UnusedEnumConstants>, public loplugin::Plugin
{
public:
    explicit UnusedEnumConstants(InstantiationData const & data): Plugin(data) {}

    virtual void run() override
    {
        TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());

        // dump all our output in one write call - this is to try and limit IO "crosstalk" between multiple processes
        // writing to the same logfile
        std::string output;
        for (const MyFieldInfo & s : definitionSet)
            output += "definition:\t" + s.parentClass + "\t" + s.fieldName + "\t" + s.sourceLocation + "\n";
        for (const MyFieldInfo & s : writeSet)
            output += "write:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        for (const MyFieldInfo & s : readSet)
            output += "read:\t" + s.parentClass + "\t" + s.fieldName + "\n";
        ofstream myfile;
        myfile.open( SRCDIR "/loplugin.unusedenumconstants.log", ios::app | ios::out);
        myfile << output;
        myfile.close();
    }

    bool shouldVisitTemplateInstantiations () const { return true; }
    bool shouldVisitImplicitCode() const { return true; }

    bool VisitEnumConstantDecl( const EnumConstantDecl * );
    bool VisitDeclRefExpr( const DeclRefExpr * );
private:
    MyFieldInfo niceName(const EnumConstantDecl*);
};

MyFieldInfo UnusedEnumConstants::niceName(const EnumConstantDecl* enumConstantDecl)
{
    MyFieldInfo aInfo;

    aInfo.parentClass = enumConstantDecl->getType().getAsString();
    aInfo.fieldName = enumConstantDecl->getNameAsString();

    SourceLocation expansionLoc = compiler.getSourceManager().getExpansionLoc( enumConstantDecl->getLocation() );
    StringRef name = compiler.getSourceManager().getFilename(expansionLoc);
    aInfo.sourceLocation = std::string(name.substr(strlen(SRCDIR)+1)) + ":" + std::to_string(compiler.getSourceManager().getSpellingLineNumber(expansionLoc));
    normalizeDotDotInFilePath(aInfo.sourceLocation);

    return aInfo;
}

bool UnusedEnumConstants::VisitEnumConstantDecl( const EnumConstantDecl* enumConstantDecl )
{
    enumConstantDecl = enumConstantDecl->getCanonicalDecl();
    if (ignoreLocation( enumConstantDecl )) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(enumConstantDecl->getLocation()))) {
        return true;
    }

    definitionSet.insert(niceName(enumConstantDecl));
    return true;
}

bool UnusedEnumConstants::VisitDeclRefExpr( const DeclRefExpr* declRefExpr )
{
    const Decl* decl = declRefExpr->getDecl();
    const EnumConstantDecl* enumConstantDecl = dyn_cast<EnumConstantDecl>(decl);
    if (!enumConstantDecl) {
        return true;
    }
    enumConstantDecl = enumConstantDecl->getCanonicalDecl();
    if (ignoreLocation(enumConstantDecl)) {
        return true;
    }
    // ignore stuff that forms part of the stable URE interface
    if (isInUnoIncludeFile(compiler.getSourceManager().getSpellingLoc(enumConstantDecl->getLocation()))) {
        return true;
    }

    const Stmt * parent = declRefExpr;
try_again:
    parent = parentStmt(parent);
    bool bWrite = false;
    bool bRead = false;
    bool bDump = false;
    if (!parent)
    {
        // could do better here, sometimes this is a constructor-initialiser-expression
        bWrite = true;
    } else if (isa<CallExpr>(parent) || isa<InitListExpr>(parent) || isa<ArraySubscriptExpr>(parent)
                || isa<ReturnStmt>(parent) || isa<DeclStmt>(parent)
                || isa<CXXConstructExpr>(parent)
                || isa<CXXThrowExpr>(parent))
    {
        bWrite = true;
    } else if (isa<CaseStmt>(parent) || isa<SwitchStmt>(parent))
    {
        bRead = true;
    } else if (const BinaryOperator * binaryOp = dyn_cast<BinaryOperator>(parent))
    {
        if (BinaryOperator::isAssignmentOp(binaryOp->getOpcode())) {
            bWrite = true;
        } else {
            bRead = true;
        }
    } else if (isa<CastExpr>(parent) || isa<UnaryOperator>(parent)
                || isa<ConditionalOperator>(parent) || isa<ParenExpr>(parent)
                || isa<MaterializeTemporaryExpr>(parent)
                || isa<ExprWithCleanups>(parent))
    {
        goto try_again;
    } else {
        bDump = true;
    }

    if (bDump) {
        parent->dump();
        report( DiagnosticsEngine::Warning,
                "xxxxxx",
                parent->getLocStart());
    }
    if (bWrite) {
        writeSet.insert(niceName(enumConstantDecl));
    }
    if (bRead) {
        readSet.insert(niceName(enumConstantDecl));
    }
    return true;
}

loplugin::Plugin::Registration< UnusedEnumConstants > X("unusedenumconstants", true);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
