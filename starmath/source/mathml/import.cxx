/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

// Our mathml
#include <mathml/import.hxx>

// LO tools to use

// Extra LO tools
#include <rtl/character.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

// Our starmath tools
#include <smmod.hxx>
#include <cfgitem.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::xml::sax;
using namespace xmloff::token;

// SmMlImportContext
/*************************************************************************************************/

namespace
{
class SmMlImportContext : public SvXMLImportContext
{
private:
    SmMlElement* m_pElement;
    SmMlElement* m_pParent;
    SmMlElement* m_pStyle;

public:
    SmMlImportContext(SmMlImport& rImport, SmMlElement* pParent)
        : SvXMLImportContext(rImport)
        , m_pElement(nullptr)
        , m_pParent(pParent)
        , m_pStyle(nullptr)
    {
    }

private:
    void declareMlError();

public:
    /** Handles characters (text)
     */
    virtual void SAL_CALL characters(const OUString& aChars) override;

    /** Starts the mathml element
     */
    virtual void SAL_CALL startFastElement(sal_Int32 nElement,
                                           Reference<XFastAttributeList>& aAttributeList);

    /** Inherits the style from it's parents
    */
    void inheritStyle();

    /** Handle mathml attributes
    */
    void handleAttributes(Reference<XFastAttributeList>& aAttributeList);

    /** Handle mathml length attributes
    */
    SmLengthValue SmMlImportContext::handleLengthAttributte(const OUString& aAttribute);
};

void SmMlImportContext::declareMlError()
{
    SmMlImport& aSmMlImport = static_cast<SmMlImport&>(GetImport());
    aSmMlImport.declareMlError();
}

void SmMlImportContext::inheritStyle()
{
    SmMlAttribute aAttribute;
    while ((m_pStyle = m_pStyle->getParent()) != nullptr)
    {
        if (m_pStyle->getParent().getElementType() == SmMlElementType::MlMstyle
            || m_pStyle->getParent().getElementType() == SmMlElementType::MlMath)
            break;
    }

    // Parent inheritation
    // Mathcolor, mathsize, dir and displaystyle are inherited from parent
    m_pElement->setAttribute(m_pParent->getAttributte(SmMlAttributeValueType::MlMathcolor));
    m_pElement->setAttribute(m_pParent->getAttributte(SmMlAttributeValueType::MlMathsize));
    m_pElement->setAttribute(m_pParent->getAttributte(SmMlAttributeValueType::MlDir));
    m_pElement->setAttribute(m_pParent->getAttributte(SmMlAttributeValueType::MlDisplaystyle));

    // Inherit operator dictionary overwrites
    if (m_pStyle != nullptr
        && (m_pElement->getMlElementType() == SmMlElementType::MlMo
            || m_pElement->getMlElementType() == SmMlElementType::MlMstyle
            || m_pElement->getMlElementType() == SmMlElementType::MlMath))
    {
        // TODO fetch operator dictionary first and then overwrite
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlAccent))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlAccent));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlFence))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlFence));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlLspace))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlLspace));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMaxsize))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlMaxsize));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMinsize))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlMinsize));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlMovablelimits))
            m_pElement->setAttribute(
                m_pStyle->getAttributte(SmMlAttributeValueType::MlMovablelimits));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlRspace))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlRspace));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlSeparator))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlSeparator));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlStretchy))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlStretchy));
        if (m_pStyle->isAttributeSet(SmMlAttributeValueType::MlSymmetric))
            m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlSymmetric));

        // Set form based in position
        SmMlAttrbute aAttribute(SmMlAttributeValueType::MlForm);
        SmMlForm aForm;
        if (m_pElement->getSubElementId() == 0)
            aForm = { SmMlAttributeValueType::MlPrefix };
        else
            aForm = { SmMlAttributeValueType::MlInfix };
        aAttribute.setMlForm(aForm);
        m_pElement->setAttribute(&aAttribute);
    }

    // Inherit mathvariant
    if (m_pStyle->isAttributSet(SmMlAttributeValueType::MlMathvariant))
        m_pElement->setAttribute(m_pStyle->getAttributte(SmMlAttributeValueType::MlMathvariant));
    else
    {
        SmMlAttrbute aAttribute(SmMlAttributeValueType::MlMathvariant);
        SmMlMathvariant aMathvariant;
        if (m_pElement->getMlElementType() == SmMlElementType::MlMi)
        {
            sal_uInt32 nIndexUtf16 = 0;
            // Check if there is only one code point
            m_pElement->getText().iterateCodePoints(&nIndexUtf16, 1);
            // Mathml says that 1 code point -> italic
            if (nIndexUtf16 == m_pElement->getText().getLength())
                aMathvariant.m_aMathvariant = SmMlAttributeValueMathvariant::italic;
            else
                aMathvariant.m_aMathvariant = SmMlAttributeValueMathvariant::normal;
        }
        else
            aMathvariant.m_aMathvariant = SmMlAttributeValueMathvariant::normal;
        aAttribute.setMlMathvariant(aMathvariant);
        aAttribute.setSet(false);
        m_pElement->setAttribute(&aAttribute);
    }
}

