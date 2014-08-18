/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * Based on LLVM/Clang.
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details.
 *
 */
/*
  This is a check/rewriter plugin.
  Refactor tool for convert Fraction to boost::rational<int>
*/
#include "plugin.hxx"

#include <functional>
#include <map>
#include <stdexcept>
#include <string>

namespace loplugin {

//
// INTERFACE
//
class FractReplace : public RecursiveASTVisitor<FractReplace>,
                     public RewritePlugin {
  public:
    FractReplace(const InstantiationData &data);

    // visit methods
    virtual void run() override;
    bool VisitVarDecl(const VarDecl *decl);
    bool VisitParmVarDecl(const ParmVarDecl *decl);
    bool VisitCXXTemporaryObjectExpr(const CXXTemporaryObjectExpr *toe);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *cmce);
    bool VisitFieldDecl(const FieldDecl *fd);
    bool VisitFunctionDecl(const FunctionDecl *fd);

    // callback methods
    void foundTypeFraction(const SourceLocation &begin,
                           const SourceRange &range, const std::string &from);
    void foundMethod_GetNumerator(const std::string name,
                                  const SourceLocation &begin,
                                  const SourceRange &range);
    void foundMethod_GetDenominator(const std::string name,
                                    const SourceLocation &begin,
                                    const SourceRange &range);
    void foundMethod_IsValid(const std::string name,
                             const SourceLocation &begin,
                             const SourceRange &range);
    void foundMethod_long(const std::string name, const SourceLocation &begin,
                          const SourceRange &range);
    void foundMethod_double(const std::string name, const SourceLocation &begin,
                            const SourceRange &range);
    void foundMethod_ReduceInaccurate(const std::string name,
                                      const SourceLocation &begin,
                                      const SourceRange &range);
    void foundMethod_operator_double(const std::string name,
                                     const SourceLocation &begin,
                                     const SourceRange &range);
    void foundMethod_operator_long(const std::string name,
                                   const SourceLocation &begin,
                                   const SourceRange &range);

  private:
    const std::string FRACT_HEADER{ "fract.hxx" };
    // switches
    enum Flags {
        Flag_EnableWarningsWhenRewrite = 1 << 0,
        Flag_EnableVarDecl = 1 << 1,
        Flag_EnableParamVarDecl = 1 << 2,
        Flag_EnableCXXTemporaryObjectExpr = 1 << 3,
        Flag_EnableCXXMemberCallExpr = 1 << 4,
        Flag_EnableFieldDecl = 1 << 5,
        Flag_EnableFunctionDecl = 1 << 6,
    };
    const int flags = Flag_EnableWarningsWhenRewrite
                      | Flag_EnableVarDecl
                      | Flag_EnableParamVarDecl
                      | Flag_EnableCXXTemporaryObjectExpr
                      | Flag_EnableCXXMemberCallExpr
                      | Flag_EnableFieldDecl
                      | Flag_EnableFunctionDecl;

    const std::map<
        std::string,
        std::function<void(FractReplace &, const std::string,
                           const SourceLocation &, const SourceRange &)> >
    method_mapping{
        { "GetNumerator", &FractReplace::foundMethod_GetNumerator },
        { "GetDenominator", &FractReplace::foundMethod_GetDenominator },
        { "IsValid", &FractReplace::foundMethod_IsValid },
        { "long", &FractReplace::foundMethod_long },
        { "double", &FractReplace::foundMethod_double },
        { "ReduceInaccurate", &FractReplace::foundMethod_ReduceInaccurate },
        { "operator double", &FractReplace::foundMethod_operator_double },
        { "operator long", &FractReplace::foundMethod_operator_long },
    };

    // helper methods
    std::string getFilename(SourceLocation loc);
    bool ignoreHeader(SourceLocation loc, std::string name);
    bool ignoreHeader(const Decl *decl, std::string name);
    bool ignoreHeader(const Stmt *stmt, std::string name);
    bool typeIsFraction(const QualType &qtype);
    bool recordIsFraction(const CXXMemberCallExpr *cmce);
    std::string getMethodName(const CXXMemberCallExpr *expr);
    SourceLocation getTypeLocation(const VarDecl *decl);
};

//
// IMPLEMENTATION
//
FractReplace::FractReplace(const Plugin::InstantiationData &data)
    : RewritePlugin(data) {}

