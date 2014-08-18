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
#include "compat.hxx"

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
    bool VisitCXXTemporaryObjectExpr(const CXXTemporaryObjectExpr *toe);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *cmce);
    bool VisitFieldDecl(const FieldDecl *fd);
    bool VisitFunctionDecl(const FunctionDecl *fd);

    void foundTypeFraction(const SourceLocation &begin,
                           const SourceRange &range, const std::string &from,
                           const QualType *type = nullptr);

    void foundMethod_GetNumerator(const SourceLocation &begin,
                                  const SourceRange &range, const Expr *);
    void foundMethod_GetDenominator(const SourceLocation &begin,
                                    const SourceRange &range, const Expr *);
    void foundMethod_IsValid(const SourceLocation &, const SourceRange &,
                             const Expr *expr);
    void foundMethod_long(const SourceLocation &begin, const SourceRange &range,
                          const Expr *);
    void foundMethod_double(const SourceLocation &begin,
                            const SourceRange &range, const Expr *);
    void foundMethod_ReduceInaccurate(const SourceLocation &,
                                      const SourceRange &, const Expr *);
    void foundMethod_operator_double(const SourceLocation &begin,
                                     const SourceRange &range, const Expr *);
    void foundMethod_operator_long(const SourceLocation &begin,
                                   const SourceRange &range, const Expr *);

  private:
    const std::string FRACT_HEADER{ "fract.hxx" };
    const std::string FRACT_CODE{ "fract.cxx" };
    // switches
    enum Flags {
        Flag_EnableNotesWhenRewrite = 1 << 0,
        Flag_EnableVarDecl = 1 << 1,
        Flag_EnableCXXTemporaryObjectExpr = 1 << 2,
        Flag_EnableCXXMemberCallExpr = 1 << 3,
        Flag_EnableFieldDecl = 1 << 4,
        Flag_EnableFunctionDecl = 1 << 5,
    };
    const int flags = Flag_EnableNotesWhenRewrite | Flag_EnableVarDecl |
                      Flag_EnableCXXTemporaryObjectExpr |
                      Flag_EnableCXXMemberCallExpr | Flag_EnableFieldDecl |
                      Flag_EnableFunctionDecl;

    const std::map<std::string,
                   std::function<void(FractReplace &, const SourceLocation &,
                                      const SourceRange &, const Expr *)> >
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
    bool ignoreFile(SourceLocation loc, std::string name);
    bool ignoreFile(const Decl *decl, std::string name);
    bool ignoreFile(const Stmt *stmt, std::string name);
    bool typeIsFraction(const QualType &qtype);
    bool recordIsFraction(const CXXMemberCallExpr *cmce);
    std::string getMethodName(const CXXMemberCallExpr *expr);
    SourceLocation getTypeLocation(const VarDecl *decl);
    std::string convertToString(const Stmt *From);
};

//
// IMPLEMENTATION
//

FractReplace::FractReplace(const Plugin::InstantiationData &data)
    : RewritePlugin(data) {}

void FractReplace::run() {
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
    if (rewriter != nullptr)
        rewriter->overwriteChangedFiles(); // write include files too
}

// FIXME: source range from TypeSourceInfo is wrong if the declaration has const
// qualifier:
// const Fraction *f
//       ^~~~~~~~~~
bool FractReplace::VisitVarDecl(const VarDecl *decl) {
    if (!(flags & Flag_EnableVarDecl))
        return true;
    if (ignoreLocation(decl))
        return true;
    if (ignoreFile(decl, FRACT_HEADER) || ignoreFile(decl, FRACT_CODE))
        return true;
    QualType type = decl->getType();
    if (!typeIsFraction(type))
        return true;
    if (const TypeSourceInfo *TSI = decl->getTypeSourceInfo()) {
        foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                          TSI->getTypeLoc().getSourceRange(), "VarDecl", &type);
    }
    return true;
}

bool
FractReplace::VisitCXXTemporaryObjectExpr(const CXXTemporaryObjectExpr *toe) {
    if (!(flags & Flag_EnableCXXTemporaryObjectExpr))
        return true;
    if (ignoreLocation(toe))
        return true;
    if (ignoreFile(toe, FRACT_HEADER) || ignoreFile(toe, FRACT_CODE))
        return true;
    const TypeSourceInfo *TSI = toe->getTypeSourceInfo();
    if (!typeIsFraction(TSI->getType()))
        return true;
    foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                      TSI->getTypeLoc().getSourceRange(),
                      "CXXTemporaryObjectExpr");
    return true;
}

