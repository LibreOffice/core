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

namespace writerfilter
{
namespace rtftok
{
RTFValue::RTFValue(int nValue, OUString sValue, const RTFSprms& rAttributes, const RTFSprms& rSprms,
                   uno::Reference<drawing::XShape> xShape, uno::Reference<io::XInputStream> xStream,
                   uno::Reference<embed::XEmbeddedObject> xObject, bool bForceString,
                   const RTFShape& aShape, const RTFPicture& rPicture)
    : m_nValue(nValue)
    , m_sValue(std::move(sValue))
    , m_pAttributes(new RTFSprms(rAttributes))
    , m_pSprms(new RTFSprms(rSprms))
    , m_xShape(std::move(xShape))
    , m_xStream(std::move(xStream))
    , m_xObject(std::move(xObject))
    , m_bForceString(bForceString)
    , m_pShape(new RTFShape(aShape))
    , m_pPicture(new RTFPicture(rPicture))
{
}

RTFValue::RTFValue()
    : m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue)
    , m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(OUString sValue, bool bForce)
    : m_sValue(std::move(sValue))
    , m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_bForceString(bForce)
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(const RTFSprms& rAttributes)
    : m_pAttributes(new RTFSprms(rAttributes))
    , m_pSprms(new RTFSprms())
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(const RTFSprms& rAttributes, const RTFSprms& rSprms)
    : m_pAttributes(new RTFSprms(rAttributes))
    , m_pSprms(new RTFSprms(rSprms))
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(uno::Reference<drawing::XShape> xShape)
    : m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_xShape(std::move(xShape))
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(uno::Reference<io::XInputStream> xStream)
    : m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_xStream(std::move(xStream))
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(uno::Reference<embed::XEmbeddedObject> xObject)
    : m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_xObject(std::move(xObject))
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(const RTFShape& aShape)
    : m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_pShape(new RTFShape(aShape))
    , m_pPicture(new RTFPicture())
{
}

RTFValue::RTFValue(const RTFPicture& rPicture)
    : m_pAttributes(new RTFSprms())
    , m_pSprms(new RTFSprms())
    , m_pShape(new RTFShape())
    , m_pPicture(new RTFPicture(rPicture))
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
    else
        ret <<= static_cast<sal_Int32>(m_nValue);
    return ret;
}

RTFShape& RTFValue::getShape() const { return *m_pShape; }

RTFPicture& RTFValue::getPicture() const { return *m_pPicture; }

writerfilter::Reference<Properties>::Pointer_t RTFValue::getProperties()
{
    return new RTFReferenceProperties(*m_pAttributes, *m_pSprms);
}

writerfilter::Reference<BinaryObj>::Pointer_t RTFValue::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

#ifdef DEBUG_WRITERFILTER
std::string RTFValue::toString() const
{
    if (!m_sValue.isEmpty() || m_bForceString)
        return OUStringToOString(m_sValue, RTL_TEXTENCODING_UTF8).getStr();

    return OString::number(m_nValue).getStr();
}
#endif

RTFValue* RTFValue::Clone()
{
    return new RTFValue(m_nValue, m_sValue, *m_pAttributes, *m_pSprms, m_xShape, m_xStream,
                        m_xObject, m_bForceString, *m_pShape, *m_pPicture);
}

RTFValue* RTFValue::CloneWithSprms(RTFSprms const& rAttributes, RTFSprms const& rSprms)
{
    return new RTFValue(m_nValue, m_sValue, rAttributes, rSprms, m_xShape, m_xStream, m_xObject,
                        m_bForceString, *m_pShape, *m_pPicture);
}

bool RTFValue::equals(RTFValue& rOther)
{
    if (m_nValue != rOther.m_nValue)
        return false;
    if (m_sValue != rOther.m_sValue)
        return false;
    if (m_pAttributes->size() != rOther.m_pAttributes->size())
        return false;
    if (!m_pAttributes->equals(rOther))
        return false;
    if (m_pSprms->size() != rOther.m_pSprms->size())
        return false;
    if (!m_pSprms->equals(rOther))
        return false;
    return true;
}

RTFSprms& RTFValue::getAttributes() { return *m_pAttributes; }

RTFSprms& RTFValue::getSprms() { return *m_pSprms; }

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
