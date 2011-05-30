#ifndef _RTFREFERENCETABLE_HXX_
#define _RTFREFERENCETABLE_HXX_

#include <map>
#include <resourcemodel/WW8ResourceModel.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        class RTFReferenceTable
            : public writerfilter::Reference<Table>
        {
            public:
                typedef std::map<int, writerfilter::Reference<Properties>::Pointer_t> Entries_t;
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
