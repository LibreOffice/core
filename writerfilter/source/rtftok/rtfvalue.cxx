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

#include <rtfvalue.hxx>
#include <rtfreferenceproperties.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

namespace writerfilter {
namespace rtftok {

using rtl::OString;
using rtl::OUString;

RTFValue::RTFValue(int nValue, rtl::OUString sValue, RTFSprms_t rAttributes,
        RTFSprms_t rSprms, uno::Reference<drawing::XShape> rShape,
        uno::Reference<io::XInputStream> rStream)
    : m_nValue(nValue),
    m_sValue(sValue),
    m_rAttributes(rAttributes),
    m_rSprms(rSprms),
    m_rShape(rShape),
    m_rStream(rStream),
    m_bForceString(false)
{
}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue),
    m_sValue(),
    m_rAttributes(),
    m_rSprms(),
    m_rShape(),
    m_rStream(),
    m_bForceString(false)
{
}

RTFValue::RTFValue(OUString sValue, bool bForce)
    : m_nValue(),
    m_sValue(sValue),
    m_rAttributes(),
    m_rSprms(),
    m_rShape(),
    m_rStream(),
    m_bForceString(bForce)
{
}

RTFValue::RTFValue(RTFSprms_t rAttributes)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(rAttributes),
    m_rSprms(),
    m_rShape(),
    m_rStream(),
    m_bForceString(false)
{
}

RTFValue::RTFValue(RTFSprms_t rAttributes, RTFSprms_t rSprms)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(rAttributes),
    m_rSprms(rSprms),
    m_rShape(),
    m_rStream(),
    m_bForceString(false)
{
}

RTFValue::RTFValue(uno::Reference<drawing::XShape> rShape)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(),
    m_rSprms(),
    m_rShape(rShape),
    m_rStream(),
    m_bForceString(false)
{
}

RTFValue::RTFValue(uno::Reference<io::XInputStream> rStream)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(),
    m_rSprms(),
    m_rShape(),
    m_rStream(rStream),
    m_bForceString(false)
{
}

int RTFValue::getInt() const
{
    return m_nValue;
}

OUString RTFValue::getString() const
{
    if (m_sValue.getLength() > 0 || m_bForceString)
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
    if (m_sValue.getLength() > 0 || m_bForceString)
        ret <<= m_sValue;
    else if (m_rShape.is())
        ret <<= m_rShape;
    else if (m_rStream.is())
        ret <<= m_rStream;
    else
        ret <<= static_cast<sal_Int32>(m_nValue);
    return ret;
}

writerfilter::Reference<Properties>::Pointer_t RTFValue::getProperties()
{
    writerfilter::Reference<Properties>::Pointer_t const pProperties(
            new RTFReferenceProperties(m_rAttributes, m_rSprms)
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
    if (m_sValue.getLength() > 0 || m_bForceString)
        return OUStringToOString(m_sValue, RTL_TEXTENCODING_UTF8).getStr();
    else
        return OString::valueOf(static_cast<sal_Int32>(m_nValue)).getStr();
}

RTFValue* RTFValue::Clone()
{
    return new RTFValue(m_nValue, m_sValue, RTFSprm::Clone(m_rAttributes), RTFSprm::Clone(m_rSprms), m_rShape, m_rStream);
}

RTFSprms_t& RTFValue::getAttributes()
{
    return m_rAttributes;
}

RTFSprms_t& RTFValue::getSprms()
{
    return m_rSprms;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
