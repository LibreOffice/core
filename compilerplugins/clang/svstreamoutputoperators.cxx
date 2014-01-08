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
This is a rewriter.

It changes the SvStream operator<< calls into calls to more explicitly named
methods, which reduces the casting needed, and makes it less likely that we
will accidentally write data to a file using the wrong data-type-size.

TODO we don't currently cope with macro expansion e.g. if the constant on the RHS is a #define

TODO we don't currently cope with code like "(*this) << 1;"

TODO we don't currently cope with code like "aStream << x << endl;" the "endl" parts ends up dangling.

TODO we don't currently cope with custom overloads of operator<< in some of the use-sites.
*/

#include "plugin.hxx"
#include <clang/Lex/Lexer.h>
#include <iostream>

namespace loplugin
{

class SvStreamOutputOperators
    : public RecursiveASTVisitor< SvStreamOutputOperators >
    , public RewritePlugin
{
    public:
        explicit SvStreamOutputOperators( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitCallExpr( const CallExpr* call );
   private:
        SourceLocation after(const SourceLocation& loc);
};

SvStreamOutputOperators::SvStreamOutputOperators( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
{
}

void SvStreamOutputOperators::run()
{
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
}

bool SvStreamOutputOperators::VisitCallExpr( const CallExpr* callExpr )
{
    if( ignoreLocation( callExpr ))
        return true;
    if( callExpr->getNumArgs() < 2 )
        return true;
    const FunctionDecl* func = dyn_cast_or_null< FunctionDecl >( callExpr->getCalleeDecl() );
    if ( func == NULL )
        return true;
    if( func->getNumParams() != 1 )
        return true;
    string qualifiedName = func->getQualifiedNameAsString();
    if( qualifiedName != "SvStream::operator<<" )
        return true;

    string arg0 = func->getParamDecl( 0 )->getType().getAsString();
    string newWriteMethod;
    if( arg0 == "sal_uInt16" )
        newWriteMethod = "WriteUInt16";
    else if( arg0 == "sal_uInt32" )
        newWriteMethod = "WriteUInt32";
    else if( arg0 == "sal_uInt64" )
        newWriteMethod = "WriteUInt64";
    else if( arg0 == "sal_Int16" )
        newWriteMethod = "WriteInt16";
    else if( arg0 == "sal_Int32" )
        newWriteMethod = "WriteInt32";
    else if( arg0 == "sal_Int64" )
        newWriteMethod = "WriteInt64";
    else if( arg0 == "sal_uInt8" )
        newWriteMethod = "WriteUInt8";
    else if( arg0 == "sal_Unicode" )
        newWriteMethod = "WriteUnicode";
    else if( arg0 == "rtl::OString" )
        newWriteMethod = "WriteOString";
    else if( arg0 == "bool" )
        newWriteMethod = "WriteBool";
    else if( arg0 == "signed char" )
        newWriteMethod = "WriteSChar";
    else if( arg0 == "char" )
        newWriteMethod = "WriteChar";
    else if( arg0 == "unsigned char" )
        newWriteMethod = "WriteUChar";
    else if( arg0 == "float" )
        newWriteMethod = "WriteFloat";
    else if( arg0 == "double" )
        newWriteMethod = "WriteDouble";
    else if( arg0 == "const double &" )
        newWriteMethod = "WriteDouble";
    else if( arg0 == "const char *" )
        newWriteMethod = "WriteCharPtr";
    else if( arg0 == "char *" )
        newWriteMethod = "WriteCharPtr";
    else if( arg0 == "const unsigned char *" )
        newWriteMethod = "WriteUCharPtr";
    else if( arg0 == "unsigned char *" )
        newWriteMethod = "WriteUCharPtr";
    else if( arg0 == "class SvStream &" )
        newWriteMethod = "WriteStream";
    else
    {
        report( DiagnosticsEngine::Warning,
                "found call to operator<< that I cannot convert with type: " + arg0,
                callExpr->getLocStart());
        return true;
    }

    // CallExpr overrides the children() method from Stmt, but not the const variant of it, so we need to cast const away.
    StmtRange range = const_cast<CallExpr*>(callExpr)->children();
    const Stmt* child1 = *range; // ImplicitCastExpr
    ++range;
    const Stmt* child2 = *range; // ImplicitCastExpr

    if( dyn_cast_or_null< UnaryOperator >( child2 ) != NULL )
    {
        // remove the "*" before the stream variable
        if( !replaceText( callExpr->getLocStart(), 1, "" ) )
            return true;
        if( !replaceText( child1->getLocStart().getLocWithOffset(-1), 4, "->" ) )
            return true;
    }
    else
    {
        if( !replaceText( child1->getLocStart().getLocWithOffset(-1), 4, "." ) )
            return true;
    }

    if( !insertTextBefore( callExpr->getArg( 1 )->getLocStart(), newWriteMethod + "( " ) )
        return true;
    if( !insertTextAfter( after( callExpr->getLocEnd() ), " )" ) )
        return true;

//TODO for some reason this is currently removing too much text
    // if there was a cast e.g. "r << (sal_Int32) 1", then remove the cast
//    const CStyleCastExpr* cast = dyn_cast_or_null< CStyleCastExpr >( callExpr->getArg(1) );
//    if (cast != NULL)
//    {
//        replaceText( SourceRange( cast->getLParenLoc(), cast->getRParenLoc() ), "" );
//    }

    // if there was already parentheses around the expression, remove them
    const ParenExpr* paren = dyn_cast_or_null< ParenExpr >( callExpr->getArg(1) );
    if (paren != NULL)
    {
        if( !replaceText( paren->getLocStart(), 1, "" ) )
             return true;
        if( !replaceText( paren->getLocEnd(), 1, "" ) )
            return true;
    }

//    report( DiagnosticsEngine::Note, "found", callExpr->getLocStart());
    return true;
}

SourceLocation SvStreamOutputOperators::after( const SourceLocation& loc )
{
    return Lexer::getLocForEndOfToken( loc, 0, compiler.getASTContext().getSourceManager(), compiler.getASTContext().getLangOpts() );
}


static Plugin::Registration< SvStreamOutputOperators > X( "svstreamoutputoperators" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
