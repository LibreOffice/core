/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERPERFECT_QA_UNIT_WPFTIMPORT_HXX
#define INCLUDED_WRITERPERFECT_QA_UNIT_WPFTIMPORT_HXX

#include <com/sun/star/uno/Reference.hxx>

namespace com
{
namespace sun
{
namespace star
{
namespace io
{
class XInputStream;
}
}
}
}

css::uno::Reference<css::io::XInputStream> createDummyInput();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
