#include <rtfreferenceproperties.hxx>
#include <rtfsprm.hxx>

namespace writerfilter {
namespace rtftok {

RTFReferenceProperties::RTFReferenceProperties(std::map<Id, int> rSprms)
    : m_rSprms(rSprms)
{
}

RTFReferenceProperties::~RTFReferenceProperties()
{
}

void RTFReferenceProperties::resolve(Properties& rHandler)
{
    for (std::map<Id, int>::const_iterator i = m_rSprms.begin(); i != m_rSprms.end(); ++i)
    {
        RTFValue::Pointer_t pValue(new RTFValue(i->second));
        RTFSprm aSprm(i->first, pValue);
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
