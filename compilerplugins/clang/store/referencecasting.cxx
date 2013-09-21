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

#include "referencecasting.hxx"

#include <clang/AST/Attr.h>
#include <iostream>

namespace loplugin
{

/*
This is a compile-time checker.

Check for cases where we have
 - two IDL interfaces A and B,
 - B extends A
 - we are converting a Reference<B> to a Reference<A>

Note that it generates the occasional false positive.

Also, it makes clang3.2 crash on about 4 files in the LO codebase.
I have logged a bug here:
  http://llvm.org/bugs/show_bug.cgi?id=15902

*/

ReferenceCasting::ReferenceCasting( CompilerInstance& compiler )
    : Plugin( compiler )
{
}

void ReferenceCasting::run()
{
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
}

// This:
// static void example_method()
// {
//     css::uno::Reference<B> b;
//     css::uno::Reference<A>(b, css::uno::UNO_QUERY);
// }
// Compiles to this AST:
// (CompoundStmt 0x205d430 </noel-extra1/libo-clang/compilerplugins/clang/noel1.cxx:17:1, line:20:1>
//   (DeclStmt 0x20580a8 <line:18:5, col:32>
//     (0x20530e0 "css::uno::Reference<B> refB =
//       (CXXConstructExpr 0x2058078 <col:28> 'css::uno::Reference<B>':'class com::sun::star::uno::Reference<class B>''void (void)')"))
//   (DeclStmt 0x205d418 <line:19:5, col:59>
//     (0x2058310 "css::uno::Reference<A> refA =
//       (CXXConstructExpr 0x205d3d8 <col:28, col:58> 'css::uno::Reference<A>':'class com::sun::star::uno::Reference<class A>''void (const class com::sun::star::uno::BaseReference &, enum com::sun::star::uno::UnoReference_Query)'
//         (ImplicitCastExpr 0x205d3c0 <col:33> 'const class com::sun::star::uno::BaseReference' lvalue <NoOp>
//           (ImplicitCastExpr 0x205d3a0 <col:33> 'class com::sun::star::uno::BaseReference' lvalue <DerivedToBase (BaseReference)>
//             (DeclRefExpr 0x20582a0 <col:33> 'css::uno::Reference<B>':'class com::sun::star::uno::Reference<class B>' lvalue Var 0x20530e0 'refB' 'css::uno::Reference<B>':'class com::sun::star::uno::Reference<class B>')))
//         (DeclRefExpr 0x2058398 <col:39, col:49> 'enum com::sun::star::uno::UnoReference_Query' EnumConstant 0x1831de0 'UNO_QUERY' 'enum com::sun::star::uno::UnoReference_Query'))")))
//
//
// This:
// static void example_method1(css::uno::Reference<A>)
// {
// }
// static void example_method2()
// {
//     css::uno::Reference<B> refB;
//     example_method1(css::uno::Reference<A>(refB, css::uno::UNO_QUERY));
// }
//  Compiles to this AST:
// static void example_method1(css::uno::Reference<A>) (CompoundStmt 0x2a74ee8 </noel-extra1/libo-clang/compilerplugins/clang/noel1.cxx:17:1, line:18:1>)
// static void example_method2() (CompoundStmt 0x2a7a650 </noel-extra1/libo-clang/compilerplugins/clang/noel1.cxx:21:1, line:24:1>
//  (DeclStmt 0x2a7a1a8 <line:22:5, col:32>
//    (0x2a751e0 "css::uno::Reference<B> refB =
//      (CXXConstructExpr 0x2a7a178 <col:28> 'css::uno::Reference<B>':'class com::sun::star::uno::Reference<class B>''void (void)')"))
//  (ExprWithCleanups 0x2a7a638 <line:23:5, col:70> 'void'
//    (CallExpr 0x2a7a570 <col:5, col:70> 'void'
//      (ImplicitCastExpr 0x2a7a558 <col:5> 'void (*)(css::uno::Reference<A>)' <FunctionToPointerDecay>
//        (DeclRefExpr 0x2a7a4d8 <col:5> 'void (css::uno::Reference<A>)' lvalue Function 0x2a6ff00 'example_method1' 'void (css::uno::Reference<A>)'))
//      (CXXBindTemporaryExpr 0x2a7a618 <col:21, col:69> 'css::uno::Reference<A>':'class com::sun::star::uno::Reference<class A>' (CXXTemporary 0x2a7a610)
//        (CXXConstructExpr 0x2a7a5d8 <col:21, col:69> 'css::uno::Reference<A>':'class com::sun::star::uno::Reference<class A>''void (const Reference<class A> &)' elidable
//          (MaterializeTemporaryExpr 0x2a7a5c0 <col:21, col:69> 'const Reference<class A>':'const class com::sun::star::uno::Reference<class A>' lvalue
//            (ImplicitCastExpr 0x2a7a5a8 <col:21, col:69> 'const Reference<class A>':'const class com::sun::star::uno::Reference<class A>' <NoOp>
//              (CXXBindTemporaryExpr 0x2a7a4b8 <col:21, col:69> 'css::uno::Reference<A>':'class com::sun::star::uno::Reference<class A>' (CXXTemporary 0x2a7a4b0)
//                (CXXTemporaryObjectExpr 0x2a7a460 <col:21, col:69> 'css::uno::Reference<A>':'class com::sun::star::uno::Reference<class A>''void (const class com::sun::star::uno::BaseReference &, enum com::sun::star::uno::UnoReference_Query)'
//                  (ImplicitCastExpr 0x2a7a448 <col:44> 'const class com::sun::star::uno::BaseReference' lvalue <NoOp>
//                    (ImplicitCastExpr 0x2a7a428 <col:44> 'class com::sun::star::uno::BaseReference' lvalue <DerivedToBase (BaseReference)>
//                      (DeclRefExpr 0x2a7a340 <col:44> 'css::uno::Reference<B>':'class com::sun::star::uno::Reference<class B>' lvalue Var 0x2a751e0 'refB' 'css::uno::Reference<B>':'class com::sun::star::uno::Reference<class B>')))
//                  (DeclRefExpr 0x2a7a398 <col:50, col:60> 'enum com::sun::star::uno::UnoReference_Query' EnumConstant 0x224ee20 'UNO_QUERY' 'enum com::sun::star::uno::UnoReference_Query'))))))))))

static const Type* extractTemplateType(Expr* cce);

bool ReferenceCasting::VisitCXXConstructExpr( CXXConstructExpr* cce )
{
    // don't bother processing anything in the Reference.h file. Makes my life easier when debugging this.
    if( compiler.getSourceManager().getFilename( cce->getSourceRange().getBegin() ).find( "Reference.h" ) != StringRef::npos )
        return true;

    // look for calls to the Reference<T>(x,UNO_something) constructor
    if( cce->getConstructor()->getNameInfo().getName().getAsString() != "Reference" )
        return true;

    if( cce->getNumArgs() != 2 )
        return true;

    // extract the type parameter passed to the template
    const Type * templateParamType = extractTemplateType(cce);
    if ( !templateParamType )
        return true;

    // extract the type of the first parameter passed to the constructor
    Expr* constructorArg0 = cce->getArg(0);
    if( !constructorArg0 )
       return true;

    // ignore the Reference(XInterface*,...) constructor
    if( constructorArg0->getType()->isPointerType() )
       return true;

    // drill down the expression tree till we hit the bottom
    DeclRefExpr* constructorSubArg2;
    Expr* constructorArg0SubExpr = constructorArg0;
    for(;;)
    {
        // if we've hit the member expression we want, break
        constructorSubArg2 = dyn_cast<DeclRefExpr>( constructorArg0SubExpr );
        if( constructorSubArg2 )
            break;
        CastExpr* tmp1 = dyn_cast<CastExpr>( constructorArg0SubExpr );
        if( tmp1 ) {
            constructorArg0SubExpr = tmp1->getSubExpr();
            continue;
        }
        MaterializeTemporaryExpr* tmp2 = dyn_cast<MaterializeTemporaryExpr>( constructorArg0SubExpr );
        if( tmp2 ) {
            constructorArg0SubExpr = tmp2->GetTemporaryExpr();
            continue;
        }
        CXXBindTemporaryExpr* tmp3 = dyn_cast<CXXBindTemporaryExpr>( constructorArg0SubExpr );
        if( tmp3 ) {
            constructorArg0SubExpr = tmp3->getSubExpr();
            continue;
        }
        CXXTemporaryObjectExpr* tmp4 = dyn_cast<CXXTemporaryObjectExpr>( constructorArg0SubExpr );
        if( tmp4 ) {
            constructorArg0SubExpr = tmp4->getArg(0);
            continue;
        }
        return true;
    }

    const Type * tmp3 = extractTemplateType( constructorSubArg2 );
    if ( !tmp3 )
        return true;

    const RecordType* templateParamRT = dyn_cast<RecordType>( templateParamType );
    const RecordType* constructorArgRT = dyn_cast<RecordType>( tmp3 );
    if( !templateParamRT || !constructorArgRT )
       return true;

    CXXRecordDecl* templateParamRD = dyn_cast<CXXRecordDecl>( templateParamRT->getDecl() );
    CXXRecordDecl* constructorArgRD = dyn_cast<CXXRecordDecl>( constructorArgRT->getDecl() );

    if (constructorArgRD->Equals(templateParamRD) || constructorArgRD->isDerivedFrom(templateParamRD))
        report( DiagnosticsEngine::Warning,
                "the source reference is already a subtype of the destination reference",
                cce->getLocStart()) // and the exact position where the message should point
                    << cce->getSourceRange(); // and the full return statement to highlight (optional)

    return true;
}

static const Type* extractTemplateType(Expr* cce)
{
    QualType cceQT = cce->getType();
    const Type* cceType = cceQT.getTypePtr();
    const TemplateSpecializationType* cceTST = dyn_cast<TemplateSpecializationType>( cceType );
    if( !cceTST )
        return NULL;
    if( cceTST->getNumArgs() != 1 )
        return NULL;
    const TemplateArgument & cceTA = cceTST->getArg(0);
    QualType templateParamQT = cceTA.getAsType();
    return templateParamQT.getTypePtr();
}


static Plugin::Registration< ReferenceCasting > X( "referencecasting" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
