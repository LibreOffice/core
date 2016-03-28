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

#ifndef STINNSPACE_H
#define STINNSPACE_H

#include "plugin.hxx"
namespace loplugin
{

class StInNspace
    : public RecursiveASTVisitor< StInNspace >
    , public Plugin
    {
    public:
        StInNspace( const InstantiationData& data );
        virtual void run() override;
        bool VisitFunctionDecl( FunctionDecl* func );

    };

} // namespace

#endif // POSTFIXINCREMENTFIX_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