SmLengthValue SmMlImportContext::handleLengthAttributte(const OUString& aAttribute)
{
    // Locate unit indication
    int32_t nUnitPos;
    for (nUnitPos = 0;
         nUnitPos < aAttribute.getLength()
         && (rtl::isAsciiHexDigit(aAttribute[nUnitPos]) || aAttribute[nUnitPos] == '.');
         ++nUnitPos)
        ;

    // Find unit
    SmLengthUnit nUnit = SmLengthUnit::MlM;
    if (nUnitPos != aAttribute.getLength())
    {
        OUString aUnit = aUnit.copy(nUnitPos);
        if (aUnit.compareToIgnoreAsciiCaseAscii("ex"))
            nUnit = SmLengthUnit::MlEx;
        if (aUnit.compareToIgnoreAsciiCaseAscii("px"))
            nUnit = SmLengthUnit::MlPx;
        if (aUnit.compareToIgnoreAsciiCaseAscii("in"))
            nUnit = SmLengthUnit::MlIn;
        if (aUnit.compareToIgnoreAsciiCaseAscii("cm"))
            nUnit = SmLengthUnit::MlCm;
        if (aUnit.compareToIgnoreAsciiCaseAscii("mm"))
            nUnit = SmLengthUnit::MlMm;
        if (aUnit.compareToIgnoreAsciiCaseAscii("pt"))
            nUnit = SmLengthUnit::MlPt;
        if (aUnit.compareToIgnoreAsciiCaseAscii("pc"))
            nUnit = SmLengthUnit::MlPc;
        if (aUnit.compareToIgnoreAsciiCaseAscii("%"))
            nUnit = SmLengthUnit::MlP;
        else
            declareMlError();
    }

    // Get value
    OUString aValue = aAttribute.copy(0, nUnitPos);
    double nValue = aValue.toDouble();
    if (nValue == 0)
    {
        nUnit = SmLengthUnit::MlM;
        nValue = 1.0;
        declareMlError();
    }

    // Return
    SmLengthValue aLengthValue = { nUnit, nValue, new OUString(aAttribute) };
    return aLengthValue;
}

