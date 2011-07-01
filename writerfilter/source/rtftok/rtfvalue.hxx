#ifndef _RTFVALUE_HXX_
#define _RTFVALUE_HXX_

#include <string>
#include <map>
#include <rtl/ustrbuf.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>

namespace writerfilter {
    namespace rtftok {
        /// Value of an RTF keyword
        class RTFValue
            : public Value
        {
            public:
                typedef ::boost::shared_ptr<RTFValue> Pointer_t;
                RTFValue(int nValue, rtl::OUString sValue, std::vector<std::pair<Id, RTFValue::Pointer_t>> rAttributes,
                        std::vector<std::pair<Id, RTFValue::Pointer_t>> rSprms, uno::Reference<drawing::XShape> rShape);
                RTFValue(int nValue);
                RTFValue(rtl::OUString sValue, bool bForce = false);
                RTFValue(std::vector<std::pair<Id, RTFValue::Pointer_t>> rAttributes);
                RTFValue(std::vector<std::pair<Id, RTFValue::Pointer_t>> rAttributes,
                        std::vector<std::pair<Id, RTFValue::Pointer_t>> rSprms);
                RTFValue(uno::Reference<drawing::XShape> rShape);
                void setString(rtl::OUString sValue);
                virtual int getInt() const;
                virtual rtl::OUString getString() const;
                virtual uno::Any getAny() const;
                virtual writerfilter::Reference<Properties>::Pointer_t getProperties();
                virtual writerfilter::Reference<Stream>::Pointer_t getStream();
                virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
                virtual std::string toString() const;
                virtual RTFValue* Clone();
                std::vector<std::pair<Id, RTFValue::Pointer_t>>& getAttributes();
                std::vector<std::pair<Id, RTFValue::Pointer_t>>& getSprms();
            private:
                int m_nValue;
                rtl::OUString m_sValue;
                std::vector<std::pair<Id, RTFValue::Pointer_t>> m_rAttributes;
                std::vector<std::pair<Id, RTFValue::Pointer_t>> m_rSprms;
                uno::Reference<drawing::XShape> m_rShape;
                bool m_bForceString;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFVALUE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
