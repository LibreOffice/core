/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "alterparser.hxx"

namespace dbahsql
{
class FbAlterStmtParser : public AlterStmtParser
{
protected:
    void ensureProperTableLengths() const;

public:
    /**
     * Compose the result of the parser to statements of Firebird dialect
     */
    virtual OUString compose() const override;
};

} // dbahsql

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
