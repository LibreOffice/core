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
/**
  This is a check/rewriter plugin.
  Helps refactor to convert Fraction in boost::rational<long>, see fdo#81356.

  Notes
  =====
  * Fraction::Fraction(double) has not a direct equivalent in boost::rational.
    We use the function

        boost::rational<long> rational_FromDouble(double);

    for doing the conversion

  * Fraction::ReduceInaccurate(unsigned) has not a direct equivalent in
    boost::rational. We use the function

        void rational_ReduceInaccurate(boost::rational<long>&, unsigned);

    as equivalent.

  * Fraction::IsValid() has not a direct equivalent in boost::rational due we
    can't instantiate boost::rational if the numerator and denominator forms an
    invalid fraction. The plugin insert a comment with the text
    LOPLUGIN:FRACTREPLACE to indicate the need of manual edit.

  * Fraction::long() or Fraction::double() has not a direct equivalent in
    boost::rational. We use

        boost::rational_cast<long>()

    and

        boost::rational_cast<double>()

    for those cases.

  Known issues
  ============
  The plugin not work in all cases.

  * In a macro definition:

        #define SCALEPOINT(frac) (frac.GetNumerator())

  * In a macro expansion:

        #define DBG_ASSERT(cond) ...

        DBG_ASSERT( f.GetNumerator());

  * Detecting the constructor Fraction::Fraction(double expr) and rewrite as
    rational_FromDouble(expr);

  * In expressions like

      long l = Fraction(1) * f; // f is a Fraction

    the rewriter fails producing a double code replacement.
    This need be corrected manually using "git diff" for detect the occurrence

  Usage as rewriter tool
  ======================
  1. Convert cxx files:
    $ make COMPILER_PLUGIN_TOOL=fractreplace
  2. Convert hxx files:
    $ make COMPILER_PLUGIN_TOOL=fractreplace UPDATE_FILES=all
  3. Find code for manual modifications:
    $ git grep "LOPLUGIN:FRACTREPLACE"
  4. Check the replacements and fix when needed:
    $ git diff

*/
#include "plugin.hxx"
#include "compat.hxx"

#include <functional>
#include <map>
#include <string>

namespace loplugin {

class FractReplace : public RecursiveASTVisitor<FractReplace>,
                     public RewritePlugin {
  public:
    FractReplace(const InstantiationData &data);

    // visit methods
    virtual void run() override;
    bool TraverseDeclStmt(DeclStmt *decl);
    bool VisitParmVarDecl(const ParmVarDecl *decl);
    bool VisitCXXTemporaryObjectExpr(const CXXTemporaryObjectExpr *toe);
    bool VisitCXXMemberCallExpr(const CXXMemberCallExpr *cmce);
    bool VisitFieldDecl(const FieldDecl *fd);
    bool VisitFunctionDecl(const FunctionDecl *fd);
    bool VisitCStyleCastExpr(const CStyleCastExpr *expr);
    bool VisitCXXFunctionalCastExpr(const CXXFunctionalCastExpr *expr);

  private:
    const std::string FRACT_HEADER{ "fract.hxx" };
    const std::string FRACT_CODE{ "fract.cxx" };
    bool touch_cast = false; // used for ignore CXXMemberCallExpr 'operator
                             // double' or 'operator long' that follows
                             // VisitCStyleCastExpr/VisitCXXFunctionalCastExpr
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
        { "operator long", &FractReplace::foundMethod_operator_long }
    };

    bool isCast(const ImplicitCastExpr *expr, std::string &name);
    void handle_cast(const Expr *expr, std::string replacewith,
                     std::string message);
    void handle_operator(const Expr *expr, std::string replacewith,
                         std::string message);
    void foundTypeFraction(const SourceLocation &begin,
                           const SourceRange &range, const std::string &from,
                           const QualType *type = nullptr);

    void
    foundCstyleCast_operator_double(const CStyleCastExpr *expr_CStyleCast,
                                    const CXXMemberCallExpr *expr_CXXMembCall);
    void
    foundCstyleCast_operator_long(const CStyleCastExpr *expr_CStyleCast,
                                  const CXXMemberCallExpr *expr_CXXMembCall);
    void foundCXXFunctionalCast_operator_double(
        const CXXFunctionalCastExpr *expr_CXXFunctionalCast,
        const CXXMemberCallExpr *expr_CXXMemberCall);
    void foundCXXFunctionalCast_operator_long(
        const CXXFunctionalCastExpr *expr_CXXFunctionalCast,
        const CXXMemberCallExpr *expr_CXXMemberCall);

