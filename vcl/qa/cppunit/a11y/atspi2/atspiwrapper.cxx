/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "atspiwrapper.hxx"

using namespace Atspi;

Accessible Relation::getTarget(int i) const { return invoke(atspi_relation_get_target, i); }

Component Accessible::queryComponent() const
{
    return queryInterface<Component>(atspi_accessible_get_component_iface);
}
Text Accessible::queryText() const { return queryInterface<Text>(atspi_accessible_get_text_iface); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
