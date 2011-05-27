#include <rtfvalue.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/ustrbuf.hxx>

namespace writerfilter {
namespace rtftok {

using rtl::OString;
using rtl::OUString;

RTFValue::RTFValue(int nValue)
    : m_nValue(nValue)
{
}

int RTFValue::getInt() const
{
    return m_nValue;
}

OUString RTFValue::getString() const
{
    return ::rtl::OUString();
}

uno::Any RTFValue::getAny() const
{
    uno::Any ret;
    ret <<= static_cast<sal_Int32>(m_nValue);
    return ret;
}

writerfilter::Reference<Properties>::Pointer_t RTFValue::getProperties()
{
    return writerfilter::Reference<Properties>::Pointer_t();
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
    return OString::valueOf(static_cast<sal_Int32>(m_nValue)).getStr();
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
