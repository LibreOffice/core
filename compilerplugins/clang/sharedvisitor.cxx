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

#ifndef FOO

#include <config_clang.h>

#include <clang/AST/ASTContext.h>
#include <clang/AST/RecursiveASTVisitor.h>

#include "plugin.hxx"

#define FOO // TODO
#include "badstatics.cxx"
#include "blockblock.cxx"
#include "charrightshift.cxx"
#include "datamembershadow.cxx"
#include "dbgunhandledexception.cxx"
#include "derefnullptr.cxx"
#include "doubleconvert.cxx"
#include "dyncastvisibility.cxx"
#include "empty.cxx"
#include "emptyif.cxx"
#include "externandnotdefined.cxx"
#include "externvar.cxx"
#include "inlinevisible.cxx"
#include "loopvartoosmall.cxx"
#include "privatebase.cxx"
#include "reservedid.cxx"
#include "simplifyconstruct.cxx"
#include "stringstatic.cxx"
#include "subtlezeroinit.cxx"
#include "unnecessaryoverride.cxx"
#include "unnecessaryparen.cxx"
#include "unreffun.cxx"
#include "unusedvariablecheck.cxx"
#include "vclwidgets.cxx"
#include "weakobject.cxx"

using namespace clang;
using namespace llvm;

namespace loplugin
{

/**
*/
class SharedRecursiveASTVisitor
//    : public RecursiveASTVisitor< SharedRecursiveASTVisitor >, public Plugin
    : public FilteringPlugin< SharedRecursiveASTVisitor >
{
public:
    explicit SharedRecursiveASTVisitor(InstantiationData const& rData)
        : FilteringPlugin(rData)
        , badStatics( nullptr )
        , blockBlock( nullptr )
        , charRightShift( nullptr )
        , dataMemberShadow( nullptr )
        , dbgUnhandledException( nullptr )
        , derefNullPtr( nullptr )
        , doubleConvert( nullptr )
        , dynCastVisibility( nullptr )
        , empty( nullptr )
        , emptyIf( nullptr )
        , externAndNotDefined( nullptr )
        , externVar( nullptr )
        , inlineVisible( nullptr )
        , loopVarTooSmall( nullptr )
        , privateBase( nullptr )
        , reservedId( nullptr )
        , simplifyConstruct( nullptr )
        , stringStatic( nullptr )
        , subtleZeroInit( nullptr )
        , unnecessaryOverride( nullptr )
        , unnecessaryParen( nullptr )
        , unrefFun( nullptr )
        , unusedVariableCheck( nullptr )
        , vclWidgets( nullptr )
        , weakObject( nullptr )
        {}
    virtual bool preRun() override
    {
        if( badStatics && !badStatics->preRun())
            badStatics = nullptr;
        if( blockBlock && !blockBlock->preRun())
            blockBlock = nullptr;
        if( charRightShift && !charRightShift->preRun())
            charRightShift = nullptr;
        if( dataMemberShadow && !dataMemberShadow->preRun())
            dataMemberShadow = nullptr;
        if( dbgUnhandledException && !dbgUnhandledException->preRun())
            dbgUnhandledException = nullptr;
        if( derefNullPtr && !derefNullPtr->preRun())
            derefNullPtr = nullptr;
        if( doubleConvert && !doubleConvert->preRun())
            doubleConvert = nullptr;
        if( dynCastVisibility && !dynCastVisibility->preRun())
            dynCastVisibility = nullptr;
        if( empty && !empty->preRun())
            empty = nullptr;
        if( emptyIf && !emptyIf->preRun())
            emptyIf = nullptr;
        if( externAndNotDefined && !externAndNotDefined->preRun())
            externAndNotDefined = nullptr;
        if( externVar && !externVar->preRun())
            externVar = nullptr;
        if( inlineVisible && !inlineVisible->preRun())
            inlineVisible = nullptr;
        if( loopVarTooSmall && !loopVarTooSmall->preRun())
            loopVarTooSmall = nullptr;
        if( privateBase && !privateBase->preRun())
            reservedId = nullptr;
        if( reservedId && !reservedId->preRun())
            reservedId = nullptr;
        if( simplifyConstruct && !simplifyConstruct->preRun())
            simplifyConstruct = nullptr;
        if( stringStatic && !stringStatic->preRun())
            stringStatic = nullptr;
        if( subtleZeroInit && !subtleZeroInit->preRun())
            subtleZeroInit = nullptr;
        if( unnecessaryOverride && !unnecessaryOverride->preRun())
            unnecessaryOverride = nullptr;
        if( unnecessaryParen && !unnecessaryParen->preRun())
            unnecessaryParen = nullptr;
        if( unrefFun && !unrefFun->preRun())
            unrefFun = nullptr;
        if( unusedVariableCheck && !unusedVariableCheck->preRun())
            unusedVariableCheck = nullptr;
        if( vclWidgets && !vclWidgets->preRun())
            vclWidgets = nullptr;
        if( weakObject && !weakObject->preRun())
            weakObject = nullptr;
        return anyPluginActive();
    }
    virtual void postRun() override
    {
        if( badStatics )
            badStatics->postRun();
        if( blockBlock )
            blockBlock->postRun();
        if( charRightShift )
            charRightShift->postRun();
        if( dataMemberShadow )
            dataMemberShadow->postRun();
        if( dbgUnhandledException )
            dbgUnhandledException->postRun();
        if( derefNullPtr )
            derefNullPtr->postRun();
        if( doubleConvert )
            doubleConvert->postRun();
        if( dynCastVisibility )
            dynCastVisibility->postRun();
        if( empty )
            empty->postRun();
        if( emptyIf )
            emptyIf->postRun();
        if( externAndNotDefined )
            externAndNotDefined->postRun();
        if( externVar )
            externVar->postRun();
        if( inlineVisible )
            inlineVisible->postRun();
        if( loopVarTooSmall )
            loopVarTooSmall->postRun();
        if( privateBase )
            privateBase->postRun();
        if( reservedId )
            reservedId->postRun();
        if( simplifyConstruct )
            simplifyConstruct->postRun();
        if( stringStatic )
            stringStatic->postRun();
        if( subtleZeroInit )
            subtleZeroInit->postRun();
        if( unnecessaryOverride )
            unnecessaryOverride->postRun();
        if( unnecessaryParen )
            unnecessaryParen->postRun();
        if( unrefFun )
            unrefFun->postRun();
        if( unusedVariableCheck )
            unusedVariableCheck->postRun();
        if( vclWidgets )
            vclWidgets->postRun();
        if( weakObject )
            weakObject->postRun();
    }
    virtual void run() override {
        if (preRun()) {
//            fprintf(stderr,"SHARED: %p %p %p %p %p %p %p %p %p %p %p %p %p\n",
//                badStatics, blockBlock, charRightShift, dataMemberShadow, dbgUnhandledException,
//                dynCastVisibility, reservedId, simplifyConstruct, stringStatic, unrefFun,
//                unusedVariableCheck, vclWidgets, weakObject);
            TraverseDecl(compiler.getASTContext().getTranslationUnitDecl());
            postRun();
        }
    }
    enum { isSharedPlugin = true };
    virtual bool setSharedPlugin( Plugin* plugin, const char* name ) override
    {
        if( strcmp( name, "badstatics" ) == 0 )
            badStatics = static_cast< BadStatics* >( plugin );
        else if( strcmp( name, "blockblock" ) == 0 )
            blockBlock = static_cast< BlockBlock* >( plugin );
        else if( strcmp( name, "charrightshift" ) == 0 )
            charRightShift = static_cast< CharRightShift* >( plugin );
        else if( strcmp( name, "datamembershadow" ) == 0 )
            dataMemberShadow = static_cast< DataMemberShadow* >( plugin );
        else if( strcmp( name, "dbgunhandledexception" ) == 0 )
            dbgUnhandledException = static_cast< DbgUnhandledException* >( plugin );
        else if( strcmp( name, "derefnullptr" ) == 0 )
            derefNullPtr = static_cast< DerefNullPtr* >( plugin );
        else if( strcmp( name, "doubleconvert" ) == 0 )
            doubleConvert = static_cast< DoubleConvert* >( plugin );
        else if( strcmp( name, "dyncastvisibility" ) == 0 )
            dynCastVisibility = static_cast< DynCastVisibility* >( plugin );
        else if( strcmp( name, "empty" ) == 0 )
            empty = static_cast< Empty* >( plugin );
        else if( strcmp( name, "emptyif" ) == 0 )
            emptyIf = static_cast< EmptyIf* >( plugin );
        else if( strcmp( name, "externandnotdefined" ) == 0 )
            externAndNotDefined = static_cast< ExternAndNotDefined* >( plugin );
        else if( strcmp( name, "externvar" ) == 0 )
            externVar = static_cast< ExternVar* >( plugin );
        else if( strcmp( name, "inlinevisible" ) == 0 )
            inlineVisible = static_cast< InlineVisible* >( plugin );
        else if( strcmp( name, "loopvartoosmall" ) == 0 )
            loopVarTooSmall = static_cast< LoopVarTooSmall* >( plugin );
        else if( strcmp( name, "privatebase" ) == 0 )
            privateBase = static_cast< PrivateBase* >( plugin );
        else if( strcmp( name, "reservedid" ) == 0 )
            reservedId = static_cast< ReservedId* >( plugin );
        else if( strcmp( name, "simplifyconstruct" ) == 0 )
            simplifyConstruct = static_cast< SimplifyConstruct* >( plugin );
        else if( strcmp( name, "stringstatic" ) == 0 )
            stringStatic = static_cast< StringStatic* >( plugin );
        else if( strcmp( name, "subtlezeroinit" ) == 0 )
            subtleZeroInit = static_cast< SubtleZeroInit* >( plugin );
        else if( strcmp( name, "unnecessaryoverride" ) == 0 )
            unnecessaryOverride = static_cast< UnnecessaryOverride* >( plugin );
        else if( strcmp( name, "unnecessaryparen" ) == 0 )
            unnecessaryParen = static_cast< UnnecessaryParen* >( plugin );
        else if( strcmp( name, "unreffun" ) == 0 )
            unrefFun = static_cast< UnrefFun* >( plugin );
        else if( strcmp( name, "unusedvariablecheck" ) == 0 )
            unusedVariableCheck = static_cast< UnusedVariableCheck* >( plugin );
        else if( strcmp( name, "vclwidgets" ) == 0 )
            vclWidgets = static_cast< VCLWidgets* >( plugin );
        else if( strcmp( name, "weakobject" ) == 0 )
            weakObject = static_cast< WeakObject* >( plugin );
        else
            return false;
        return true;
    }
    bool shouldVisitTemplateInstantiations () const
    {
        if( badStatics != nullptr && badStatics->shouldVisitTemplateInstantiations())
            return true;
        if( blockBlock != nullptr && blockBlock->shouldVisitTemplateInstantiations())
            return true;
        if( charRightShift != nullptr && charRightShift->shouldVisitTemplateInstantiations())
            return true;
        if( dataMemberShadow != nullptr && dataMemberShadow->shouldVisitTemplateInstantiations())
            return true;
        if( dbgUnhandledException != nullptr && dbgUnhandledException->shouldVisitTemplateInstantiations())
            return true;
        if( derefNullPtr != nullptr && derefNullPtr->shouldVisitTemplateInstantiations())
            return true;
        if( doubleConvert != nullptr && doubleConvert->shouldVisitTemplateInstantiations())
            return true;
        if( dynCastVisibility != nullptr && dynCastVisibility->shouldVisitTemplateInstantiations())
            return true;
        if( empty != nullptr && empty->shouldVisitTemplateInstantiations())
            return true;
        if( emptyIf != nullptr && emptyIf->shouldVisitTemplateInstantiations())
            return true;
        if( externAndNotDefined != nullptr && externAndNotDefined->shouldVisitTemplateInstantiations())
            return true;
        if( externVar != nullptr && externVar->shouldVisitTemplateInstantiations())
            return true;
        if( inlineVisible != nullptr && inlineVisible->shouldVisitTemplateInstantiations())
            return true;
        if( loopVarTooSmall != nullptr && loopVarTooSmall->shouldVisitTemplateInstantiations())
            return true;
        if( privateBase != nullptr && privateBase->shouldVisitTemplateInstantiations())
            return true;
        if( reservedId != nullptr && reservedId->shouldVisitTemplateInstantiations())
            return true;
        if( simplifyConstruct != nullptr && simplifyConstruct->shouldVisitTemplateInstantiations())
            return true;
        if( stringStatic != nullptr && stringStatic->shouldVisitTemplateInstantiations())
            return true;
        if( subtleZeroInit != nullptr && subtleZeroInit->shouldVisitTemplateInstantiations())
            return true;
        if( unnecessaryOverride != nullptr && unnecessaryOverride->shouldVisitTemplateInstantiations())
            return true;
        if( unnecessaryParen != nullptr && unnecessaryParen->shouldVisitTemplateInstantiations())
            return true;
        if( unrefFun != nullptr && unrefFun->shouldVisitTemplateInstantiations())
            return true;
        if( unusedVariableCheck != nullptr && unusedVariableCheck->shouldVisitTemplateInstantiations())
            return true;
        if( vclWidgets != nullptr && vclWidgets->shouldVisitTemplateInstantiations())
            return true;
        if( weakObject != nullptr && weakObject->shouldVisitTemplateInstantiations())
            return true;
        return false;
    }
    bool VisitVarDecl(VarDecl const*const pVarDecl)
    {
        if( ignoreLocation( pVarDecl ))
            return true;
        if( badStatics != nullptr )
        {
            if( !badStatics->VisitVarDecl( pVarDecl ))
                badStatics = nullptr;
        }
        if( externVar != nullptr )
        {
            if( !externVar->VisitVarDecl( pVarDecl ))
                externVar = nullptr;
        }
        if( stringStatic != nullptr )
        {
            if( !stringStatic->VisitVarDecl( pVarDecl ))
                stringStatic = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitVarDecl( pVarDecl ))
                unnecessaryParen = nullptr;
        }
        if( unusedVariableCheck != nullptr )
        {
            if( !unusedVariableCheck->VisitVarDecl( pVarDecl ))
                unusedVariableCheck = nullptr;
        }
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitVarDecl( pVarDecl ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCompoundStmt(CompoundStmt const * compoundStmt)
    {
        if( ignoreLocation( compoundStmt ))
            return true;
        if( blockBlock != nullptr )
        {
            if( !blockBlock->VisitCompoundStmt( compoundStmt ))
                blockBlock = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinShr(BinaryOperator const * expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( charRightShift != nullptr )
        {
            if( !charRightShift->VisitBinShr( expr ))
                charRightShift = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXMethodDecl(CXXMethodDecl const*const pMethodDecl)
    {
        if( ignoreLocation( pMethodDecl ))
            return true;
        if( unnecessaryOverride != nullptr )
        {
            if( !unnecessaryOverride->VisitCXXMethodDecl( pMethodDecl ))
                unnecessaryOverride = nullptr;
        }
        if( weakObject != nullptr )
        {
            if( !weakObject->VisitCXXMethodDecl( pMethodDecl ))
                weakObject = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitFieldDecl(const FieldDecl *decl)
    {
        if( ignoreLocation( decl ))
            return true;
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitFieldDecl( decl ))
                vclWidgets = nullptr;
        }
        if( dataMemberShadow != nullptr )
        {
            if( !dataMemberShadow->VisitFieldDecl( decl ))
                dataMemberShadow = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitParmVarDecl(const ParmVarDecl *decl)
    {
        if( ignoreLocation( decl ))
            return true;
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitParmVarDecl( decl ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitFunctionDecl(const FunctionDecl *decl)
    {
        if( ignoreLocation( decl ))
            return true;
        if( externAndNotDefined != nullptr )
        {
            if( !externAndNotDefined->VisitFunctionDecl( decl ))
                externAndNotDefined = nullptr;
        }
        if( inlineVisible != nullptr )
        {
            if( !inlineVisible->VisitFunctionDecl( decl ))
                inlineVisible = nullptr;
        }
        if( unrefFun != nullptr )
        {
            if( !unrefFun->VisitFunctionDecl( decl ))
                unrefFun = nullptr;
        }
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitFunctionDecl( decl ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXDestructorDecl(const CXXDestructorDecl *decl)
    {
        if( ignoreLocation( decl ))
            return true;
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitCXXDestructorDecl( decl ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXDeleteExpr(const CXXDeleteExpr *expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitCXXDeleteExpr( expr ))
                unnecessaryParen = nullptr;
        }
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitCXXDeleteExpr( expr ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCallExpr(const CallExpr *expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( dbgUnhandledException != nullptr )
        {
            if( !dbgUnhandledException->VisitCallExpr( expr ))
                dbgUnhandledException = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitCallExpr( expr ))
                unnecessaryParen = nullptr;
        }
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitCallExpr( expr ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitDeclRefExpr(const DeclRefExpr *expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitDeclRefExpr( expr ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXConstructExpr(const CXXConstructExpr *expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitCXXConstructExpr( expr ))
                vclWidgets = nullptr;
        }
        if( simplifyConstruct != nullptr )
        {
            if( !simplifyConstruct->VisitCXXConstructExpr( expr ))
                simplifyConstruct = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinaryOperator(const BinaryOperator *op)
    {
        if( ignoreLocation( op ))
            return true;
        if( vclWidgets != nullptr )
        {
            if( !vclWidgets->VisitBinaryOperator( op ))
                vclWidgets = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr const * expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( dynCastVisibility != nullptr )
        {
            if( !dynCastVisibility->VisitCXXDynamicCastExpr( expr ))
                dynCastVisibility = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitNamedDecl(NamedDecl const * decl)
    {
        if( ignoreLocation( decl ))
            return true;
        if( reservedId != nullptr )
        {
            if( !reservedId->VisitNamedDecl( decl ))
                reservedId = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitReturnStmt(ReturnStmt const* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( stringStatic != nullptr )
        {
            if( !stringStatic->VisitReturnStmt( stmt ))
                stringStatic = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitReturnStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXNewExpr(CXXNewExpr const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( subtleZeroInit != nullptr )
        {
            if( !subtleZeroInit->VisitCXXNewExpr( expr ))
                subtleZeroInit = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitParenExpr(const ParenExpr* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitParenExpr( expr ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitIfStmt(const IfStmt* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( emptyIf != nullptr )
        {
            if( !emptyIf->VisitIfStmt( stmt ))
                emptyIf = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitIfStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitDoStmt(const DoStmt* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( loopVarTooSmall != nullptr )
        {
            if( !loopVarTooSmall->VisitDoStmt( stmt ))
                loopVarTooSmall = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitDoStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitWhileStmt(const WhileStmt* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( loopVarTooSmall != nullptr )
        {
            if( !loopVarTooSmall->VisitWhileStmt( stmt ))
                loopVarTooSmall = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitWhileStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitForStmt(const ForStmt* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( loopVarTooSmall != nullptr )
        {
            if( !loopVarTooSmall->VisitForStmt( stmt ))
                loopVarTooSmall = nullptr;
        }
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitForStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitSwitchStmt(const SwitchStmt* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitSwitchStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCaseStmt(const CaseStmt* stmt)
    {
        if( ignoreLocation( stmt ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitCaseStmt( stmt ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitCXXOperatorCallExpr(const CXXOperatorCallExpr* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitCXXOperatorCallExpr( expr ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitUnaryExprOrTypeTraitExpr( expr ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitConditionalOperator(ConditionalOperator const * expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitConditionalOperator( expr ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinaryConditionalOperator(BinaryConditionalOperator const * expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitBinaryConditionalOperator( expr ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitMemberExpr(const MemberExpr* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( unnecessaryParen != nullptr )
        {
            if( !unnecessaryParen->VisitMemberExpr( expr ))
                unnecessaryParen = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitUnaryDeref(UnaryOperator const * op)
    {
        if( ignoreLocation( op ))
            return true;
        if( derefNullPtr != nullptr )
        {
            if( !derefNullPtr->VisitUnaryDeref( op ))
                derefNullPtr = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitMaterializeTemporaryExpr(MaterializeTemporaryExpr const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( doubleConvert != nullptr )
        {
            if( !doubleConvert->VisitMaterializeTemporaryExpr( expr ))
                doubleConvert = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinLT(BinaryOperator const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( empty != nullptr )
        {
            if( !empty->VisitBinLT( expr ))
                empty = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinGT(BinaryOperator const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( empty != nullptr )
        {
            if( !empty->VisitBinGT( expr ))
                empty = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinLE(BinaryOperator const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( empty != nullptr )
        {
            if( !empty->VisitBinLE( expr ))
                empty = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinGE(BinaryOperator const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( empty != nullptr )
        {
            if( !empty->VisitBinGE( expr ))
                empty = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinEQ(BinaryOperator const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( empty != nullptr )
        {
            if( !empty->VisitBinEQ( expr ))
                empty = nullptr;
        }
        return anyPluginActive();
    }
    bool VisitBinNE(BinaryOperator const* expr)
    {
        if( ignoreLocation( expr ))
            return true;
        if( empty != nullptr )
        {
            if( !empty->VisitBinNE( expr ))
                empty = nullptr;
        }
        return anyPluginActive();
    }

    bool TraverseCXXCatchStmt(CXXCatchStmt* catchStmt)
    {
        DbgUnhandledException* saveDbgUnhandledException = dbgUnhandledException;
        if( dbgUnhandledException != nullptr )
        {
            if( !dbgUnhandledException->PreTraverseCXXCatchStmt( catchStmt ))
                dbgUnhandledException = nullptr;
        }
        bool ret = RecursiveASTVisitor::TraverseCXXCatchStmt(catchStmt);
        if( dbgUnhandledException != nullptr )
        {
            if( !dbgUnhandledException->PostTraverseCXXCatchStmt( catchStmt, ret ))
                saveDbgUnhandledException = nullptr;
        }
        dbgUnhandledException = saveDbgUnhandledException;
        return ret;
    }
    bool TraverseFriendDecl(FriendDecl * decl)
    {
        UnrefFun* saveUnrefFun = unrefFun;
        if( unrefFun != nullptr )
        {
            if( !unrefFun->PreTraverseFriendDecl( decl ))
                unrefFun = nullptr;
        }
        bool ret = RecursiveASTVisitor::TraverseFriendDecl(decl);
        if( unrefFun != nullptr )
        {
            if( !unrefFun->PostTraverseFriendDecl( decl, ret ))
                saveUnrefFun = nullptr;
        }
        unrefFun = saveUnrefFun;
        return ret;
    }
    bool TraverseReturnStmt(ReturnStmt* stmt)
    {
        SimplifyConstruct* saveSimplifyConstruct = simplifyConstruct;
        if( simplifyConstruct != nullptr )
        {
            if( !simplifyConstruct->PreTraverseReturnStmt( stmt ))
                simplifyConstruct = nullptr;
        }
        bool ret = RecursiveASTVisitor::TraverseReturnStmt( stmt );
        if( simplifyConstruct != nullptr )
        {
//            if( !simplifyConstruct->PostTraverseReturnStmt( stmt, ret ))
//                saveSimplifyConstruct = nullptr;
        }
        simplifyConstruct = saveSimplifyConstruct;
        return ret;
    }
    bool TraverseInitListExpr(InitListExpr* expr)
    {
        SimplifyConstruct* saveSimplifyConstruct = simplifyConstruct;
        if( simplifyConstruct != nullptr )
        {
            if( !simplifyConstruct->PreTraverseInitListExpr( expr ))
                simplifyConstruct = nullptr;
        }
        bool ret = RecursiveASTVisitor::TraverseInitListExpr( expr );
        if( simplifyConstruct != nullptr )
        {
//            if( !simplifyConstruct->PostTraverseInitListExpr( expr, ret ))
//                saveSimplifyConstruct = nullptr;
        }
        simplifyConstruct = saveSimplifyConstruct;
        return ret;
    }
    bool TraverseCXXBindTemporaryExpr(CXXBindTemporaryExpr* expr)
    {
        SimplifyConstruct* saveSimplifyConstruct = simplifyConstruct;
        if( simplifyConstruct != nullptr )
        {
            if( !simplifyConstruct->PreTraverseCXXBindTemporaryExpr( expr ))
                simplifyConstruct = nullptr;
        }
        bool ret = RecursiveASTVisitor::TraverseCXXBindTemporaryExpr( expr );
        if( simplifyConstruct != nullptr )
        {
//            if( !simplifyConstruct->PostTraverseCXXBindTemporaryExpr( expr, ret ))
//                saveSimplifyConstruct = nullptr;
        }
        simplifyConstruct = saveSimplifyConstruct;
        return ret;
    }

private:
    bool anyPluginActive() const
    {
        return badStatics != nullptr
            || blockBlock != nullptr
            || charRightShift != nullptr
            || dataMemberShadow != nullptr
            || dbgUnhandledException != nullptr
            || derefNullPtr != nullptr
            || doubleConvert != nullptr
            || dynCastVisibility != nullptr
            || empty != nullptr
            || emptyIf != nullptr
            || externAndNotDefined != nullptr
            || externVar != nullptr
            || inlineVisible != nullptr
            || loopVarTooSmall != nullptr
            || privateBase != nullptr
            || reservedId != nullptr
            || simplifyConstruct != nullptr
            || stringStatic != nullptr
            || subtleZeroInit != nullptr
            || unnecessaryOverride != nullptr
            || unnecessaryParen != nullptr
            || unrefFun != nullptr
            || unusedVariableCheck != nullptr
            || vclWidgets != nullptr
            || weakObject != nullptr;
    }
    // TODO mutable
    mutable BadStatics* badStatics;
    mutable BlockBlock* blockBlock;
    mutable CharRightShift* charRightShift;
    mutable DataMemberShadow* dataMemberShadow;
    mutable DbgUnhandledException* dbgUnhandledException;
    mutable DerefNullPtr* derefNullPtr;
    mutable DoubleConvert* doubleConvert;
    mutable DynCastVisibility* dynCastVisibility;
    mutable Empty* empty;
    mutable EmptyIf* emptyIf;
    mutable ExternAndNotDefined* externAndNotDefined;
    mutable ExternVar* externVar;
    mutable InlineVisible* inlineVisible;
    mutable LoopVarTooSmall* loopVarTooSmall;
    mutable PrivateBase* privateBase;
    mutable ReservedId* reservedId;
    mutable SimplifyConstruct* simplifyConstruct;
    mutable StringStatic* stringStatic;
    mutable SubtleZeroInit* subtleZeroInit;
    mutable UnnecessaryOverride* unnecessaryOverride;
    mutable UnnecessaryParen* unnecessaryParen;
    mutable UnrefFun* unrefFun;
    mutable UnusedVariableCheck* unusedVariableCheck;
    mutable VCLWidgets* vclWidgets;
    mutable WeakObject* weakObject;
};

loplugin::Plugin::Registration< SharedRecursiveASTVisitor > X("sharedvisitor");

} // namespace

#endif // FOO

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
