#include <rtfreferenceproperties.hxx>
#include <rtfsprm.hxx>

namespace writerfilter {
namespace rtftok {

RTFReferenceProperties::RTFReferenceProperties(std::multimap<Id, RTFValue::Pointer_t> rAttributes, std::multimap<Id, RTFValue::Pointer_t> rSprms)
    : m_rAttributes(rAttributes),
    m_rSprms(rSprms)
{
}

RTFReferenceProperties::~RTFReferenceProperties()
{
}

void RTFReferenceProperties::resolve(Properties& rHandler)
{
    for (std::multimap<Id, RTFValue::Pointer_t>::iterator i = m_rAttributes.begin(); i != m_rAttributes.end(); ++i)
        rHandler.attribute(i->first, *i->second.get());
    for (std::multimap<Id, RTFValue::Pointer_t>::iterator i = m_rSprms.begin(); i != m_rSprms.end(); ++i)
    {
        RTFSprm aSprm(i->first, i->second);
        rHandler.sprm(aSprm);
    }
}

std::string RTFReferenceProperties::getType() const
{
    return "RTFReferenceProperties";
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
