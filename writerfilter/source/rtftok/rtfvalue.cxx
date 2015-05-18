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

using namespace com::sun::star;

namespace writerfilter
{
namespace rtftok
{


RTFValue::RTFValue(int nValue, const OUString& sValue,
                   RTFSprms rAttributes, RTFSprms rSprms,
                   uno::Reference<drawing::XShape> const& xShape,
                   uno::Reference<io::XInputStream> const& xStream,
                   uno::Reference<embed::XEmbeddedObject> const& xObject,
                   bool bForceString,
                   const RTFShape& aShape)
    : m_nValue(nValue),
      m_sValue(sValue),
      m_pAttributes(std::make_shared<RTFSprms>(rAttributes)),
      m_pSprms(std::make_shared<RTFSprms>(rSprms)),
      m_xShape(xShape),
      m_xStream(xStream),
      m_xObject(xObject),
      m_bForceString(bForceString),
      m_pShape(std::make_shared<RTFShape>(aShape))
{
}

RTFValue::RTFValue()
    : m_nValue(0),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(),
      m_xObject(),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(),
      m_xObject(),
      m_bForceString(false)
{
    m_pShape.reset(new RTFShape());
}

RTFValue::RTFValue(const OUString& sValue, bool bForce)
    : m_nValue(),
      m_sValue(sValue),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(),
      m_xObject(),
      m_bForceString(bForce),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(RTFSprms rAttributes)
    : m_nValue(),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>(rAttributes)),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(),
      m_xObject(),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(RTFSprms rAttributes, RTFSprms rSprms)
    : m_nValue(),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>(rAttributes)),
      m_pSprms(std::make_shared<RTFSprms>(rSprms)),
      m_xShape(),
      m_xStream(),
      m_xObject(),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(uno::Reference<drawing::XShape> const& xShape)
    : m_nValue(),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(xShape),
      m_xStream(),
      m_xObject(),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(uno::Reference<io::XInputStream> const& xStream)
    : m_nValue(),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(xStream),
      m_xObject(),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(uno::Reference<embed::XEmbeddedObject> const& xObject)
    : m_nValue(),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(),
      m_xObject(xObject),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>())
{
}

RTFValue::RTFValue(const RTFShape& aShape)
    : m_nValue(),
      m_sValue(),
      m_pAttributes(std::make_shared<RTFSprms>()),
      m_pSprms(std::make_shared<RTFSprms>()),
      m_xShape(),
      m_xStream(),
      m_xObject(),
      m_bForceString(false),
      m_pShape(std::make_shared<RTFShape>(aShape))
{
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

void RTFValue::setString(const OUString& sValue)
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
    return std::make_shared<RTFReferenceProperties>(*m_pAttributes, *m_pSprms);
}

writerfilter::Reference<Stream>::Pointer_t RTFValue::getStream()
{
    return writerfilter::Reference<Stream>::Pointer_t();
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
    else
        return OString::number(m_nValue).getStr();
}
#endif

RTFValue* RTFValue::Clone()
{
    return new RTFValue(m_nValue, m_sValue, *m_pAttributes, *m_pSprms, m_xShape, m_xStream, m_xObject, m_bForceString, *m_pShape);
}

RTFValue* RTFValue::CloneWithSprms(RTFSprms const& rAttributes, RTFSprms const& rSprms)
{
    return new RTFValue(m_nValue, m_sValue, rAttributes, rSprms, m_xShape, m_xStream, m_xObject, m_bForceString, *m_pShape);
}

bool RTFValue::equals(RTFValue& rOther)
{
    if (m_nValue != rOther.m_nValue)
        return false;
    if (m_sValue != rOther.m_sValue)
        return false;
    if (m_pAttributes->size() != rOther.m_pAttributes->size())
        return false;
    else if (!m_pAttributes->equals(rOther))
        return false;
    if (m_pSprms->size() != rOther.m_pSprms->size())
        return false;
    else if (!m_pSprms->equals(rOther))
        return false;
    return true;
}

RTFSprms& RTFValue::getAttributes()
{
    return *m_pAttributes;
}

RTFSprms& RTFValue::getSprms()
{
    return *m_pSprms;
}

}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
