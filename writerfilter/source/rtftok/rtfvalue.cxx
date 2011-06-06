#include <rtfvalue.hxx>
#include <rtfreferenceproperties.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

namespace writerfilter {
namespace rtftok {

using rtl::OString;
using rtl::OUString;

RTFValue::RTFValue(int nValue, rtl::OUString sValue, std::multimap<Id, RTFValue::Pointer_t> rAttributes,
        std::multimap<Id, RTFValue::Pointer_t> rSprms)
    : m_nValue(nValue),
    m_sValue(sValue),
    m_rAttributes(rAttributes),
    m_rSprms(rSprms)
{
}

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue),
    m_sValue(),
    m_rAttributes(),
    m_rSprms()
{
}

RTFValue::RTFValue(OUString sValue)
    : m_nValue(),
    m_sValue(sValue),
    m_rAttributes(),
    m_rSprms()
{
}

RTFValue::RTFValue(std::multimap<Id, RTFValue::Pointer_t> rAttributes)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(rAttributes),
    m_rSprms()
{
}

RTFValue::RTFValue(std::multimap<Id, RTFValue::Pointer_t> rAttributes, std::multimap<Id, RTFValue::Pointer_t> rSprms)
    : m_nValue(),
    m_sValue(),
    m_rAttributes(rAttributes),
    m_rSprms(rSprms)
{
}

int RTFValue::getInt() const
{
    return m_nValue;
}

OUString RTFValue::getString() const
{
    return m_sValue;
}

uno::Any RTFValue::getAny() const
{
    uno::Any ret;
    if (m_sValue.getLength() > 0)
        ret <<= m_sValue;
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
    return new RTFValue(m_nValue, m_sValue, m_rAttributes, m_rSprms);
}

std::multimap<Id, RTFValue::Pointer_t>& RTFValue::getAttributes()
{
    return m_rAttributes;
}

std::multimap<Id, RTFValue::Pointer_t>& RTFValue::getSprms()
{
    return m_rSprms;
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
