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
This is a compile check.

Warns about 'auto' declarations becoming rtl::OUStringConcat, such as
auto str = "string" + OUString::number( 10 );
The type of the expression is rtl::OUStringConcat and those refer to temporaries
and so their lifecycle should not extend the lifecycle of those temporaries.
*/

#ifndef LO_CLANG_SHARED_PLUGINS

#include "config_clang.h"

#include "plugin.hxx"
#include "check.hxx"

namespace loplugin
{

class StringConcatAuto
    : public FilteringPlugin< StringConcatAuto >
    {
    public:
        StringConcatAuto( const InstantiationData& data );
        virtual void run() override;
        bool shouldVisitTemplateInstantiations () const { return true; }
        bool VisitVarDecl( const VarDecl* decl );
        bool VisitFunctionDecl( const FunctionDecl* decl );
    private:
        enum class Check { Var, Return };
        bool checkDecl( const DeclaratorDecl* decl, const QualType type, const SourceRange& range, Check check );
    };

StringConcatAuto::StringConcatAuto( const InstantiationData& data )
    : FilteringPlugin( data )
    {
    }

void StringConcatAuto::run()
    {
    TraverseDecl( compiler.getASTContext().getTranslationUnitDecl());
    }

bool StringConcatAuto::VisitVarDecl( const VarDecl* decl )
    {
    return checkDecl( decl, decl->getType(),
        decl->getTypeSourceInfo()
            ? decl->getTypeSourceInfo()->getTypeLoc().getSourceRange()
            : decl->getSourceRange(),
        Check::Var );
    }

bool StringConcatAuto::VisitFunctionDecl( const FunctionDecl* decl )
    {
    return checkDecl( decl, decl->getReturnType(), decl->getReturnTypeSourceRange(), Check::Return );
    }

bool StringConcatAuto::checkDecl( const DeclaratorDecl* decl, QualType type, const SourceRange& range, Check check )
    {
    if( ignoreLocation( decl ))
        return true;
    if( isa< ParmVarDecl >( decl )) // parameters should be fine, temporaries should exist during the call
        return true;
    std::string fileName = getFilenameOfLocation(
        compiler.getSourceManager().getSpellingLoc(decl->getBeginLoc())).str();
    loplugin::normalizeDotDotInFilePath(fileName);
    if (loplugin::isSamePathname(fileName, SRCDIR "/include/rtl/string.hxx")
        || loplugin::isSamePathname(fileName, SRCDIR "/include/rtl/ustring.hxx")
        || loplugin::isSamePathname(fileName, SRCDIR "/include/rtl/strbuf.hxx")
        || loplugin::isSamePathname(fileName, SRCDIR "/include/rtl/ustrbuf.hxx")
        || loplugin::isSamePathname(fileName, SRCDIR "/include/rtl/stringconcat.hxx"))
        return true;
    auto const tc = loplugin::TypeCheck( type.getNonReferenceType().getCanonicalType());
    const char* typeString = nullptr;
    if( tc.Struct("StringConcat").Namespace("rtl").GlobalNamespace())
        typeString = "O(U)String";
    else
        return true;
    report( DiagnosticsEngine::Warning,
        check == Check::Var
            ? "creating a variable of type %0 will make it reference temporaries"
            : "returning a variable of type %0 will make it reference temporaries",
        decl->getLocation())
        << type;
    report( DiagnosticsEngine::Note,
        "use %0 instead",
        range.getBegin())
        << typeString
        << FixItHint::CreateReplacement( range, typeString );
    return true;
    }

static Plugin::Registration< StringConcatAuto > stringconcatauto( "stringconcatauto" );

} // namespace

#endif // LO_CLANG_SHARED_PLUGINS

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
