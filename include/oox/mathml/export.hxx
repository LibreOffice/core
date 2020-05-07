/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_OOX_MATHML_EXPORT_HXX
#define INCLUDED_OOX_MATHML_EXPORT_HXX

#include <oox/core/filterbase.hxx>
#include <oox/dllapi.h>
#include <oox/export/utils.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/textenc.h>
#include <sax/fshelper.hxx>

namespace oox
{

/**
 Interface class, StarMath will implement writeFormula*() to write out markup
 representing the formula.
 */
class OOX_DLLPUBLIC FormulaExportBase
{
public:
    virtual void writeFormulaOoxml(::sax_fastparser::FSHelperPtr pSerializer,
            oox::core::OoxmlVersion version,
            oox::drawingml::DocumentType documentType, sal_Int8 nAlign) = 0;
    virtual void writeFormulaRtf( OStringBuffer& rBuffer, rtl_TextEncoding nEncoding ) = 0;
    enum eFormulaAlign { INLINE, CENTER, GROUPEDCENTER, LEFT, RIGHT };

protected:
    FormulaExportBase();

    ~FormulaExportBase() {}
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