    void foundMethod_GetNumerator(const SourceLocation &begin,
                                  const SourceRange &range, const Expr *);
    void foundMethod_GetDenominator(const SourceLocation &begin,
                                    const SourceRange &range, const Expr *);
    void foundMethod_IsValid(const SourceLocation &, const SourceRange &,
                             const Expr *expr);
    void foundMethod_long(const SourceLocation &begin, const SourceRange &range,
                          const Expr *expr);
    void foundMethod_double(const SourceLocation &begin,
                            const SourceRange &range, const Expr *expr);
    void foundMethod_ReduceInaccurate(const SourceLocation &,
                                      const SourceRange &, const Expr *);
    void foundMethod_operator_double(const SourceLocation &,
                                     const SourceRange &, const Expr *expr);
    void foundMethod_operator_long(const SourceLocation &, const SourceRange &,
                                   const Expr *expr);

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

FractReplace::FractReplace(const Plugin::InstantiationData &data)
    : RewritePlugin(data) {}

void FractReplace::run() {
    TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
}

bool FractReplace::TraverseDeclStmt(DeclStmt *decl) {
    if (!ignoreByLocationOrFile(decl)) {
        auto it = decl->decl_begin();
        if (VarDecl *vardecl = dyn_cast<VarDecl>(*it)) {
            QualType type = vardecl->getType();
            if (typeIsFraction(type)) {
                if (const TypeSourceInfo *TSI = vardecl->getTypeSourceInfo()) {
                    foundTypeFraction(TSI->getTypeLoc().getBeginLoc(),
                                      TSI->getTypeLoc().getSourceRange(),
                                      "TraverseDeclStmt", &type);
                }
                return true; // only process the first VarDecl
            }
        }
    }
    return RecursiveASTVisitor::TraverseDeclStmt(decl);
}

bool FractReplace::VisitParmVarDecl(const ParmVarDecl *decl) {
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
        report(DiagnosticsEngine::Error,
               "Can't handle method '" + name + "'", info.getLocStart())
            << info.getSourceRange();
    }
    touch_cast = false;
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
        if (const ImplicitCastExpr *expr_ImplCast =
                dyn_cast<ImplicitCastExpr>(expr_CStyleCast->getSubExpr())) {
            std::string name;
            if (isCast(expr_ImplCast, name)) {
                touch_cast = true;
                const CXXMemberCallExpr *expr_CXXMembCall =
                    dyn_cast<CXXMemberCallExpr>(expr_ImplCast->getSubExpr());
                if (name == "operator double") {
                    foundCstyleCast_operator_double(expr_CStyleCast,
                                                    expr_CXXMembCall);
                    return true;
                }
                if (name == "operator long") {
                    foundCstyleCast_operator_long(expr_CStyleCast,
                                                  expr_CXXMembCall);
                    return true;
                }
                report(DiagnosticsEngine::Error, "Can't handle '" + name + "'");
            }
        }
    }
    return true;
}

bool FractReplace::VisitCXXFunctionalCastExpr(
    const CXXFunctionalCastExpr *expr_CXXFunctionalCast) {
    if (ignoreByLocationOrFile(expr_CXXFunctionalCast))
        return true;
    if (CK_NoOp == expr_CXXFunctionalCast->getCastKind()) {
        if (const ImplicitCastExpr *expr_ImplCast = dyn_cast<ImplicitCastExpr>(
                expr_CXXFunctionalCast->getSubExpr())) {
            std::string name;
            if (isCast(expr_ImplCast, name)) {
                touch_cast = true;
                const CXXMemberCallExpr *expr_CXXMembCall =
                    dyn_cast<CXXMemberCallExpr>(expr_ImplCast->getSubExpr());
                if (name == "operator double") {
                    foundCXXFunctionalCast_operator_double(
                        expr_CXXFunctionalCast, expr_CXXMembCall);
                    return true;
                }
                if (name == "operator long") {
                    foundCXXFunctionalCast_operator_long(expr_CXXFunctionalCast,
                                                         expr_CXXMembCall);
                    return true;
                }
                report(DiagnosticsEngine::Error, "Can't handle '" + name + "'");
            }
        }
    }
    return true;
}

bool FractReplace::isCast(const ImplicitCastExpr *expr, std::string &name) {
    if (CK_UserDefinedConversion == expr->getCastKind()) {
        if (const CXXMemberCallExpr *expr_CXXMembCall =
                dyn_cast<CXXMemberCallExpr>(expr->getSubExpr())) {
            if (recordIsFraction(expr_CXXMembCall)) {
                const MemberExpr *expr_Memb =
                    dyn_cast<MemberExpr>(expr_CXXMembCall->getCallee());
                const DeclarationNameInfo &info =
                    expr_Memb->getMemberNameInfo();
                name = info.getAsString();
                return true;
            }
        }
    }
    return false;
}

void FractReplace::handle_cast(const Expr *expr, std::string replacewith,
                               std::string message) {
    const SourceLocation &begin = expr->getExprLoc();
    const SourceRange &range = expr->getSourceRange();
    if (rewriter != nullptr)
        replaceText(range, replacewith);
    else
        report(DiagnosticsEngine::Warning, message, begin) << range;
}

