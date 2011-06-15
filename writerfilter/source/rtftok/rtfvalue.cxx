#include <rtfvalue.hxx>
#include <rtfreferenceproperties.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

namespace writerfilter {
namespace rtftok {

using rtl::OString;
using rtl::OUString;

RTFValue::RTFValue(int nValue, rtl::OUString sValue, std::vector<std::pair<Id, RTFValue::Pointer_t>> rAttributes,
        std::vector<std::pair<Id, RTFValue::Pointer_t>> rSprms, uno::Reference<drawing::XShape> rShape)
    : m_nValue(nValue),
    m_sValue(sValue),
    m_rAttributes(rAttributes),
    m_rSprms(rSprms),
    m_rShape(rShape)
{
}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue),
    m_sValue(),
    m_rAttributes(),
    m_rSprms(),
    m_rShape()
{
}

RTFValue::RTFValue(OUString sValue)
    : m_nValue(),
    m_sValue(sValue),
    m_rAttributes(),
    m_rSprms(),
    m_rShape()
{
}

RTFValue::RTFValue(std::vector<std::pair<Id, RTFValue::Pointer_t>> rAttributes)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(rAttributes),
    m_rSprms(),
    m_rShape()
{
}

RTFValue::RTFValue(std::vector<std::pair<Id, RTFValue::Pointer_t>> rAttributes, std::vector<std::pair<Id, RTFValue::Pointer_t>> rSprms)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(rAttributes),
    m_rSprms(rSprms),
    m_rShape()
{
}

RTFValue::RTFValue(uno::Reference<drawing::XShape> rShape)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(),
    m_rSprms(),
    m_rShape(rShape)
{
}

int RTFValue::getInt() const
{
    return m_nValue;
}

OUString RTFValue::getString() const
{
    if (m_sValue.getLength() > 0)
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
    if (m_sValue.getLength() > 0)
        ret <<= m_sValue;
    else if (m_rShape.is())
        ret <<= m_rShape;
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
    if (m_sValue.getLength() > 0)
        return OUStringToOString(m_sValue, RTL_TEXTENCODING_UTF8).getStr();
    else
        return OString::valueOf(static_cast<sal_Int32>(m_nValue)).getStr();
}

RTFValue* RTFValue::Clone()
{
    return new RTFValue(m_nValue, m_sValue, m_rAttributes, m_rSprms, m_rShape);
}

std::vector<std::pair<Id, RTFValue::Pointer_t>>& RTFValue::getAttributes()
{
    return m_rAttributes;
}

std::vector<std::pair<Id, RTFValue::Pointer_t>>& RTFValue::getSprms()
{
    return m_rSprms;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
