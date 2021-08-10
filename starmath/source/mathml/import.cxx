/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <mathml/import.hxx>

#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnamespace.hxx>
#include <xmloff/xmltoken.hxx>

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
    SmMlElement* m_pSmMlElement;
    SmMlElement* m_pParent;

public:
    SmMlImportContext(SmMlImport& rImport, SmMlElement* pParent)
        : SvXMLImportContext(rImport)
        , m_pSmMlElement(nullptr)
        , m_pParent(pParent)
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

    /** Handle mathml attributes
    */
    void handleAttributes(Reference<XFastAttributeList>& aAttributeList);
};

void SmMlImportContext::declareMlError()
{
    SmMlImport& aSmMlImport = static_cast<SmMlImport&>(GetImport());
    aSmMlImport.declareMlError();
}

void SmMlImportContext::handleAttributes(Reference<XFastAttributeList>& aAttributeList)
{
    for (auto& aIter : sax_fastparser::castToFastAttributeList(aAttributeList))
    {
        switch (aIter.getToken() & TOKEN_MASK)
        {
            case XML_ACCENT:
            {
                if (IsXMLToken(aIter, XML_TRUE))
                {
                    SmMlAttribute aAttribute(SmMlAttributeValueType::MlAccent);
                    struct SmMlAccent aAccent = { SmMlAttributeValueAccent::MlTrue };
                    aAttribute.setMlAccent(&aAccent);
                }
                else if (IsXMLToken(aIter, XML_FALSE))
                {
                    SmMlAttribute aAttribute(SmMlAttributeValueType::MlAccent);
                    struct SmMlAccent aAccent = { SmMlAttributeValueAccent::MlFalse };
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
                    SmMlAttribute aAttribute(SmMlAttributeValueType::MlDir);
                    struct SmMlDir aDir = { SmMlAttributeValueDir::MlRtl };
                    aAttribute.setMlAccent(&aAccent);
                }
                else if (IsXMLToken(aIter, XML_LTR))
                {
                    SmMlAttribute aAttribute(SmMlAttributeValueType::MlDir);
                    struct SmMlDir aDir = { SmMlAttributeValueDir::MlLtr };
                    aAttribute.setMlAccent(&aDir);
                }
                else
                {
                    declareMlError();
                }
                break;
            }
            case XML_DISPLAYSTYLE:
                break;
            case XML_FENCE:
                break;
            case XML_HREF:
                break;
            case XML_LSPACE:
                break;
            case XML_MATHBACKGROUND:
                break;
            case XML_MATHCOLOR:
                break;
            case XML_MATHSIZE:
                break;
            case XML_MATHVARIANT:
                break;
            case XML_MAXSIZE:
                break;
            case XML_MINSIZE:
                break;
            case XML_MOVABLELIMITS:
                break;
            case XML_RSPACE:
                break;
            case XML_SEPARATOR:
                break;
            case XML_STRETCHY:
                break;
            case XML_SYMMETRIC:
                break;
            default:
                break;
        }
    }
}

void SmMlImportContext::characters(const OUString& aChars) { m_pSmMlElement->setText(aChars); }

void SmMlImportContext::startFastElement(sal_Int32 nElement,
                                         Reference<XFastAttributeList>& aAttributeList)
{
    SvXMLImportContext::startFastElement(nElement, aAttributeList);
    switch (nElement)
    {
        case XML_ELEMENT(MATH, XML_MATH):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMath);
            break;
        case XML_ELEMENT(MATH, XML_MI):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMi);
            break;
        case XML_ELEMENT(MATH, XML_MERROR):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMerror);
            break;
        case XML_ELEMENT(MATH, XML_MN):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMn);
            break;
        case XML_ELEMENT(MATH, XML_MO):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMo);
            break;
        case XML_ELEMENT(MATH, XML_MROW):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMrow);
            break;
        case XML_ELEMENT(MATH, XML_MTEXT):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMtext);
            break;
        case XML_ELEMENT(MATH, XML_MSTYLE):
            m_pSmMlElement = new SmMlElement(SmMlElementType::MlMstyle);
            break;
        default:
            m_pSmMlElement = new SmMlElement(SmMlElementType::NMlEmpty);
            declareMlError();
            break;
    }
    m_pParent->setSubElement(m_pParent->getSubElementsCount(), m_pSmMlElement);
}
}

// SmMlImport
/*************************************************************************************************/

SmMlImport::SmMlImport(const css::uno::Reference<css::uno::XComponentContext>& rContext,
                       OUString const& implementationName, SvXMLImportFlags nImportFlags)
    : SvXMLImport(rContext, implementationName, nImportFlags)
    , m_pElementTree(nullptr)
    , m_bSuccess(false)
    , m_nSmSyntaxVersion(SM_MOD()->GetConfig()->GetDefaultSmSyntaxVersion())
{
}

/** Handles an error on the mathml structure
 */
void SmMlImport::declareMlError() { SAL_WARN("starmath", "MathML error"); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