void FractReplace::run() {
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool FractReplace::VisitVarDecl(const VarDecl *decl) {
    if (!(flags & Flag_EnableVarDecl))
        return true;
    if (ignoreLocation(decl))
        return true;
    // don't bother processing anything in the Fraction.hxx file.
    if (ignoreHeader(decl, FRACT_HEADER))
        return true;
    if (!typeIsFraction(decl->getType()))
        return true;

    const TypeSourceInfo *TSI = decl->getTypeSourceInfo();
    if (!TSI)
        return true;
    foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                      TSI->getTypeLoc().getSourceRange(), "VarDecl");
    return true;
}

/*
  This:
    void f(Fraction a)
  Compiles to this AST:
    `-FunctionDecl 0x2674eb0 <test.cxx:3:1, line:20:1> f 'void (class Fraction)'
      |-ParmVarDecl 0x2674df0 <line:3:8, col:17> a 'class Fraction'
*/
bool FractReplace::VisitParmVarDecl(const ParmVarDecl *decl) {
    if (!(flags & Flag_EnableParamVarDecl))
        return true;
    if (ignoreLocation(decl))
        return true;
    // don't bother processing anything in the Fraction.hxx file.
    if (ignoreHeader(decl, FRACT_HEADER))
        return true;
    if (!typeIsFraction(decl->getType()))
        return true;

    const TypeSourceInfo *TSI = decl->getTypeSourceInfo();
    if (!TSI)
        return true;
    foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                      TSI->getTypeLoc().getSourceRange(), "ParamVarDecl");
    return true;
}

/*
  This:
    Fraction a;
  Compiles to this AST:
    `-DeclStmt 0x1a27da8 <line:5:5, col:15>
      `-VarDecl 0x1a27d20 <col:5, col:14> a 'class Fraction'
        `-CXXConstructExpr 0x1a27d78 <col:14> 'class Fraction' 'void (void)'
*/
bool
FractReplace::VisitCXXTemporaryObjectExpr(const CXXTemporaryObjectExpr *toe) {
    if (!(flags & Flag_EnableCXXTemporaryObjectExpr))
        return true;
    if (ignoreLocation(toe))
        return true;
    if (ignoreHeader(toe, FRACT_HEADER))
        return true;
    const TypeSourceInfo *TSI = toe->getTypeSourceInfo();
    if (!typeIsFraction(TSI->getType()))
        return true;
    foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                      TSI->getTypeLoc().getSourceRange(),
                      "CXXTemporaryObjectExpr");
    return true;
}

/*
  This:
    // Fraction fract;
    fract.GetNumerator();
  Compiles to this AST:
    `-CXXMemberCallExpr 0x3040f98 <line:6:5, col:24> 'long'
      `-MemberExpr 0x3040f68 <col:5, col:11> '<bound member function type>'
  .GetNumerator 0x30361c0
        `-ImplicitCastExpr 0x303dab0 <col:5> 'const class Fraction' lvalue
  <NoOp>
          `-DeclRefExpr 0x3040f40 <col:5> 'class Fraction' lvalue Var 0x3040ea0
  'fract' 'class Fraction'
 */
bool FractReplace::VisitCXXMemberCallExpr(const CXXMemberCallExpr *cmce) {
    if (!(flags & Flag_EnableCXXMemberCallExpr))
        return true;
    if (ignoreLocation(cmce))
        return true;
    if (ignoreHeader(cmce, FRACT_HEADER))
        return true;

    if (!recordIsFraction(cmce))
        return true;

    const MemberExpr *member = dyn_cast<MemberExpr>(cmce->getCallee());
    const DeclarationNameInfo &info = member->getMemberNameInfo();

    std::string name = info.getAsString();

    try {
        method_mapping.at(name)(*this, name, info.getLocStart(),
                                info.getSourceRange());
    }
    catch (std::out_of_range) {
        report(DiagnosticsEngine::Error,
               "Can't handle method '" + name + "'", info.getLocStart())
            << info.getSourceRange();
    }
    return true;
}

/*
  This:
    class C {
      Fraction f;
    };
  Compiles to this AST:
    |-CXXRecordDecl 0x252bdf0 <test.cxx:3:1, line:5:1> class C definition
    | |-CXXRecordDecl 0x252bf00 <line:3:1, col:7> class C
    | `-FieldDecl 0x2528ad0 <line:4:3, col:12> f 'class Fraction'
 */
