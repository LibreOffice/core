/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/linguistic2/ProofreadingIterator.hpp>
#include <com/sun/star/linguistic2/XProofreadingIterator.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <vcl/svapp.hxx>

#include <proofreadingiterator.hxx>

namespace
{
css::uno::Reference<css::linguistic2::XProofreadingIterator> instance;
bool disposed = false;

void doDispose(css::uno::Reference<css::linguistic2::XProofreadingIterator> const& inst)
{
    css::uno::Reference<css::lang::XComponent> comp(inst, css::uno::UNO_QUERY);
    if (comp.is())
    {
        SolarMutexReleaser r;
        comp->dispose();
    }
}
}

css::uno::Reference<css::linguistic2::XProofreadingIterator>
sw::proofreadingiterator::get(css::uno::Reference<css::uno::XComponentContext> const& context)
{
    css::uno::Reference<css::linguistic2::XProofreadingIterator> inst(
        css::linguistic2::ProofreadingIterator::create(context));
    bool disp;
    {
        SolarMutexGuard g;
        instance = inst;
        disp = disposed;
    }
    if (disp)
    {
        doDispose(inst);
    }
    return inst;
}

void sw::proofreadingiterator::dispose()
{
    css::uno::Reference<css::linguistic2::XProofreadingIterator> inst;
    {
        SolarMutexGuard g;
        inst = instance;
        instance.clear();
        disposed = true;
    }
    doDispose(inst);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
