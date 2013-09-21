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

#ifndef REFERENCECASTING_H
#define REFERENCECASTING_H

#include "plugin.hxx"

namespace loplugin
{

class ReferenceCasting
    : public RecursiveASTVisitor< ReferenceCasting >
    , public Plugin
    {
    public:
        explicit ReferenceCasting( CompilerInstance& compiler );
        virtual void run() override;
        bool VisitCXXConstructExpr( CXXConstructExpr* cce );
    };

} // namespace

#endif // REFERENCECASTING_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
