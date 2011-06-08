#include <rtfsprm.hxx>
#include <rtl/strbuf.hxx>

#include <resourcemodel/QNameToString.hxx>

using rtl::OStringBuffer;

namespace writerfilter {
namespace rtftok {

RTFSprm::RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue)
    : m_nKeyword(nKeyword),
    m_pValue(pValue)
{
}

sal_uInt32 RTFSprm::getId() const
{
    return m_nKeyword;
}

Value::Pointer_t RTFSprm::getValue()
{
    return Value::Pointer_t(m_pValue->Clone());
}

writerfilter::Reference<BinaryObj>::Pointer_t RTFSprm::getBinary()
{
    return m_pValue->getBinary();
}

writerfilter::Reference<Stream>::Pointer_t RTFSprm::getStream()
{
    return m_pValue->getStream();
}

writerfilter::Reference<Properties>::Pointer_t RTFSprm::getProps()
{
    return m_pValue->getProperties();
}

Sprm::Kind RTFSprm::getKind()
{
    return Sprm::UNKNOWN;
}

std::string RTFSprm::getName() const
{
    return "RTFSprm";
}

std::string RTFSprm::toString() const
{
    OStringBuffer aBuf("RTFSprm");

    std::string sResult = (*QNameToString::Instance())(m_nKeyword);
    if (sResult.length() == 0)
        sResult = (*SprmIdToString::Instance())(m_nKeyword);

    aBuf.append(" ('");
    if (sResult.length() == 0)
        aBuf.append(sal_Int32(m_nKeyword));
    else
        aBuf.append(sResult.c_str());
    aBuf.append("', '");
    aBuf.append(m_pValue->toString().c_str());
    aBuf.append("')");

    return aBuf.makeStringAndClear().getStr();
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
