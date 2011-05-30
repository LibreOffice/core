#ifndef _RTFREFERENCEPROPERTIES_HXX_
#define _RTFREFERENCEPROPERTIES_HXX_

#include <map>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <rtfvalue.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        /// Sends RTFSprm instances to DomainMapper.
        class RTFReferenceProperties
            : public writerfilter::Reference<Properties>
        {
            public:
                RTFReferenceProperties(std::map<Id, RTFValue::Pointer_t> rSprms);
                virtual ~RTFReferenceProperties();
                virtual void resolve(Properties & rHandler);
                virtual std::string getType() const;
            private:
                std::map<Id, RTFValue::Pointer_t> m_rSprms;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFREFERENCEPROPERTIES_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
