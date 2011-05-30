#include <rtfreferencetable.hxx>

namespace writerfilter {
namespace rtftok {

RTFReferenceTable::RTFReferenceTable(Entries_t const& rEntries)
    : m_aEntries(rEntries)
{
}

RTFReferenceTable::~RTFReferenceTable()
{
}

void RTFReferenceTable::resolve(Table& rHandler)
{
    for (Entries_t::const_iterator i = m_aEntries.begin(); i != m_aEntries.end(); ++i)
        rHandler.entry(i->first, i->second);
}

std::string RTFReferenceTable::getType() const
{
    return "RTFReferenceTable";
}

} // namespace rtftok
} // namespace writerfilter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