bool FractReplace::VisitCXXMemberCallExpr(const CXXMemberCallExpr *cmce) {
    if (!(flags & Flag_EnableCXXMemberCallExpr))
        return true;
    if (ignoreLocation(cmce))
        return true;
    if (ignoreFile(cmce, FRACT_HEADER) || ignoreFile(cmce, FRACT_CODE))
        return true;

    if (!recordIsFraction(cmce))
        return true;

    const MemberExpr *member = dyn_cast<MemberExpr>(cmce->getCallee());
    const DeclarationNameInfo &info = member->getMemberNameInfo();

    std::string name = info.getAsString();

    try {
        method_mapping.at(name)(*this, info.getLocStart(),
                                info.getSourceRange(), cmce);
    }
    catch (std::out_of_range) {
        report(DiagnosticsEngine::Error,
               "Can't handle method '" + name + "'", info.getLocStart())
            << info.getSourceRange();
    }
    return true;
}

bool FractReplace::VisitFieldDecl(const FieldDecl *fd) {
    if (!(flags & Flag_EnableFieldDecl))
        return true;
    if (ignoreLocation(fd))
        return true;
    if (ignoreFile(fd, FRACT_HEADER) || ignoreFile(fd, FRACT_CODE))
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

bool FractReplace::VisitFunctionDecl(const FunctionDecl *fd) {
    if (!(flags & Flag_EnableFunctionDecl))
        return true;
    if (ignoreLocation(fd))
        return true;
    if (ignoreFile(fd, FRACT_HEADER) || ignoreFile(fd, FRACT_CODE))
        return true;

    QualType type = compat::getReturnType(*fd);
    if (typeIsFraction(type)) {
        const SourceRange range = compat::getReturnTypeSourceRange(*fd);
        foundTypeFraction(range.getBegin(), range, "FunctionDecl", &type);
    }
    return true;
}

void FractReplace::foundMethod_GetNumerator(const SourceLocation &begin,
                                            const SourceRange &range,
                                            const Expr *) {
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(
                DiagnosticsEngine::Note,
                "replacing 'GetNumerator()' method with 'numerator()'", begin)
                << range;
        replaceText(range, "numerator");
    } else
        report(DiagnosticsEngine::Warning,
               "replace 'GetNumerator()' method with 'numerator()'", begin)
            << range;
}

void FractReplace::foundMethod_GetDenominator(const SourceLocation &begin,
                                              const SourceRange &range,
                                              const Expr *) {
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "replacing 'GetDenominator()' method with 'denominator()'",
                   begin)
                << range;
        replaceText(range, "denominator");
    } else
        report(DiagnosticsEngine::Warning,
               "replace 'GetDenominator()' method with 'denominator()'", begin)
            << range;
}

void FractReplace::foundMethod_IsValid(const SourceLocation &,
                                       const SourceRange &, const Expr *expr) {
    const SourceLocation &begin = expr->getLocStart();
    const SourceRange &range = expr->getSourceRange();
    std::string exprText = convertToString(expr);
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "add comment for refactor code around '" + exprText +
                       "' (always is true "
                       "when use boost::rational<long>)",
                   begin)
                << range;
        replaceText(
            range,
            "true /* LOPLUGIN:FRACTREPLACE FIXME: need manual refactor: '" +
                exprText + "' always is true when use boost::rational<long> "
                           "*/");
    } else
        report(DiagnosticsEngine::Warning,
               "need manual refactor: '" + exprText +
                   "' always is true "
                   "when use boost::rational<long>",
               begin)
            << range;
}

void FractReplace::foundMethod_long(const SourceLocation &begin,
                                    const SourceRange &range, const Expr *) {
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "add comment for manual change of the expression", begin)
                << range;
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE manual change is needed "
                          "for rewrite the whole expression using "
                          "'boost::rational_cast<long>(expr)' */ ");
    } else
        report(DiagnosticsEngine::Warning, "manual change is needed for "
                                           "rewrite the whole expression using "
                                           "'boost::rational_cast<long>(expr)'",
               begin)
            << range;
}

void FractReplace::foundMethod_double(const SourceLocation &begin,
                                      const SourceRange &range, const Expr *) {
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Warning,
                   "add comment for manual change of the expression", begin)
                << range;
        insertText(begin, "/* LOPLUGIN:FRACTREPLACE manual change is needed "
                          "for rewrite the whole expression "
                          "using 'boost::rational_cast<double>(expr)' */");
    } else
        report(DiagnosticsEngine::Warning,
               "manual change is needed for rewrite the whole expression using "
               "'boost::rational_cast<double>(expr)'",
               begin)
            << range;
}

