/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <rtfreferenceproperties.hxx>

namespace writerfilter {
namespace rtftok {


RTFValue::RTFValue(int nValue, OUString sValue, RTFSprms rAttributes,
        RTFSprms rSprms, uno::Reference<drawing::XShape> xShape,
        uno::Reference<io::XInputStream> xStream, uno::Reference<embed::XEmbeddedObject> xObject, bool bForceString)
    : m_nValue(nValue),
    m_sValue(sValue),
    m_xShape(xShape),
    m_xStream(xStream),
    m_xObject(xObject),
    m_bForceString(bForceString)
{
    m_pAttributes.reset(new RTFSprms(rAttributes));
    m_pSprms.reset(new RTFSprms(rSprms));
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
        return OUString::valueOf(sal_Int32(m_nValue));
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
    return new RTFValue(m_nValue, m_sValue, *m_pAttributes, *m_pSprms, m_xShape, m_xStream, m_xObject, m_bForceString);
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
