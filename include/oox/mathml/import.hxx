/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_OOX_MATHML_IMPORT_HXX
#define INCLUDED_OOX_MATHML_IMPORT_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <tools/gen.hxx>

#include <oox/dllapi.h>

namespace oox
{

namespace formulaimport
{
class XmlStream;
}

/**
 Interface class, StarMath will implement readFormulaOoxml() to read OOXML
 representing the formula and getFormulaSize() to provide the size of the resulting
 formula.
 */
class OOX_DLLPUBLIC FormulaImportBase
{
public:
    virtual void readFormulaOoxml( oox::formulaimport::XmlStream& stream ) = 0;
    virtual Size getFormulaSize() const = 0;

protected:
    FormulaImportBase();

    ~FormulaImportBase() {}
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
