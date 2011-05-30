#ifndef _RTFSPRM_HXX_
#define _RTFSPRM_HXX_

#include <string>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>

namespace writerfilter {
    namespace rtftok {
        /// RTF keyword with a parameter
        class RTFSprm
            : public Sprm
        {
            public:
                RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue);
                virtual sal_uInt32 getId() const;
                virtual Value::Pointer_t getValue();
                virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
                virtual writerfilter::Reference<Stream>::Pointer_t getStream();
                virtual writerfilter::Reference<Properties>::Pointer_t getProps();
                virtual Kind getKind();
                virtual std::string getName() const;
                virtual std::string toString() const;
            private:
                Id m_nKeyword;
                RTFValue::Pointer_t& m_pValue;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFSPRM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
