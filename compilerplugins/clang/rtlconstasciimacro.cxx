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

Remove uses of the macro RTL_CONSTASCII_USTRINGPARAM. One run is for one
specific use (see below), modify source to remove other uses.
*/

#include "plugin.hxx"

#include <clang/Lex/Preprocessor.h>

namespace loplugin
{

class RtlConstAsciiMacro
    : public RecursiveASTVisitor< RtlConstAsciiMacro >
    , public PPCallbacks
    , public RewritePlugin
    {
    public:
        explicit RtlConstAsciiMacro( CompilerInstance& compiler, Rewriter& rewriter );
        virtual void run() override;
        bool VisitCXXConstructExpr( CXXConstructExpr* expr );
        bool VisitCXXTemporaryObjectExpr( CXXTemporaryObjectExpr* expr );
        bool VisitStringLiteral( const StringLiteral* literal );
#if __clang_major__ < 3 || __clang_major__ == 3 && __clang_minor__ < 3
        virtual void MacroExpands( const Token& macro, const MacroInfo* info, SourceRange range ) override;
#else
        virtual void MacroExpands( const Token& macro, const MacroDirective* directive,
            SourceRange range, const MacroArgs* args ) override;
#endif
        enum { isPPCallback = true };
    private:
        map< SourceLocation, SourceLocation > expansions; // start location -> end location
        bool searchingForString;
        bool suitableString;
    };

RtlConstAsciiMacro::RtlConstAsciiMacro( CompilerInstance& compiler, Rewriter& rewriter )
    : RewritePlugin( compiler, rewriter )
    , searchingForString( false )
    {
    compiler.getPreprocessor().addPPCallbacks( this );
    }

void RtlConstAsciiMacro::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }


#if __clang_major__ < 3 || __clang_major__ == 3 && __clang_minor__ < 3
void RtlConstAsciiMacro::MacroExpands( const Token& macro, const MacroInfo*, SourceRange range )
#else
void RtlConstAsciiMacro::MacroExpands( const Token& macro, const MacroDirective*,
    SourceRange range, const MacroArgs* )
#endif
    {
    if( macro.getIdentifierInfo()->getName() != "RTL_CONSTASCII_USTRINGPARAM" )
        return;
    expansions[ range.getBegin() ] = range.getEnd();
    }

/* Remove use with the following ctor:
    OUString( const sal_Char * value, sal_Int32 length,
              rtl_TextEncoding encoding,
              sal_uInt32 convertFlags = OSTRING_TO_OUSTRING_CVTFLAGS )
   This means searching for CXXConstructExpr.
   For removal when used with functions it should check e.g. for CallExpr.
*/
bool RtlConstAsciiMacro::VisitCXXConstructExpr( CXXConstructExpr* expr )
    {
    if( ignoreLocation( expr ))
        return true;
    if( expr->getNumArgs() != 4 )
        return true;
    // The last argument should be the default one when the macro is used.
    if( dyn_cast< CXXDefaultArgExpr >( expr->getArg( 3 )) == NULL )
        return true;
    if( expr->getConstructor()->getQualifiedNameAsString() != "rtl::OUString::OUString" )
        return true;
    const SourceManager& src = compiler.getSourceManager();
    SourceLocation start = src.getExpansionLoc( expr->getArg( 0 )->getLocStart());
    // Macro fills in the first 3 arguments, so they must all come from the same expansion.
    if( start != src.getExpansionLoc( expr->getArg( 2 )->getLocEnd()))
        return true;
    if( expansions.find( start ) == expansions.end())
        return true;
    SourceLocation end = expansions[ start ];
    // Remove the location, since sometimes the same code may be processed more than once
    // (e.g. non-trivial default arguments).
    expansions.erase( start );
    // Check if the string argument to the macro is suitable.
    searchingForString = true;
    suitableString = false;
    TraverseStmt( expr->getArg( 0 ));
    searchingForString = false;
    if( !suitableString )
        return true;
    // Seach for '(' (don't just remove a given length to handle possible whitespace).
    const char* text = compiler.getSourceManager().getCharacterData( start );
    const char* pos = text;
    while( *pos != '(' )
        ++pos;
    ++pos;
    if( text[ -1 ] == ' ' && *pos == ' ' )
        ++pos; // do not leave two spaces
    removeText( start, pos - text, RemoveLineIfEmpty );
    const char* textend = compiler.getSourceManager().getCharacterData( end );
    if( textend[ -1 ] == ' ' && textend[ 1 ] == ' ' )
        removeText( end, 2, RemoveLineIfEmpty ); // Remove ') '.
    else
        removeText( end, 1, RemoveLineIfEmpty ); // Remove ')'.
    return true;
    }

bool RtlConstAsciiMacro::VisitCXXTemporaryObjectExpr( CXXTemporaryObjectExpr* expr )
    {
    return VisitCXXConstructExpr( expr );
    }

bool RtlConstAsciiMacro::VisitStringLiteral( const StringLiteral* literal )
    {
    if( !searchingForString )
        return true;
    if( suitableString ) // two string literals?
        {
        report( DiagnosticsEngine::Warning, "cannot analyze RTL_CONSTASCII_USTRINGPARAM (plugin needs fixing)" )
            << literal->getSourceRange();
        return true;
        }
    if( !literal->isAscii()) // ignore
        return true;
    if( !literal->containsNonAsciiOrNull())
        suitableString = true;
    return true;
    }

static Plugin::Registration< RtlConstAsciiMacro > X( "rtlconstasciimacro" );

} // namespace
