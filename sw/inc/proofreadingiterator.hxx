/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SW_INC_PROOFREADINGITERATOR_HXX
#define INCLUDED_SW_INC_PROOFREADINGITERATOR_HXX

#include <sal/config.h>

#include <com/sun/star/uno/Reference.hxx>
#include <sal/types.h>

namespace com { namespace sun { namespace star {
    namespace linguistic2 { class XProofreadingIterator; }
    namespace uno { class XComponentContext; }
} } }

// A simple wrapper around the css.linguistic2.ProofreadingIterator
// single-instance service.  The first time that service implementation gets
// instantiated it immediately starts a GrammarCheckingIterator thread that
// eventually needs to be joined (via dispose):

namespace sw { namespace proofreadingiterator {

css::uno::Reference<css::linguistic2::XProofreadingIterator> get(
    css::uno::Reference<css::uno::XComponentContext> const & context);

void dispose();

} }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
