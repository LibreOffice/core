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

void SmMlExport::exportMlAttributtes(const SmMlElement* pMlElement);
{
    size_t nAttributeCount = pMlElement->getAttributeCount();
    for (size_t i = 0; i < nAttributeCount; ++i)
    {
        SmMlAttributeValue aAttribute = pMlElement->getAttribute(i);
        switch (aAttribute.getMlAttributeValueType())
        {
            case MlAccent:
            {
                auto aAttributeValue = aAttribute.getAccent();
                switch (aAttributeValue->m_aAccent)
                {
                    case SmMlAttributeValueAccent::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_ACCENT, XML_FALSE);
                        break;
                    case SmMlAttributeValueAccent::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_ACCENT, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlDir:
            {
                auto aAttributeValue = aAttribute.getDir();
                switch (aAttributeValue->m_aDir)
                {
                    case SmMlAttributeValueAccent::MlLtr:
                        AddAttribute(XML_NAMESPACE_MATH, XML_DIR, XML_LTR);
                        break;
                    case SmMlAttributeValueAccent::MlRtl:
                        AddAttribute(XML_NAMESPACE_MATH, XML_DIR, XML_RTL);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlDisplaystyle:
            {
                auto aAttributeValue = aAttribute.getDisplaystyle();
                switch (aAttributeValue->m_aDisplaystyle)
                {
                    case SmMlAttributeValueDisplaystyle::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_DISPLAY_STYLE, XML_FALSE);
                        break;
                    case SmMlAttributeValueDisplaystyle::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_DISPLAY_STYLE, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlFence:
            {
                auto aAttributeValue = aAttribute.getFence();
                switch (aAttributeValue->m_aFence)
                {
                    case SmMlAttributeValueFence::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_FENCE, XML_FALSE);
                        break;
                    case SmMlAttributeValueFence::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_FENCE, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlHref:
            {
                auto aAttributeValue = aAttribute.getHref();
                switch (aAttributeValue->m_aHref)
                {
                    case SmMlAttributeValueFence::NMlEmpty:
                        break;
                    case SmMlAttributeValueFence::NMlValid:
                        AddAttribute(XML_NAMESPACE_MATH, XML_FENCE, aAttributeValue->m_aLnk);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlLspace:
            case MlMathbackground:
            {
                auto aAttributeValue = aAttribute.getMathbackground();
                switch (aAttributeValue->m_aMathbackground)
                {
                    case SmMlAttributeValueMathBackground::MlTransparent:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_BACKGROUND, "transparent");
                        break;
                    case SmMlAttributeValueMathBackground::MlRgb:
                        OUString sssStr = OUString::createFromAscii(
                            starmathdatabase::Identify_Color_MATHML(aAttributeValue->m_aCol)
                                .pIdent);
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_BACKGROUND,
                                     aAttributeValue->m_aLnk);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlMathcolor:
            {
                auto aAttributeValue = aAttribute.getMathColor();
                switch (aAttributeValue->m_aMathbackground)
                {
                    case SmMlAttributeValueMathColor::MlDefault:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_BACKGROUND, "black");
                        break;
                    case SmMlAttributeValueMathColor::MlRgb:
                        OUString sssStr = OUString::createFromAscii(
                            starmathdatabase::Identify_Color_MATHML(aAttributeValue->m_aCol)
                                .pIdent);
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_BACKGROUND,
                                     aAttributeValue->m_aLnk);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlMathsize:
            case MlMathvariant:
            {
                auto aAttributeValue = aAttribute.getMathVariant();
                switch (aAttributeValue->m_aMathVariant)
                {
                    case normal:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "normal");
                        break;
                    case bold:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "bold");
                        break;
                    case italic:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "italic");
                        break;
                    case double_struck:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "double-struck");
                        break;
                    case script:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "script");
                        break;
                    case fraktur:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "fraktur");
                        break;
                    case sans_serif:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "sans-serif");
                        break;
                    case monospace:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "monospace");
                        break;
                    case bold_italic:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "bold-italic");
                        break;
                    case bold_fraktur:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "bold-fracktur");
                        break;
                    case bold_script:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "bold-script");
                        break;
                    case bold_sans_serif:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "bold-sans-serif");
                        break;
                    case sans_serif_italic:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "sans-serif-italic");
                        break;
                    case sans_serif_bold_italic:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT,
                                     "sans-serif-bold-italic");
                        break;
                    case initial:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "initial");
                        break;
                    case tailed:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "tailed");
                        break;
                    case looped:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "looped");
                        break;
                    case stretched:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MATH_VARIANT, "stretched");
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }

            case MlMaxsize:
            case MlMinsize:
            case MlMovablelimits:
            {
                auto aAttributeValue = aAttribute.getMovablelimits();
                switch (aAttributeValue->m_aMovableLimits)
                {
                    case SmMlAttributeValueMovablelimits::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MOVABLE_LIMITS, XML_FALSE);
                        break;
                    case SmMlAttributeValueMovablelimits::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_MOVABLE_LIMITS, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlRspace:
            case MlSeparator:
            {
                auto aAttributeValue = aAttribute.getMlSeparator();
                switch (aAttributeValue->m_aSeparator)
                {
                    case SmMlAttributeValueSeparator::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_SEPARATOR, XML_FALSE);
                        break;
                    case SmMlAttributeValueSeparator::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_SEPARATOR, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }

            case MlStretchy:
            {
                auto aAttributeValue = aAttribute.getMlStretchy();
                switch (aAttributeValue->m_aStretchy)
                {
                    case SmMlAttributeValueStretchy::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_FALSE);
                        break;
                    case SmMlAttributeValueStretchy::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_STRETCHY, XML_TRUE);
                        break;
                    default:
                        declareMlError();
                        break;
                }
                break;
            }
            case MlSymmetric:
            {
                auto aAttributeValue = aAttribute.getMlSymmetric();
                switch (aAttributeValue->m_aSymmetric)
                {
                    case SmMlAttributeValueSymmetric::MlFalse:
                        AddAttribute(XML_NAMESPACE_MATH, XML_SYMMETRIC, XML_FALSE);
                        break;
                    case SmMlAttributeValueSymmetric::MlTrue:
                        AddAttribute(XML_NAMESPACE_MATH, XML_SYMMETRIC, XML_TRUE);
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

SvXMLElementExport* SmMlExport::exportMlElement(const SmMlElement* pMlElement)
{
    SvXMLElementExport* pElementExport;
    switch (pMlElement->getMlElementType)
    {
        case MlMath:
            pElementExport = createElementExport(XML_MATH);
            break;
        case MlMi:
            pElementExport = createElementExport(XML_MI);
            break;
        case MlMerror:
            pElementExport = createElementExport(XML_MERROR);
            break;
        case MlMn:
            pElementExport = createElementExport(XML_MN);
            break;
        case MlMo:
            pElementExport = createElementExport(XML_MO);
            break;
        case MlMrow:
            pElementExport = createElementExport(XML_MROW);
            break;
        case MlMtext:
            pElementExport = createElementExport(XML_MTEXT);
            break;
        case MlMstyle:
            pElementExport = createElementExport(XML_MSTYLE);
            break;
    }
    exportMlAttributtes(pMlElement);
}

SvXMLElementExport pSvXMLElementExport* = new SvXMLElementExport(*this, XML_NAMESPACE_MATH,
                                                                 XML_MATH, true, false);

SmXMLExport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
            OUString const& implementationName, SvXMLExportFlags nExportFlags);
    : SvXMLExport(rContext, implementationName, util::MeasureUnit::INCH, XML_MATH, nExportFlags)
    , pTree(nullptr)
    , bSuccess(false)
    {
    }

    /* vim:set shiftwidth=4 softtabstop=4 expandtab: */
