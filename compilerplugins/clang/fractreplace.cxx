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
    bool VisitCStyleCastExpr(const CStyleCastExpr *expr);

    void foundTypeFraction(const SourceLocation &begin,
                           const SourceRange &range, const std::string &from,
                           const QualType *type = nullptr);

    void foundCast_operator_double(const CStyleCastExpr *expr_CStyleCast,
                                   const CXXMemberCallExpr *expr_CXXMemberCall);

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
    void foundMethod_operator_double(const SourceLocation &,
                                     const SourceRange &,
                                     const Expr *expr);
    void foundMethod_operator_long(const SourceLocation &begin,
                                   const SourceRange &range, const Expr *);

  private:
    const std::string FRACT_HEADER{ "fract.hxx" };
    const std::string FRACT_CODE{ "fract.cxx" };
    const bool Flag_IgnoreUnknownMethods = false;
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
    bool ignoreByLocationOrFile(const Decl *decl);
    bool ignoreByLocationOrFile(const Stmt *stmt);
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
}

// FIXME: source range from TypeSourceInfo is wrong if the declaration has const
// qualifier:
// const Fraction *f
//       ^~~~~~~~~~
bool FractReplace::VisitVarDecl(const VarDecl *decl) {
    if (ignoreByLocationOrFile(decl))
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
    if (ignoreByLocationOrFile(toe))
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
    if (ignoreByLocationOrFile(cmce))
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
        report(Flag_IgnoreUnknownMethods ? DiagnosticsEngine::Warning : DiagnosticsEngine::Error,
               "Can't handle method '" + name + "'", info.getLocStart())
            << info.getSourceRange();
    }
    return true;
}

bool FractReplace::VisitFieldDecl(const FieldDecl *fd) {
    if (ignoreByLocationOrFile(fd))
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
    if (ignoreByLocationOrFile(fd))
        return true;
    QualType type = compat::getReturnType(*fd);
    if (typeIsFraction(type)) {
        const SourceRange range = compat::getReturnTypeSourceRange(*fd);
        foundTypeFraction(range.getBegin(), range, "FunctionDecl", &type);
    }
    return true;
}

bool FractReplace::VisitCStyleCastExpr(const CStyleCastExpr *expr_CStyleCast) {
    if (ignoreByLocationOrFile(expr_CStyleCast))
        return true;
    if (CK_NoOp == expr_CStyleCast->getCastKind()) {
        if (const ImplicitCastExpr *expr_ImplCast = dyn_cast<ImplicitCastExpr>(expr_CStyleCast->getSubExpr())) {
            if (CK_UserDefinedConversion == expr_ImplCast->getCastKind()) {
                if (const CXXMemberCallExpr *expr_CXXMembCall = dyn_cast<CXXMemberCallExpr>(expr_ImplCast->getSubExpr())) {
                    if (recordIsFraction(expr_CXXMembCall)) {
                        const MemberExpr *expr_Memb = dyn_cast<MemberExpr>(expr_CXXMembCall->getCallee());
                        const DeclarationNameInfo &info = expr_Memb->getMemberNameInfo();
                        std::string name = info.getAsString();
                        if (name == "operator double") {
                            foundCast_operator_double(expr_CStyleCast, expr_CXXMembCall);
                            return false;
                        } else {
                            report(DiagnosticsEngine::Error, "can't handle cast for member call '" + name + "'");
                        }
                    }
                }
            }
        }
    }
    return true;
}

void FractReplace::foundCast_operator_double(const CStyleCastExpr *expr_CStyleCast, const CXXMemberCallExpr *expr_CXXMemberCall) {
    const SourceLocation& begin = expr_CStyleCast->getExprLoc();
    const SourceRange& range = expr_CStyleCast->getSourceRange();
    std::string implicitObject = convertToString(expr_CXXMemberCall->getImplicitObjectArgument());
    if (rewriter != nullptr)
        replaceText(range, "boost::rational_cast<double>(" + implicitObject + ")");
    else
        report(DiagnosticsEngine::Warning, "replace c-style cast with 'boost::rational_cast<{cast-type}>(" + implicitObject + ")'", begin) << range;
}

void FractReplace::foundMethod_GetNumerator(const SourceLocation &begin,
                                            const SourceRange &range,
                                            const Expr *) {
    if (rewriter != nullptr)
        replaceText(range, "numerator");
    else
        report(DiagnosticsEngine::Warning,
               "replace 'GetNumerator()' method with 'numerator()'", begin)
            << range;
}

