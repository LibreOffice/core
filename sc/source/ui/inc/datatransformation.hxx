/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_UI_DATAPROVIDER_DATATRANSFORMATION_HXX
#define INCLUDED_SC_SOURCE_UI_DATAPROVIDER_DATATRANSFORMATION_HXX

#include <address.hxx>
#include <scdllapi.h>

class ScDocument;

namespace sc {

class SC_DLLPUBLIC DataTransformation
{
public:
    virtual ~DataTransformation();

    virtual void Transform(ScDocument& rDoc) = 0;
};

class SC_DLLPUBLIC ColumnRemoveTransformation : public DataTransformation
{
    SCCOL mnCol;

public:

    ColumnRemoveTransformation(SCCOL nCol);
    virtual ~ColumnRemoveTransformation();

    virtual void Transform(ScDocument& rDoc) override;
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
