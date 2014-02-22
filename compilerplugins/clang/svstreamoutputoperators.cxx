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
        explicit SvStreamOutputOperators( InstantiationData const & data );
        virtual void run() override;
        bool VisitCallExpr( const CallExpr* call );
   private:
        SourceLocation after(const SourceLocation& loc);
};

SvStreamOutputOperators::SvStreamOutputOperators( InstantiationData const & data )
    : RewritePlugin( data )
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
    bool bOutputOperator;
    if( qualifiedName == "SvStream::operator<<" )
      bOutputOperator = true;
    else if( qualifiedName == "SvStream::operator>>" )
      bOutputOperator = false;
    else
        return true;

    string arg0 = func->getParamDecl( 0 )->getType().getAsString();
    string newIOMethod;
    if (bOutputOperator)
    {
        if( arg0 == "sal_uInt16" )
            newIOMethod = "WriteUInt16";
        else if( arg0 == "sal_uInt32" )
            newIOMethod = "WriteUInt32";
        else if( arg0 == "sal_uInt64" )
            newIOMethod = "WriteUInt64";
        else if( arg0 == "sal_Int16" )
            newIOMethod = "WriteInt16";
        else if( arg0 == "sal_Int32" )
            newIOMethod = "WriteInt32";
        else if( arg0 == "sal_Int64" )
            newIOMethod = "WriteInt64";
        else if( arg0 == "sal_uInt8" )
            newIOMethod = "WriteUInt8";
        else if( arg0 == "sal_Unicode" )
            newIOMethod = "WriteUnicode";
        else if( arg0 == "rtl::OString" )
            newIOMethod = "WriteOString";
        else if( arg0 == "bool" )
            newIOMethod = "WriteBool";
        else if( arg0 == "signed char" )
            newIOMethod = "WriteSChar";
        else if( arg0 == "char" )
            newIOMethod = "WriteChar";
        else if( arg0 == "unsigned char" )
            newIOMethod = "WriteUChar";
        else if( arg0 == "float" )
            newIOMethod = "WriteFloat";
        else if( arg0 == "double" )
            newIOMethod = "WriteDouble";
        else if( arg0 == "const double &" )
            newIOMethod = "WriteDouble";
        else if( arg0 == "const char *" )
            newIOMethod = "WriteCharPtr";
        else if( arg0 == "char *" )
            newIOMethod = "WriteCharPtr";
        else if( arg0 == "const unsigned char *" )
            newIOMethod = "WriteUCharPtr";
        else if( arg0 == "unsigned char *" )
            newIOMethod = "WriteUCharPtr";
        else if( arg0 == "class SvStream &" )
            newIOMethod = "WriteStream";
        else
        {
            report( DiagnosticsEngine::Warning,
                    "found call to operator<< that I cannot convert with type: " + arg0,
                    callExpr->getLocStart());
            return true;
        }
    }
    else
    {
        if( arg0 == "sal_uInt16 &" )
            newIOMethod = "ReadUInt16";
        else if( arg0 == "sal_uInt32 &" )
            newIOMethod = "ReadUInt32";
        else if( arg0 == "sal_uInt64 &" )
            newIOMethod = "ReadUInt64";
        else if( arg0 == "sal_Int16 &" )
            newIOMethod = "ReadInt16";
        else if( arg0 == "sal_Int32 &" )
            newIOMethod = "ReadInt32";
        else if( arg0 == "sal_Int64 &" )
            newIOMethod = "ReadInt64";
        else if( arg0 == "sal_uInt8 &" )
            newIOMethod = "ReadUInt8";
        else if( arg0 == "signed char &" )
            newIOMethod = "ReadSChar";
        else if( arg0 == "char &" )
            newIOMethod = "ReadChar";
        else if( arg0 == "unsigned char &" )
            newIOMethod = "ReadUChar";
        else if( arg0 == "float &" )
            newIOMethod = "ReadFloat";
        else if( arg0 == "double &" )
            newIOMethod = "ReadDouble";
        else if( arg0 == "class SvStream &" )
            newIOMethod = "ReadStream";
        else
        {
            report( DiagnosticsEngine::Warning,
                    "found call to operator>> that I cannot convert with type: " + arg0,
                    callExpr->getLocStart());
            return true;
        }
    }

    
    StmtRange range = const_cast<CallExpr*>(callExpr)->children();
    const Stmt* child1 = *range; 
    ++range;
    const Stmt* child2 = *range; 

    if( dyn_cast_or_null< UnaryOperator >( child2 ) != NULL )
    {
        
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

    if( !insertTextBefore( callExpr->getArg( 1 )->getLocStart(), newIOMethod + "( " ) )
        return true;
    if( !insertTextAfter( after( callExpr->getLocEnd() ), " )" ) )
        return true;


    






    
    const ParenExpr* paren = dyn_cast_or_null< ParenExpr >( callExpr->getArg(1) );
    if (paren != NULL)
    {
        if( !replaceText( paren->getLocStart(), 1, "" ) )
             return true;
        if( !replaceText( paren->getLocEnd(), 1, "" ) )
            return true;
    }


    return true;
}

SourceLocation SvStreamOutputOperators::after( const SourceLocation& loc )
{
    return Lexer::getLocForEndOfToken( loc, 0, compiler.getASTContext().getSourceManager(), compiler.getASTContext().getLangOpts() );
}


static Plugin::Registration< SvStreamOutputOperators > X( "svstreamoutputoperators" );

} 

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
