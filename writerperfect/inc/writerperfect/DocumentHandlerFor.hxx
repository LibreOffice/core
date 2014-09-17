/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_DOCUMENTHANDLERFOR_HXX
#define INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_DOCUMENTHANDLERFOR_HXX

namespace writerperfect
{

/** Definition of XML import service used with a Generator.

    This template must be specialized for every libodfgen generator.
  */
template<class Generator>
struct DocumentHandlerFor
{
    // static const rtl::OUString name();
};

}

#endif // INCLUDED_WRITERPERFECT_INC_WRITERPERFECT_DOCUMENTHANDLERFOR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
