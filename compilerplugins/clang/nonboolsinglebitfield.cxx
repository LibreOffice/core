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

#include "plugin.hxx"

#include <clang/AST/ASTContext.h>
#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

namespace loplugin
{

/*
This is a compile check.

Look for fields like
struct {
   sal_uInt16 mbDerivedR2L  : 1
}
which should be declared as type bool.
*/

class NonBoolSingleBitField
    : public RecursiveASTVisitor< NonBoolSingleBitField >
    , public Plugin
{
    public:
        explicit NonBoolSingleBitField( const InstantiationData& data );
        void run();
        bool VisitCXXRecordDecl( const CXXRecordDecl* expr );
    private:
        bool isExcludedFile(SourceLocation spellingLocation) const;
};

NonBoolSingleBitField::NonBoolSingleBitField( const InstantiationData& data )
    : Plugin( data )
{
}

void NonBoolSingleBitField::run()
{
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
}

bool NonBoolSingleBitField::VisitCXXRecordDecl( const CXXRecordDecl* RD )
{
    if( ignoreLocation( RD ))
        return true;
    for (RecordDecl::field_iterator it = RD->field_begin(); it != RD->field_end(); ++it)
    {
        FieldDecl *I = *it;
        if (I->isBitField()
            && I->getBitWidthValue(compiler.getASTContext()) == 1
            && !(I->getType().getAsString() == "bool" || I->getType().getAsString() == "_Bool" || I->getType().getAsString() == "const _Bool")
            && !isExcludedFile( RD->getLocStart() ) )
        {
            report( DiagnosticsEngine::Warning,
                "non-bool single bit bit-field", I->getLocStart())
                << I->getSourceRange();
        }
    }

    return true;
}

bool NonBoolSingleBitField::isExcludedFile(SourceLocation spellingLocation) const {
    StringRef name { compiler.getSourceManager().getFilename(spellingLocation) };
    return name == SRCDIR "/include/sal/mathconf.h";
}

static Plugin::Registration< NonBoolSingleBitField > X( "nonboolsinglebitfield" );

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