void SmMlImportContext::handleAttributes(Reference<XFastAttributeList>& aAttributeList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(aAttributeList))
    {
        SmMlAttribute aAttribute(SmMlAttributeValueType::NMlEmpty);
        switch (aIter.getToken() & TOKEN_MASK)
        {
            case XML_ACCENT:
            {
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlAccent);
                    SmMlAccent aAccent = { SmMlAttributeValueAccent::MlTrue };
                    aAttribute.setMlAccent(&aAccent);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlAccent);
                    SmMlAccent aAccent = { SmMlAttributeValueAccent::MlFalse };
                    aAttribute.setMlAccent(&aAccent);
                }
                else
                {
                    declareMlError();
                }
                break;
            }
            case XML_DIR:
            {
                if (IsXMLToken(aIter, XML_RTL))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDir);
                    SmMlDir aDir = { SmMlAttributeValueDir::MlRtl };
                    aAttribute.setMlDir(&aDir);
                }
                else if (IsXMLToken(aIter, XML_LTR))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDir);
                    SmMlDir aDir = { SmMlAttributeValueDir::MlLtr };
                    aAttribute.setMlDir(&aDir);
                }
                else
                {
                    declareMlError();
                }
                break;
            }
            case XML_DISPLAYSTYLE:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDisplaystyle);
                    SmMlDisplaystyle aDisplaystyle = { SmMlAttributeValueDisplaystyle::MlTrue };
                    aAttribute.setMlDisplaystyle(&aDisplaystyle);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlDisplaystyle);
                    SmMlDisplaystyle aDisplaystyle = { SmMlAttributeValueDisplaystyle::MlFalse };
                    aAttribute.setMlDisplaystyle(&aDisplaystyle);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_FENCE:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlFence);
                    SmMlFence aFence = { SmMlAttributeValueFence::MlTrue };
                    aAttribute.setMlFence(&aFence);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlFence);
                    SmMlFence aFence = { SmMlAttributeValueFence::MlFalse };
                    aAttribute.setMlFence(&aFence);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_HREF:
            {
                aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlHref);
                OUString* aRef = new OUString(aIter.toString());
                SmMlHref aHref = { SmMlAttributeValueHref::NMlValid, aRef };
                aAttribute.setMlHref(&aHref);
                break;
            }
            case XML_LSPACE:
            {
                SmMlLspace aLspace;
                aLspace.m_aLengthValue = handleLengthAttributte(aIter.toString());
                aAttribute.setMlLspace(&aLspace);
                break;
            }
            case XML_MATHBACKGROUND:
            {
                if (IsXMLToken(aIter, XML_TRANSPARENT))
                {
                    SmMlMathbackground aMathbackground
                        = { SmMlAttributeValueMathbackground::MlTransparent, COL_TRANSPARENT };
                    aAttribute.setMlMathbackground(&aMathbackground);
                }
                else
                {
                    Color aColor = Color::STRtoRGB(aIter.toString());
                    SmMlMathbackground aMathbackground
                        = { SmMlAttributeValueMathbackground::MlRgb, aColor };
                    aAttribute.setMlMathbackground(&aMathbackground);
                }
                break;
            }
            case XML_MATHCOLOR:
            {
                if (IsXMLToken(aIter, XML_DEFAULT))
                {
                    SmMlMathcolor aMathcolor
                        = { SmMlAttributeValueMathcolor::MlDefault, COL_BLACK };
                    aAttribute.setMlMathcolor(&aMathcolor);
                }
                else
                {
                    Color aColor = Color::STRtoRGB(aIter.toString());
                    SmMlMathcolor aMathcolor = { SmMlAttributeValueMathcolor::MlRgb, aColor };
                    aAttribute.setMlMathcolor(&aMathcolor);
                }
                break;
            }
            case XML_MATHSIZE:
            {
                SmMlMathsize aMathsize;
                aMathsize.m_aLengthValue = handleLengthAttributte(aIter.toString());
                aAttribute.setMlMathsize(&aMathsize);
                break;
            }
            case XML_MATHVARIANT:
            {
                OUString aVariant = aIter.toString();
                SmMlAttributeValueMathvariant nVariant = SmMlAttributeValueMathvariant::normal;
                if (aVariant.compareTo(u"normal"))
                    nVariant = SmMlAttributeValueMathvariant::normal;
                else if (aVariant.compareTo(u"bold"))
                    nVariant = SmMlAttributeValueMathvariant::bold;
                else if (aVariant.compareTo(u"italic"))
                    nVariant = SmMlAttributeValueMathvariant::italic;
                else if (aVariant.compareTo(u"double-struck"))
                    nVariant = SmMlAttributeValueMathvariant::double_struck;
                else if (aVariant.compareTo(u"script"))
                    nVariant = SmMlAttributeValueMathvariant::script;
                else if (aVariant.compareTo(u"fraktur"))
                    nVariant = SmMlAttributeValueMathvariant::fraktur;
                else if (aVariant.compareTo(u"sans-serif"))
                    nVariant = SmMlAttributeValueMathvariant::sans_serif;
                else if (aVariant.compareTo(u"monospace"))
                    nVariant = SmMlAttributeValueMathvariant::monospace;
                else if (aVariant.compareTo(u"bold-italic"))
                    nVariant = SmMlAttributeValueMathvariant::bold_italic;
                else if (aVariant.compareTo(u"bold-fracktur"))
                    nVariant = SmMlAttributeValueMathvariant::bold_fraktur;
                else if (aVariant.compareTo(u"bold-script"))
                    nVariant = SmMlAttributeValueMathvariant::bold_script;
                else if (aVariant.compareTo(u"bold-sans-serif"))
                    nVariant = SmMlAttributeValueMathvariant::bold_sans_serif;
                else if (aVariant.compareTo(u"sans-serif-italic"))
                    nVariant = SmMlAttributeValueMathvariant::sans_serif_italic;
                else if (aVariant.compareTo(u"sans-serif-bold-italic"))
                    nVariant = SmMlAttributeValueMathvariant::sans_serif_bold_italic;
                else if (aVariant.compareTo(u"initial"))
                    nVariant = SmMlAttributeValueMathvariant::initial;
                else if (aVariant.compareTo(u"tailed"))
                    nVariant = SmMlAttributeValueMathvariant::tailed;
                else if (aVariant.compareTo(u"looped"))
                    nVariant = SmMlAttributeValueMathvariant::looped;
                else if (aVariant.compareTo(u"stretched"))
                    nVariant = SmMlAttributeValueMathvariant::stretched;
                else
                    declareMlError();
                break;
            }
            case XML_MAXSIZE:
            {
                SmMlMaxsize aMaxsize;
                if (IsXMLToken(aIter, XML_INFINITY))
                    aMaxsize.m_aMaxsize = SmMlAttributeValueMaxsize::MlInfinity;
                else
                {
                    aMaxsize.m_aMaxsize = SmMlAttributeValueMaxsize::MlFinite;
                    aMaxsize.m_aLengthValue = handleLengthAttributte(aIter.toString());
                }
                break;
                aAttribute.setMlMaxsize(&aMaxsize);
            }
            case XML_MINSIZE:
            {
                SmMlMinsize aMinsize;
                aMinsize.m_aLengthValue = handleLengthAttributte(aIter.toString());
                aAttribute.setMlMinsize(&aMinsize);
                break;
            }
            case XML_MOVABLELIMITS:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlMovablelimits);
                    SmMlMovablelimits aMovablelimits = { SmMlAttributeValueMovablelimits::MlTrue };
                    aAttribute.setMlMovablelimits(&aMovablelimits);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlMovablelimits);
                    SmMlMovablelimits aMovablelimits = { SmMlAttributeValueMovablelimits::MlFalse };
                    aAttribute.setMlMovablelimits(&aMovablelimits);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_RSPACE:
            {
                SmMlRspace aRspace;
                aRspace.m_aLengthValue = handleLengthAttributte(aIter.toString());
                aAttribute.setMlRspace(&aRspace);
                break;
            }
            case XML_SEPARATOR:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSeparator);
                    SmMlSeparator aSeparator = { SmMlAttributeValueSeparator::MlTrue };
                    aAttribute.setMlSeparator(&aSeparator);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSeparator);
                    SmMlSeparator aSeparator = { SmMlAttributeValueSeparator::MlFalse };
                    aAttribute.setMlSeparator(&aSeparator);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_STRETCHY:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlStretchy);
                    SmMlStretchy aStretchy = { SmMlAttributeValueStretchy::MlTrue };
                    aAttribute.setMlStretchy(&aStretchy);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlStretchy);
                    SmMlStretchy aStretchy = { SmMlAttributeValueStretchy::MlFalse };
                    aAttribute.setMlStretchy(&aStretchy);
                }
                else
                {
                    declareMlError();
                }
                break;
            case XML_SYMMETRIC:
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSymmetric);
                    SmMlSymmetric aSymmetric = { SmMlAttributeValueSymmetric::MlTrue };
                    aAttribute.setMlSymmetric(&aSymmetric);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    aAttribute.setMlAttributeValueType(SmMlAttributeValueType::MlSymmetric);
                    SmMlSymmetric aSymmetric = { SmMlAttributeValueSymmetric::MlFalse };
                    aAttribute.setMlSymmetric(&aSymmetric);
                }
                else
                {
                    declareMlError();
                }
                break;
            default:
                declareMlError();
                break;
        }
        if (aAttribute.isNullAttribute())
            declareMlError();
        else
            m_pElement->setAttribute(&aAttribute);
    }
}

