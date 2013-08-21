/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <rtfreferenceproperties.hxx>
#include <rtfdocumentimpl.hxx>

namespace writerfilter {
namespace rtftok {


RTFValue::RTFValue(int nValue, OUString sValue, RTFSprms rAttributes,
        RTFSprms rSprms, uno::Reference<drawing::XShape> xShape,
        uno::Reference<io::XInputStream> xStream, uno::Reference<embed::XEmbeddedObject> xObject, bool bForceString,
        RTFShape aShape)
    : m_nValue(nValue),
    m_sValue(sValue),
    m_xShape(xShape),
    m_xStream(xStream),
    m_xObject(xObject),
    m_bForceString(bForceString)
{
    m_pAttributes.reset(new RTFSprms(rAttributes));
    m_pSprms.reset(new RTFSprms(rSprms));
    m_pShape.reset(new RTFShape(aShape));
}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue),
    m_sValue(),
    m_xShape(),
    m_xStream(),
    m_xObject(),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms());
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(OUString sValue, bool bForce)
    : m_nValue(),
    m_sValue(sValue),
    m_xShape(),
    m_xStream(),
    m_xObject(),
    m_bForceString(bForce)
{
    m_pAttributes.reset(new RTFSprms());
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(RTFSprms rAttributes)
    : m_nValue(),
    m_sValue(),
    m_xShape(),
    m_xStream(),
    m_xObject(),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms(rAttributes));
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(RTFSprms rAttributes, RTFSprms rSprms)
    : m_nValue(),
    m_sValue(),
    m_xShape(),
    m_xStream(),
    m_xObject(),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms(rAttributes));
    m_pSprms.reset(new RTFSprms(rSprms));
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(uno::Reference<drawing::XShape> rShape)
    : m_nValue(),
    m_sValue(),
    m_xShape(rShape),
    m_xStream(),
    m_xObject(),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms());
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(uno::Reference<io::XInputStream> rStream)
    : m_nValue(),
    m_sValue(),
    m_xShape(),
    m_xStream(rStream),
    m_xObject(),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms());
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(uno::Reference<embed::XEmbeddedObject> xObject)
    : m_nValue(),
    m_sValue(),
    m_xShape(),
    m_xStream(),
    m_xObject(xObject),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms());
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(RTFShape aShape)
    : m_nValue(),
    m_sValue(),
    m_xShape(),
    m_xStream(),
    m_xObject(),
    m_bForceString(false)
{
    m_pAttributes.reset(new RTFSprms());
    m_pSprms.reset(new RTFSprms());
    m_pShape.reset(new RTFShape(aShape));
}

RTFValue::~RTFValue()
{
}

int RTFValue::getInt() const
{
    return m_nValue;
}

OUString RTFValue::getString() const
{
    if (!m_sValue.isEmpty() || m_bForceString)
        return m_sValue;
    else
        return OUString::number(m_nValue);
}

void RTFValue::setString(OUString sValue)
{
    m_sValue = sValue;
}

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

RTFShape& RTFValue::getShape() const
{
    return *m_pShape;
}

writerfilter::Reference<Properties>::Pointer_t RTFValue::getProperties()
{
    writerfilter::Reference<Properties>::Pointer_t const pProperties(
            new RTFReferenceProperties(*m_pAttributes, *m_pSprms)
            );
    return pProperties;
}

writerfilter::Reference<Stream>::Pointer_t RTFValue::getStream()
{
    return writerfilter::Reference<Stream>::Pointer_t();
}

writerfilter::Reference<BinaryObj>::Pointer_t RTFValue::getBinary()
{
    return writerfilter::Reference<BinaryObj>::Pointer_t();
}

std::string RTFValue::toString() const
{
    if (!m_sValue.isEmpty() || m_bForceString)
        return OUStringToOString(m_sValue, RTL_TEXTENCODING_UTF8).getStr();
    else
        return OString::number(m_nValue).getStr();
}

RTFValue* RTFValue::Clone()
{
    return new RTFValue(m_nValue, m_sValue, *m_pAttributes, *m_pSprms, m_xShape, m_xStream, m_xObject, m_bForceString, *m_pShape);
}

bool RTFValue::equals(RTFValue& rOther)
{
    return m_nValue == rOther.m_nValue;
}

RTFSprms& RTFValue::getAttributes()
{
    return *m_pAttributes;
}

RTFSprms& RTFValue::getSprms()
{
    return *m_pSprms;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