// FIXME: avoid use of handle_cast with convertToString
void FractReplace::foundCstyleCast_operator_double(
    const CStyleCastExpr *expr_CStyleCast,
    const CXXMemberCallExpr *expr_CXXMemberCall) {
    std::string implicitObject =
        convertToString(expr_CXXMemberCall->getImplicitObjectArgument());
    handle_cast(expr_CStyleCast,
                "boost::rational_cast<double>(" + implicitObject + ")",
                "replace c-style cast with 'boost::rational_cast<double>(" +
                    implicitObject + ")'");
}

void FractReplace::foundCstyleCast_operator_long(
    const CStyleCastExpr *expr_CStyleCast,
    const CXXMemberCallExpr *expr_CXXMemberCall) {
    std::string implicitObject =
        convertToString(expr_CXXMemberCall->getImplicitObjectArgument());
    handle_cast(expr_CStyleCast,
                "boost::rational_cast<long>(" + implicitObject + ")",
                "replace c-style cast with 'boost::rational_cast<long>(" +
                    implicitObject + ")'");
}

void FractReplace::foundCXXFunctionalCast_operator_double(
    const CXXFunctionalCastExpr *expr_CXXFunctionalCast,
    const CXXMemberCallExpr *expr_CXXMemberCall) {
    std::string implicitObject =
        convertToString(expr_CXXMemberCall->getImplicitObjectArgument());
    handle_cast(
        expr_CXXFunctionalCast,
        "boost::rational_cast<double>(" + implicitObject + ")",
        "replace cxx functional cast with 'boost::rational_cast<double>(" +
            implicitObject + ")'");
}

void FractReplace::foundCXXFunctionalCast_operator_long(
    const CXXFunctionalCastExpr *expr_CXXFunctionalCast,
    const CXXMemberCallExpr *expr_CXXMemberCall) {
    std::string implicitObject =
        convertToString(expr_CXXMemberCall->getImplicitObjectArgument());
    handle_cast(
        expr_CXXFunctionalCast,
        "boost::rational_cast<long>(" + implicitObject + ")",
        "replace cxx functional cast with 'boost::rational_cast<long>(" +
            implicitObject + ")'");
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
        insertText(begin, "/* LOPLUGIN:FRACTREPLACE manual change is needed "
                          "for rewrite the whole expression using "
                          "'boost::rational_cast<long>(expr)' */");
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
                          "for rewrite the whole expression using "
                          "'boost::rational_cast<double>(expr)' */");
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
        replaceText(range, "rational_ReduceInaccurate(" + implicitObject + ", " +
                               argument + ")");
    else
        report(DiagnosticsEngine::Warning,
               "replace '" + implicitObject + ".ReduceInaccurate(" + argument +
                   ")' with 'rational_ReduceInaccurate(" + implicitObject + ", " +
                   argument + ")'",
               begin)
            << range;
}

void FractReplace::handle_operator(const Expr *expr, std::string replacewith,
                                   std::string message) {
    const SourceLocation &begin = expr->getExprLoc();
    const SourceRange &range = expr->getSourceRange();
    if (rewriter != nullptr)
        replaceText(range, replacewith);
    else
        report(DiagnosticsEngine::Warning, message, begin) << range;
}

void FractReplace::foundMethod_operator_double(const SourceLocation &,
                                               const SourceRange &,
                                               const Expr *expr) {
    if (touch_cast)
        return;
    const CXXMemberCallExpr *callexpr = dyn_cast<CXXMemberCallExpr>(expr);
    std::string implicitObject =
        convertToString(callexpr->getImplicitObjectArgument());
    handle_operator(
        expr, "boost::rational_cast<double>(" + implicitObject + ")",
        "replace " + implicitObject + " with boost::rational_cast<double>(" +
            implicitObject + ")");
}

// FIXME: issues with implicitObject
// Example:
// void test_assign_and_create(long &l, const Fraction &f) {
//   l = Fraction(1, 1) * f; // similar to svx/source/form/fmvwimp.cxx:1453
// }
void FractReplace::foundMethod_operator_long(const SourceLocation &,
                                             const SourceRange &,
                                             const Expr *expr) {
    if (touch_cast)
        return;
    const CXXMemberCallExpr *callexpr = dyn_cast<CXXMemberCallExpr>(expr);
    std::string implicitObject =
        convertToString(callexpr->getImplicitObjectArgument());
    handle_operator(
        expr, "/* LOPLUGIN:FRACTREPLACE check if cast to long exists and "
              "remove it */ boost::rational_cast<long>(" +
                  implicitObject + ")",
        "replace " + implicitObject + " with boost::rational_cast<long>(" +
            implicitObject + ") - remove cast to long if exists");
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

static Plugin::Registration<FractReplace> X("fractreplace", true);

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