void SmMlImportContext::characters(const OUString& aChars) { m_pElement->setText(aChars); }

void SmMlImportContext::startFastElement(sal_Int32 nElement,
                                         Reference<XFastAttributeList>& aAttributeList)
{
    SvXMLImportContext::startFastElement(nElement, aAttributeList);
    switch (nElement)
    {
        case XML_ELEMENT(MATH, XML_MATH):
            m_pElement = new SmMlElement(SmMlElementType::MlMath);
            break;
        case XML_ELEMENT(MATH, XML_MI):
            m_pElement = new SmMlElement(SmMlElementType::MlMi);
            break;
        case XML_ELEMENT(MATH, XML_MERROR):
            m_pElement = new SmMlElement(SmMlElementType::MlMerror);
            break;
        case XML_ELEMENT(MATH, XML_MN):
            m_pElement = new SmMlElement(SmMlElementType::MlMn);
            break;
        case XML_ELEMENT(MATH, XML_MO):
            m_pElement = new SmMlElement(SmMlElementType::MlMo);
            break;
        case XML_ELEMENT(MATH, XML_MROW):
            m_pElement = new SmMlElement(SmMlElementType::MlMrow);
            break;
        case XML_ELEMENT(MATH, XML_MTEXT):
            m_pElement = new SmMlElement(SmMlElementType::MlMtext);
            break;
        case XML_ELEMENT(MATH, XML_MSTYLE):
            m_pElement = new SmMlElement(SmMlElementType::MlMstyle);
            break;
        default:
            m_pElement = new SmMlElement(SmMlElementType::NMlEmpty);
            declareMlError();
            break;
    }
    m_pParent->setSubElement(m_pParent->getSubElementsCount(), m_pElement);
    inheritStyle();
    handleAttributes(aAttributeList);
}
}

// SmMlImport
/*************************************************************************************************/

SmMlImport::SmMlImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                       OUString const& implementationName, SvXMLImportFlags nImportFlags)
    : SvXMLImport(rContext, implementationName, nImportFlags)
    , Tree(nullptr)
    , m_bSuccess(false)
    , m_nSmSyntaxVersion(SM_MOD()->GetConfig()->GetDefaultSmSyntaxVersion())
{
}

/** Handles an error on the mathml structure
 */
void SmMlImport::declareMlError() { SAL_WARN("starmath", "MathML error"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