void FractReplace::foundMethod_GetDenominator(const SourceLocation &begin,
                                              const SourceRange &range,
                                              const Expr *) {
    if (rewriter != nullptr)
        replaceText(range, "denominator");
    else
        report(DiagnosticsEngine::Warning,
               "replace 'GetDenominator()' method with 'denominator()'", begin)
            << range;
}

void FractReplace::foundMethod_IsValid(const SourceLocation &,
                                       const SourceRange &, const Expr *expr) {
    const SourceLocation &begin = expr->getLocStart();
    const SourceRange &range = expr->getSourceRange();
    std::string exprText = convertToString(expr);
    if (rewriter != nullptr)
        replaceText(
            range,
            "true /* LOPLUGIN:FRACTREPLACE FIXME: need manual refactor: '" +
                exprText + "' always is true when use boost::rational<long> "
                           "*/");
    else
        report(DiagnosticsEngine::Warning,
               "need manual refactor: '" + exprText +
                   "' always is true "
                   "when use boost::rational<long>",
               begin)
            << range;
}

void FractReplace::foundMethod_long(const SourceLocation &begin,
                                    const SourceRange &range, const Expr *) {
    if (rewriter != nullptr)
        insertText(begin, " /* LOPLUGIN:FRACTREPLACE manual change is needed "
                          "for rewrite the whole expression using "
                          "'boost::rational_cast<long>(expr)' */ ");
    else
        report(DiagnosticsEngine::Warning, "manual change is needed for "
                                           "rewrite the whole expression using "
                                           "'boost::rational_cast<long>(expr)'",
               begin)
            << range;
}

void FractReplace::foundMethod_double(const SourceLocation &begin,
                                      const SourceRange &range, const Expr *) {
    if (rewriter != nullptr)
        insertText(begin, "/* LOPLUGIN:FRACTREPLACE manual change is needed "
                          "for rewrite the whole expression "
                          "using 'boost::rational_cast<double>(expr)' */");
    else
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

    if (rewriter != nullptr)
        replaceText(range, "reduceInaccurate(" + implicitObject + ", " +
                               argument + ")");
    else
        report(DiagnosticsEngine::Warning,
               "replace '" + implicitObject + ".ReduceInaccurate(" + argument +
                   ")' with 'reduceInaccurate(" + implicitObject + ", " +
                   argument + ")'",
               begin)
            << range;
}

void FractReplace::foundMethod_operator_double(const SourceLocation &,
                                               const SourceRange &,
                                               const Expr *expr) {
    // handle: double fScaleX(xFact);
    const SourceLocation& begin = expr->getExprLoc();
    const SourceRange& range = expr->getSourceRange();
    const CXXMemberCallExpr *callexpr = dyn_cast<CXXMemberCallExpr>(expr);
    std::string implicitObject =
        convertToString(callexpr->getImplicitObjectArgument());
    if (rewriter != nullptr)
        replaceText(range, "/* LOPLUGIN:FRACTREPLACE check if C-Style cast to double exists and remove it */ boost::rational_cast<double>(" + implicitObject + ")");
    else
        report(DiagnosticsEngine::Warning,
               "replace " + implicitObject + " with boost::rational_cast<double>(" + implicitObject + ") - remove C-Style cast to double if exists", begin)
            << range;
}

void FractReplace::foundMethod_operator_long(const SourceLocation &begin,
                                             const SourceRange &range,
                                             const Expr *) {
    // handle: long fScaleX(xFact);
    if (rewriter != nullptr)
        insertText(begin, "/* LOPLUGIN:FRACTREPLACE i don't know how to "
                          "handle this: 'operator long' */");
    else
        report(DiagnosticsEngine::Warning,
               "i don't know how to handle this: operator long", begin)
            << range;
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

    if (rewriter != nullptr)
        replaceText(range, newtype);
    else
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

bool FractReplace::ignoreByLocationOrFile(const Decl *decl) {
    return ignoreLocation(decl) || ignoreFile(decl, FRACT_HEADER) ||
           ignoreFile(decl, FRACT_CODE);
}

bool FractReplace::ignoreByLocationOrFile(const Stmt *stmt) {
    return ignoreLocation(stmt) || ignoreFile(stmt, FRACT_HEADER) ||
           ignoreFile(stmt, FRACT_CODE);
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
