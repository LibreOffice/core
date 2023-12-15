/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_OOX_MATHML_IMEXPORT_HXX
#define INCLUDED_OOX_MATHML_IMEXPORT_HXX

#include <oox/core/filterbase.hxx>
#include <oox/dllapi.h>
#include <oox/export/utils.hxx>
#include <rtl/ref.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/textenc.h>
#include <sax/fshelper.hxx>
#include <tools/gen.hxx>

namespace oox
{

namespace formulaimport
{
class XmlStream;
}

/**
 Interface class, StarMath will implement readFormulaOoxml() to read OOXML
 representing the formula, getFormulaSize() to provide the size of the resulting
 formula, and writeFormula*() to write out markup representing the formula.
 */
class OOX_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") FormulaImExportBase
{
public:
    virtual void readFormulaOoxml( oox::formulaimport::XmlStream& stream ) = 0;
    virtual Size getFormulaSize() const = 0; // Unit is mm100

    virtual void writeFormulaOoxml(::sax_fastparser::FSHelperPtr pSerializer,
            oox::core::OoxmlVersion version,
            oox::drawingml::DocumentType documentType, sal_Int8 nAlign) = 0;
    virtual void writeFormulaRtf( OStringBuffer& rBuffer, rtl_TextEncoding nEncoding ) = 0;
    enum eFormulaAlign { INLINE, CENTER, LEFT, RIGHT };

protected:
    FormulaImExportBase();

    ~FormulaImExportBase() {}
};

namespace core { class ContextHandler; }
namespace drawingml { class TextParagraph; }

::rtl::Reference<core::ContextHandler> CreateLazyMathBufferingContext(
        core::ContextHandler const& rParent, drawingml::TextParagraph & rPara);

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
