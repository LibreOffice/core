#ifndef _RTFREFERENCETABLE_HXX_
#define _RTFREFERENCETABLE_HXX_

#include <map>
#include <resourcemodel/WW8ResourceModel.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        /// Sends tables (e.g. font table) to the domain mapper.
        class RTFReferenceTable
            : public writerfilter::Reference<Table>
        {
            public:
                typedef std::multimap<int, writerfilter::Reference<Properties>::Pointer_t> Entries_t;
                typedef std::pair<int, writerfilter::Reference<Properties>::Pointer_t> Entry_t;
                RTFReferenceTable(Entries_t const& rEntries);
                virtual ~RTFReferenceTable();
                virtual void resolve(Table & rHandler);
                virtual std::string getType() const;
            private:
                Entries_t m_aEntries;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFREFERENCETABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
