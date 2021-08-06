/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <mathml/export.hxx>

#include <com/sun/star/util/MeasureUnit.hpp>
#include <com/sun/star/xml/sax/Writer.hpp>

using namespace ::com::sun::star;
using namespace xmloff::token;

void SmMlExport::declareMlError() { SAL_WARN("starmath", "Invalid use of mathml."); }

void SmMlExport::exportMlAttributtes(const SmMlElement* pMlElement)
{
    size_t nAttributeCount = pMlElement->getAttributeCount();
    for (size_t i = 0; i < nAttributeCount; ++i)
    {
        SmMlAttribute aAttribute = pMlElement->getAttribute(i);
        switch (aAttribute.getMlAttributeValueType())
        {
            case SmMlAttributeValueType::MlAccent:
            {
                auto aAttributeValue = aAttribute.getMlAccent();
                switch (aAttributeValue->m_aAccent)
                {
                    case SmMlAttributeValueAccent::MlFalse:
                        addAttribute(XML_ACCENT, XML_FALSE);
                        break;
                    case SmMlAttributeValueAccent::MlTrue:
                        addAttribute(XML_ACCENT, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlDir:
            {
                auto aAttributeValue = aAttribute.getMlDir();
                switch (aAttributeValue->m_aDir)
                {
                    case SmMlAttributeValueDir::MlLtr:
                        addAttribute(XML_DIR, XML_LTR);
                        break;
                    case SmMlAttributeValueDir::MlRtl:
                        addAttribute(XML_DIR, XML_RTL);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlDisplaystyle:
            {
                auto aAttributeValue = aAttribute.getMlDisplaystyle();
                switch (aAttributeValue->m_aDisplaystyle)
                {
                    case SmMlAttributeValueDisplaystyle::MlTrue:
                        addAttribute(XML_DISPLAYSTYLE, XML_FALSE);
                        break;
                    case SmMlAttributeValueDisplaystyle::MlFalse:
                        addAttribute(XML_DISPLAYSTYLE, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlFence:
            {
                auto aAttributeValue = aAttribute.getMlFence();
                switch (aAttributeValue->m_aFence)
                {
                    case SmMlAttributeValueFence::MlTrue:
                        addAttribute(XML_FENCE, XML_FALSE);
                        break;
                    case SmMlAttributeValueFence::MlFalse:
                        addAttribute(XML_FENCE, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlHref:
            {
                auto aAttributeValue = aAttribute.getMlHref();
                switch (aAttributeValue->m_aHref)
                {
                    case SmMlAttributeValueHref::NMlEmpty:
                        break;
                    case SmMlAttributeValueHref::NMlValid:
                        addAttribute(XML_FENCE, *aAttributeValue->m_aLnk);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlLspace:
            case SmMlAttributeValueType::MlMathbackground:
            {
                auto aAttributeValue = aAttribute.getMlMathbackground();
                switch (aAttributeValue->m_aMathbackground)
                {
                    case SmMlAttributeValueMathbackground::MlTransparent:
                        addAttribute(XML_MATHBACKGROUND, "transparent");
                        break;
                    case SmMlAttributeValueMathbackground::MlRgb:
                    {
                        OUString aTextColor = OUString::createFromAscii(
                            starmathdatabase::Identify_Color_MATHML(
                                static_cast<uint32_t>(aAttributeValue->m_aCol))
                                .pIdent);
                        addAttribute(XML_MATHBACKGROUND, aTextColor);
                        break;
                    }
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlMathcolor:
            {
                auto aAttributeValue = aAttribute.getMlMathcolor();
                switch (aAttributeValue->m_aMathcolor)
                {
                    case SmMlAttributeValueMathcolor::MlDefault:
                        break;
                    case SmMlAttributeValueMathcolor::MlRgb:
                    {
                        OUString aTextColor = OUString::createFromAscii(
                            starmathdatabase::Identify_Color_MATHML(
                                static_cast<uint32_t>(aAttributeValue->m_aCol))
                                .pIdent);
                        addAttribute(XML_MATHCOLOR, aTextColor);
                        break;
                    }
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlMathsize:
            case SmMlAttributeValueType::MlMathvariant:
            {
                auto aAttributeValue = aAttribute.getMlMathvariant();
                switch (aAttributeValue->m_aMathvariant)
                {
                    case SmMlAttributeValueMathvariant::normal:
                        addAttribute(XML_MATHVARIANT, "normal");
                        break;
                    case SmMlAttributeValueMathvariant::bold:
                        addAttribute(XML_MATHVARIANT, "bold");
                        break;
                    case SmMlAttributeValueMathvariant::italic:
                        addAttribute(XML_MATHVARIANT, "italic");
                        break;
                    case SmMlAttributeValueMathvariant::double_struck:
                        addAttribute(XML_MATHVARIANT, "double-struck");
                        break;
                    case SmMlAttributeValueMathvariant::script:
                        addAttribute(XML_MATHVARIANT, "script");
                        break;
                    case SmMlAttributeValueMathvariant::fraktur:
                        addAttribute(XML_MATHVARIANT, "fraktur");
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif:
                        addAttribute(XML_MATHVARIANT, "sans-serif");
                        break;
                    case SmMlAttributeValueMathvariant::monospace:
                        addAttribute(XML_MATHVARIANT, "monospace");
                        break;
                    case SmMlAttributeValueMathvariant::bold_italic:
                        addAttribute(XML_MATHVARIANT, "bold-italic");
                        break;
                    case SmMlAttributeValueMathvariant::bold_fraktur:
                        addAttribute(XML_MATHVARIANT, "bold-fracktur");
                        break;
                    case SmMlAttributeValueMathvariant::bold_script:
                        addAttribute(XML_MATHVARIANT, "bold-script");
                        break;
                    case SmMlAttributeValueMathvariant::bold_sans_serif:
                        addAttribute(XML_MATHVARIANT, "bold-sans-serif");
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif_italic:
                        addAttribute(XML_MATHVARIANT, "sans-serif-italic");
                        break;
                    case SmMlAttributeValueMathvariant::sans_serif_bold_italic:
                        addAttribute(XML_MATHVARIANT, "sans-serif-bold-italic");
                        break;
                    case SmMlAttributeValueMathvariant::initial:
                        addAttribute(XML_MATHVARIANT, "initial");
                        break;
                    case SmMlAttributeValueMathvariant::tailed:
                        addAttribute(XML_MATHVARIANT, "tailed");
                        break;
                    case SmMlAttributeValueMathvariant::looped:
                        addAttribute(XML_MATHVARIANT, "looped");
                        break;
                    case SmMlAttributeValueMathvariant::stretched:
                        addAttribute(XML_MATHVARIANT, "stretched");
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }

            case SmMlAttributeValueType::MlMaxsize:
            case SmMlAttributeValueType::MlMinsize:
            case SmMlAttributeValueType::MlMovablelimits:
            {
                auto aAttributeValue = aAttribute.getMlMovablelimits();
                switch (aAttributeValue->m_aMovablelimits)
                {
                    case SmMlAttributeValueMovablelimits::MlFalse:
                        addAttribute(XML_MOVABLELIMITS, XML_FALSE);
                        break;
                    case SmMlAttributeValueMovablelimits::MlTrue:
                        addAttribute(XML_MOVABLELIMITS, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlRspace:
            case SmMlAttributeValueType::MlSeparator:
            {
                auto aAttributeValue = aAttribute.getMlSeparator();
                switch (aAttributeValue->m_aSeparator)
                {
                    case SmMlAttributeValueSeparator::MlFalse:
                        addAttribute(XML_SEPARATOR, XML_FALSE);
                        break;
                    case SmMlAttributeValueSeparator::MlTrue:
                        addAttribute(XML_SEPARATOR, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }

            case SmMlAttributeValueType::MlStretchy:
            {
                auto aAttributeValue = aAttribute.getMlStretchy();
                switch (aAttributeValue->m_aStretchy)
                {
                    case SmMlAttributeValueStretchy::MlFalse:
                        addAttribute(XML_STRETCHY, XML_FALSE);
                        break;
                    case SmMlAttributeValueStretchy::MlTrue:
                        addAttribute(XML_STRETCHY, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case SmMlAttributeValueType::MlSymmetric:
            {
                auto aAttributeValue = aAttribute.getMlSymmetric();
                switch (aAttributeValue->m_aSymmetric)
                {
                    case SmMlAttributeValueSymmetric::MlFalse:
                        addAttribute(XML_SYMMETRIC, XML_FALSE);
                        break;
                    case SmMlAttributeValueSymmetric::MlTrue:
                        addAttribute(XML_SYMMETRIC, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            default:
                declareMlError();
                break;
        }
    }
}

void SmMlExport::exportMlElement(const SmMlElement* pMlElement)
{
    SvXMLElementExport* pElementExport;
    switch (pMlElement->getMlElementType())
    {
        case SmMlElementType::MlMath:
            pElementExport = createElementExport(XML_MATH);
            break;
        case SmMlElementType::MlMi:
            pElementExport = createElementExport(XML_MI);
            break;
        case SmMlElementType::MlMerror:
            pElementExport = createElementExport(XML_MERROR);
            break;
        case SmMlElementType::MlMn:
            pElementExport = createElementExport(XML_MN);
            break;
        case SmMlElementType::MlMo:
            pElementExport = createElementExport(XML_MO);
            break;
        case SmMlElementType::MlMrow:
            pElementExport = createElementExport(XML_MROW);
            break;
        case SmMlElementType::MlMtext:
            pElementExport = createElementExport(XML_MTEXT);
            break;
        case SmMlElementType::MlMstyle:
            pElementExport = createElementExport(XML_MSTYLE);
            break;
        default:
            pElementExport = nullptr;
    }
    exportMlAttributtes(pMlElement);
    const OUString& aElementText = pMlElement->getText();
    if (aElementText.isEmpty())
        GetDocHandler()->characters(aElementText);
    (void)pElementExport;
}

SmMlExport::SmMlExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                       OUString const& implementationName, SvXMLExportFlags nExportFlags)
    : SvXMLExport(rContext, implementationName, util::MeasureUnit::INCH, XML_MATH, nExportFlags)
    , m_pTree(nullptr)
    , m_bSuccess(false)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
