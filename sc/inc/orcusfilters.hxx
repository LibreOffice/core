/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef __SC_ORCUSFILTERS_HXX__
#define __SC_ORCUSFILTERS_HXX__

#include "rtl/ustring.hxx"

class ScDocument;

/**
 * Collection of orcus filter wrappers.
 */
class ScOrcusFilters
{
public:
    virtual bool importCSV(ScDocument& rDoc, const rtl::OUString& rPath) const = 0;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