bool FractReplace::VisitFieldDecl(const FieldDecl *fd) {
    if (!(flags & Flag_EnableFieldDecl))
        return true;
    if (ignoreLocation(fd))
        return true;
    if (ignoreHeader(fd, FRACT_HEADER))
        return true;
    if (!typeIsFraction(fd->getType()))
        return true;
    const TypeSourceInfo *TSI = fd->getTypeSourceInfo();
    if (!TSI)
        return true;
    foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                      TSI->getTypeLoc().getSourceRange(), "FieldDecl");
    return true;
}

/*
  This:
    Fract f();
  Compiles to this AST:
    |-FunctionDecl 0x33f13d0 <test.cxx:3:1, col:9> f 'int (void)' invalid
  Notes:
    'int (void)' would be 'Fract (void)' if no errors occurs
 */
bool FractReplace::VisitFunctionDecl(const FunctionDecl *fd) {
    if (!(flags & Flag_EnableFunctionDecl))
        return true;
    if (ignoreLocation(fd))
        return true;
    if (ignoreHeader(fd, FRACT_HEADER))
        return true;
    if (typeIsFraction(fd->getType())) {
        const TypeSourceInfo *TSI = fd->getTypeSourceInfo();
        if (TSI) {
            foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                              TSI->getTypeLoc().getSourceRange(),
                              "FunctionDecl type");
        }
    }
    for (int N = fd->getNumParams(), i = 0; i < N; i++) {
        const ParmVarDecl *param = fd->getParamDecl(i);
        if (typeIsFraction(param->getType())) {
            const TypeSourceInfo *TSI = param->getTypeSourceInfo();
            if (TSI) {
                foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                                  TSI->getTypeLoc().getSourceRange(),
                                  "FunctionDecl param");
            }
        }
    }
    return true;
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"

void FractReplace::foundTypeFraction(const SourceLocation &begin,
                                     const SourceRange &range,
                                     const std::string &from) {
    if (rewriter == nullptr) {
        report(DiagnosticsEngine::Warning,
               "Fraction type found [" + from + "]", begin)
            << range;
    } else {
        if ((flags & Flag_EnableWarningsWhenRewrite)) {
            report(DiagnosticsEngine::Warning,
                   "replacing Fraction type [" + from + "]", begin)
                << range;
        }
        replaceText(range, "boost::rational<long>");
    }
}

void FractReplace::foundMethod_GetNumerator(const std::string name,
                                            const SourceLocation &begin,
                                            const SourceRange &range) {
    if (rewriter == nullptr) {
        report(DiagnosticsEngine::Warning, "Found '" + name + "'", begin)
            << range;

    } else {
        if ((flags & Flag_EnableWarningsWhenRewrite)) {
            report(DiagnosticsEngine::Warning,
                   "replacing '" + name + "'", begin)
                << range;
        }
        replaceText(begin, name.length(), "numerator");
    }
}

void FractReplace::foundMethod_GetDenominator(const std::string name,
                                              const SourceLocation &begin,
                                              const SourceRange &range) {
    if (rewriter == nullptr) {
        report(DiagnosticsEngine::Warning, "Found '" + name + "'", begin)
            << range;
    } else {
        if ((flags & Flag_EnableWarningsWhenRewrite)) {
            report(DiagnosticsEngine::Warning,
                   "replacing '" + name + "'", begin)
                << range;
        }
        replaceText(begin, name.length(), "denominator");
    }
}

void FractReplace::foundMethod_IsValid(const std::string name,
                                       const SourceLocation &begin,
                                       const SourceRange &range) {
    if (rewriter == nullptr || (flags & Flag_EnableWarningsWhenRewrite)) {
        report(DiagnosticsEngine::Warning,
               "in boost::rational only valid rationals "
               "can be instantiated - need manual change",
               begin)
            << range;
    }
    if (rewriter != nullptr) {
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE manual change needed */ ");
    }
}

void FractReplace::foundMethod_long(const std::string name,
                                    const SourceLocation &begin,
                                    const SourceRange &range) {
    if (rewriter == nullptr) {
        report(DiagnosticsEngine::Warning,
               "cast using 'boost::rational_cast<long>(expr)'", begin)
            << range;
    } else {
        if ((flags & Flag_EnableWarningsWhenRewrite)) {
            report(DiagnosticsEngine::Warning, "rewrite whole expression using "
                                               "'boost::rational_cast<long>("
                                               "expr)'",
                   begin)
                << range;
        }
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE rewrite whole expression "
                          "using 'boost::rational_cast<long>(expr)' */ ");
    }
}