void FractReplace::foundMethod_ReduceInaccurate(const SourceLocation &,
                                                const SourceRange &,
                                                const Expr *expr) {
    const SourceLocation &begin = expr->getLocStart();
    const SourceRange &range = expr->getSourceRange();
    const CXXMemberCallExpr *callexpr = dyn_cast<CXXMemberCallExpr>(expr);
    std::string implicitObject =
        convertToString(callexpr->getImplicitObjectArgument());
    std::string argument = convertToString(callexpr->getArg(0));

    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "replaced '" + implicitObject + ".ReduceInaccurate(" +
                       argument + ")' with 'reduceInaccurate(" +
                       implicitObject + ", " + argument + ")'",
                   begin)
                << range;
        replaceText(range, "reduceInaccurate(" + implicitObject + ", " +
                               argument + ")");
    } else
        report(DiagnosticsEngine::Warning,
               "replace '" + implicitObject + ".ReduceInaccurate(" + argument +
                   ")' with 'reduceInaccurate(" + implicitObject + ", " +
                   argument + ")'",
               begin)
            << range;
}

void FractReplace::foundMethod_operator_double(const SourceLocation &begin,
                                               const SourceRange &range,
                                               const Expr *) {
    // handle: double fScaleX(xFact);
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "add comment for refactor 'operator double' manually", begin)
                << range;
        insertText(begin, "/* LOPLUGIN:FRACTREPLACE i don't know how to "
                          "handle this: 'operator double' */");
    } else
        report(DiagnosticsEngine::Warning,
               "i don't know how to handle this: operator double", begin)
            << range;
}

void FractReplace::foundMethod_operator_long(const SourceLocation &begin,
                                             const SourceRange &range,
                                             const Expr *) {
    // handle: long fScaleX(xFact);
    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "add comment for refactor 'operator long' manually", begin)
                << range;
        insertText(begin, "/* LOPLUGIN:FRACTREPLACE i don't know how to "
                          "handle this: 'operator long' */");
    } else
        report(DiagnosticsEngine::Warning,
               "i don't know how to handle this: operator long", begin)
            << range;
}

std::string FractReplace::getFilename(SourceLocation loc) {
    SourceLocation expansionLoc =
        compiler.getSourceManager().getExpansionLoc(loc);
    return compiler.getSourceManager()
        .getPresumedLoc(expansionLoc)
        .getFilename();
}

bool FractReplace::ignoreFile(SourceLocation loc, std::string name) {
    return getFilename(loc).find(name) != std::string::npos;
}

inline bool FractReplace::ignoreFile(const Decl *decl, std::string name) {
    return ignoreFile(decl->getLocation(), name);
}

inline bool FractReplace::ignoreFile(const Stmt *stmt, std::string name) {
    // Invalid location can happen at least for ImplicitCastExpr of
    // ImplicitParam 'self' in Objective C method declarations:
    return stmt->getLocStart().isValid() &&
           ignoreFile(stmt->getLocStart(), name);
}

void FractReplace::foundTypeFraction(const SourceLocation &begin,
                                     const SourceRange &range,
                                     const std::string &from,
                                     const QualType *type) {
    std::string nametype = type != nullptr ? type->getAsString() : "Fraction";
    std::string newtype = "boost::rational<long>";
    bool isconst = false;
    if (type != nullptr) {
        if ((*type)->isReferenceType()) {
            newtype += "&";
            if (from == "FunctionDecl")
                newtype += " ";
            isconst = (*type)->getPointeeType().isLocalConstQualified();
        }
        if ((*type)->isPointerType()) {
            newtype += " *";
            isconst = (*type)->getPointeeType().isLocalConstQualified();
        }
    }

    if (rewriter != nullptr) {
        if (flags & Flag_EnableNotesWhenRewrite)
            report(DiagnosticsEngine::Note,
                   "replaced '" + nametype + "' with '" +
                       ((isconst) ? "const " : "") + newtype + "' [" + from +
                       "]",
                   begin)
                << range;
        replaceText(range, newtype);
    } else
        report(DiagnosticsEngine::Warning,
               "replace '" + nametype + "' with '" +
                   ((isconst) ? "const " : "") + newtype + "' [" + from + "]",
               begin)
            << range;
}

bool FractReplace::typeIsFraction(const QualType &type) {
    const QualType &t = (type->isPointerType() || type->isReferenceType())
                            ? type->getPointeeType()
                            : type;
    const CXXRecordDecl *record_decl = t->getAsCXXRecordDecl();
    return record_decl != nullptr &&
           record_decl->getQualifiedNameAsString() == "Fraction";
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

std::string FractReplace::convertToString(const Stmt *From) {
    assert(From != nullptr && "Expected non-null Stmt");
    std::string SStr;
    llvm::raw_string_ostream S(SStr);
    From->printPretty(S, nullptr, PrintingPolicy(LangOptions()));
    return S.str();
}

// Register the plugin action with the LO plugin handling.
static Plugin::Registration<FractReplace> X("fractreplace", true);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
