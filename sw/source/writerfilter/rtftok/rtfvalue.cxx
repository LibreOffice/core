/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "rtfreferenceproperties.hxx"
#include "rtfdocumentimpl.hxx"
#include <com/sun/star/embed/XEmbeddedObject.hpp>

using namespace com::sun::star;

namespace writerfilter::rtftok
{
RTFValue::RTFValue(int nValue, OUString sValue, const RTFSprms* pAttributes, const RTFSprms* pSprms,
                   uno::Reference<drawing::XShape> xShape, uno::Reference<io::XInputStream> xStream,
                   uno::Reference<embed::XEmbeddedObject> xObject, bool bForceString,
                   const RTFShape* pShape, const RTFPicture* pPicture)
    : m_nValue(nValue)
    , m_sValue(std::move(sValue))
    , m_xShape(std::move(xShape))
    , m_xStream(std::move(xStream))
    , m_xObject(std::move(xObject))
    , m_bForceString(bForceString)
{
    if (pAttributes)
        m_pAttributes = new RTFSprms(*pAttributes);
    if (pSprms)
        m_pSprms = new RTFSprms(*pSprms);
    if (pShape)
        m_pShape = new RTFShape(*pShape);
    if (pPicture)
        m_pPicture = new RTFPicture(*pPicture);
}

RTFValue::RTFValue() {}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue)
{
}

RTFValue::RTFValue(OUString sValue, bool bForce)
    : m_sValue(std::move(sValue))
    , m_bForceString(bForce)
{
}

RTFValue::RTFValue(const RTFSprms& rAttributes)
    : m_pAttributes(new RTFSprms(rAttributes))
{
}

RTFValue::RTFValue(const RTFSprms& rAttributes, const RTFSprms& rSprms)
    : m_pAttributes(new RTFSprms(rAttributes))
    , m_pSprms(new RTFSprms(rSprms))
{
}

RTFValue::RTFValue(uno::Reference<drawing::XShape> xShape)
    : m_xShape(std::move(xShape))
{
}

RTFValue::RTFValue(uno::Reference<io::XInputStream> xStream)
    : m_xStream(std::move(xStream))
{
}

RTFValue::RTFValue(uno::Reference<embed::XEmbeddedObject> xObject)
    : m_xObject(std::move(xObject))
{
}

RTFValue::RTFValue(const RTFShape& aShape)
    : m_pShape(new RTFShape(aShape))
{
}

RTFValue::RTFValue(const RTFPicture& rPicture)
    : m_pPicture(new RTFPicture(rPicture))
{
}

RTFValue::RTFValue(text::GraphicCrop const& rCrop)
    : m_oCrop(rCrop)
{
}

RTFValue::~RTFValue() = default;

int RTFValue::getInt() const { return m_nValue; }

OUString RTFValue::getString() const
{
    if (!m_sValue.isEmpty() || m_bForceString)
        return m_sValue;

    return OUString::number(m_nValue);
}

void RTFValue::setString(const OUString& sValue) { m_sValue = sValue; }

uno::Any RTFValue::getAny() const
{
    uno::Any ret;
    if (!m_sValue.isEmpty() || m_bForceString)
        ret <<= m_sValue;
    else if (m_xShape.is())
        ret <<= m_xShape;
    else if (m_xStream.is())
        ret <<= m_xStream;
    else if (m_xObject.is())
        ret <<= m_xObject;
    else if (m_oCrop)
    {
        ret <<= *m_oCrop;
    }
    else
        ret <<= static_cast<sal_Int32>(m_nValue);
    return ret;
}

RTFShape& RTFValue::getShape() const
{
    if (!m_pShape)
        m_pShape = new RTFShape();
    return *m_pShape;
}

RTFPicture& RTFValue::getPicture() const
{
    if (!m_pPicture)
        m_pPicture = new RTFPicture;
    return *m_pPicture;
}

writerfilter::Reference<Properties>::Pointer_t RTFValue::getProperties()
{
    return new RTFReferenceProperties(getAttributes(), getSprms());
}

writerfilter::Reference<BinaryObj>::Pointer_t RTFValue::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

#ifdef DBG_UTIL
std::string RTFValue::toString() const
{
    if (!m_sValue.isEmpty() || m_bForceString)
        return std::string(OUStringToOString(m_sValue, RTL_TEXTENCODING_UTF8));

    return std::string(OString::number(m_nValue));
}
#endif

RTFValue* RTFValue::Clone() const
{
    return new RTFValue(m_nValue, m_sValue, m_pAttributes.get(), m_pSprms.get(), m_xShape,
                        m_xStream, m_xObject, m_bForceString, m_pShape.get(), m_pPicture.get());
}

RTFValue* RTFValue::CloneWithSprms(RTFSprms const& rAttributes, RTFSprms const& rSprms) const
{
    return new RTFValue(m_nValue, m_sValue, &rAttributes, &rSprms, m_xShape, m_xStream, m_xObject,
                        m_bForceString, m_pShape.get(), m_pPicture.get());
}

bool RTFValue::equals(const RTFValue& rOther) const
{
    if (m_nValue != rOther.m_nValue)
        return false;
    if (m_sValue != rOther.m_sValue)
        return false;

    if (m_pAttributes && rOther.m_pAttributes)
    {
        if (m_pAttributes->size() != rOther.m_pAttributes->size())
            return false;
        if (!m_pAttributes->equals(*rOther.m_pAttributes))
            return false;
    }
    else if (m_pAttributes && m_pAttributes->size())
    {
        return false;
    }
    else if (rOther.m_pAttributes && rOther.m_pAttributes->size())
    {
        return false;
    }

    if (m_pSprms && rOther.m_pSprms)
    {
        if (m_pSprms->size() != rOther.m_pSprms->size())
            return false;
        if (!m_pSprms->equals(*rOther.m_pSprms))
            return false;
    }
    else if (m_pSprms && m_pSprms->size())
    {
        return false;
    }
    else if (rOther.m_pSprms && rOther.m_pSprms->size())
    {
        return false;
    }

    return true;
}

RTFSprms& RTFValue::getAttributes() const
{
    if (!m_pAttributes)
        m_pAttributes = new RTFSprms();
    return *m_pAttributes;
}

RTFSprms& RTFValue::getSprms() const
{
    if (!m_pSprms)
        m_pSprms = new RTFSprms();
    return *m_pSprms;
}

} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