void FractReplace::foundMethod_double(const std::string name,
                                      const SourceLocation &begin,
                                      const SourceRange &range) {
    if (rewriter == nullptr) {
        report(DiagnosticsEngine::Warning,
               "cast using 'boost::rational_cast<double>(expr)'", begin)
            << range;
    } else {
        if ((flags & Flag_EnableWarningsWhenRewrite)) {
            report(DiagnosticsEngine::Warning,
                   "rewrite whole expression using "
                   "'boost::rational_cast<double>(expr)"
                   "'",
                   begin)
                << range;
        }
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE rewrite whole expression "
                          "using 'boost::rational_cast<double>(expr)' */ ");
    }
}

void FractReplace::foundMethod_ReduceInaccurate(const std::string name,
                                                const SourceLocation &begin,
                                                const SourceRange &range) {
    if (rewriter == nullptr) {
        report(DiagnosticsEngine::Warning,
               "call function ReduceInaccurate(variable)", begin)
            << range;
    } else {
        if ((flags & Flag_EnableWarningsWhenRewrite)) {
            report(DiagnosticsEngine::Warning,
                   "rewrite whole expression to call "
                   "function "
                   "ReduceInaccurate(variable)",
                   begin)
                << range;
        }
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE rewrite whole expression "
                          "using 'boost::rational_cast<double>(expr)' */ ");
    }
}

void FractReplace::foundMethod_operator_double(const std::string name,
                                               const SourceLocation &begin,
                                               const SourceRange &range) {
    // handle: double fScaleX(xFact);
    if (rewriter == nullptr || (flags & Flag_EnableWarningsWhenRewrite)) {
        report(DiagnosticsEngine::Warning,
               "i don't know how to handle this: operator double", begin)
            << range;
    }
    if (rewriter != nullptr) {
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE i don't know how to "
                          "handle this: 'operator double' */ ");
    }
}

void FractReplace::foundMethod_operator_long(const std::string name,
                                             const SourceLocation &begin,
                                             const SourceRange &range) {
    // handle: long fScaleX(xFact);
    if (rewriter == nullptr || (flags & Flag_EnableWarningsWhenRewrite)) {
        report(DiagnosticsEngine::Warning,
               "i don't know how to handle this: operator long", begin)
            << range;
    }
    if (rewriter != nullptr) {
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE i don't know how to "
                          "handle this: 'operator long' */ ");
    }
}

#pragma clang diagnostic pop

std::string FractReplace::getFilename(SourceLocation loc) {
    SourceLocation expansionLoc =
        compiler.getSourceManager().getExpansionLoc(loc);
    return compiler.getSourceManager()
        .getPresumedLoc(expansionLoc)
        .getFilename();
}

bool FractReplace::ignoreHeader(SourceLocation loc, std::string name) {
    return getFilename(loc).find(name) != std::string::npos;
}

inline bool FractReplace::ignoreHeader(const Decl *decl, std::string name) {
    return ignoreHeader(decl->getLocation(), name);
}

inline bool FractReplace::ignoreHeader(const Stmt *stmt, std::string name) {
    // Invalid location can happen at least for ImplicitCastExpr of
    // ImplicitParam 'self' in Objective C method declarations:
    return stmt->getLocStart().isValid() &&
           ignoreHeader(stmt->getLocStart(), name);
}

bool FractReplace::typeIsFraction(const QualType &qtype) {
    const Type *type = qtype.getTypePtr();
    if (const CXXRecordDecl *record_decl = type->getAsCXXRecordDecl()) {
        return record_decl->getQualifiedNameAsString() == "Fraction";
    }
    return false;
}

bool FractReplace::recordIsFraction(const CXXMemberCallExpr *expr) {
    const CXXRecordDecl *record = expr->getRecordDecl();
    return record && record->getNameAsString() == "Fraction";
}

std::string FractReplace::getMethodName(const CXXMemberCallExpr *expr) {
    const CXXMethodDecl *method = expr->getMethodDecl();
    return method->getNameAsString();
}

SourceLocation FractReplace::getTypeLocation(const VarDecl *decl) {
    return decl->getLocStart();
}

// Register the plugin action with the LO plugin handling.
static Plugin::Registration<FractReplace> X("fractreplace", true);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
