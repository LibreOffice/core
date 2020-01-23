/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_clang.h"

// no warning expected
namespace xxx::yyy
{
}
namespace xxx::yyy {
}
// expected-error@+1 {{statement left brace mis-aligned [loplugin:namespaceindentation]}}
namespace xxx::yyy
  {
// expected-error@+1 {{statement right brace mis-aligned [loplugin:namespaceindentation]}}
  }
namespace xxx::yyy
{
 // expected-error@+1 {{statement right brace mis-aligned [loplugin:namespaceindentation]}}
  }
// expected-error@+1 {{statement left brace mis-aligned [loplugin:namespaceindentation]}}
namespace xxx::yyy
  {
}
namespace xxx::yyy {
// expected-error@+1 {{statement right brace mis-aligned [loplugin:namespaceindentation]}}
  }
namespace xxx::yyy
{
} // fooo baaaar
// expected-error@-1 {{incorrect comment at end of namespace xxx::yyy [loplugin:namespaceindentation]}}
namespace aaa::bbb
{
} // namespace foo
// expected-error@-1 {{incorrect comment at end of namespace aaa::bbb [loplugin:namespaceindentation]}}
namespace xxx::yyy
{
} // xxx::yyy
// no warning expected
namespace com { namespace sun { namespace star { } } }
// no warning expected
namespace com::sun::star::uno { class XComponentContext; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
