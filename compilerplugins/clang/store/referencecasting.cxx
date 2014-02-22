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
  http:

*/

ReferenceCasting::ReferenceCasting( CompilerInstance& compiler )
    : Plugin( compiler )
{
}

void ReferenceCasting::run()
{
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
}



















//
//






























static const Type* extractTemplateType(Expr* cce);

bool ReferenceCasting::VisitCXXConstructExpr( CXXConstructExpr* cce )
{
    
    if( compiler.getSourceManager().getFilename( cce->getSourceRange().getBegin() ).find( "Reference.h" ) != StringRef::npos )
        return true;

    
    if( cce->getConstructor()->getNameInfo().getName().getAsString() != "Reference" )
        return true;

    if( cce->getNumArgs() != 2 )
        return true;

    
    const Type * templateParamType = extractTemplateType(cce);
    if ( !templateParamType )
        return true;

    
    Expr* constructorArg0 = cce->getArg(0);
    if( !constructorArg0 )
       return true;

    
    if( constructorArg0->getType()->isPointerType() )
       return true;

    
    DeclRefExpr* constructorSubArg2;
    Expr* constructorArg0SubExpr = constructorArg0;
    for(;;)
    {
        
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
                cce->getLocStart()) 
                    << cce->getSourceRange(); 

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

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
