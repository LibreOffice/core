#ifndef INCLUDED_LISTTABLE_HXX
#define INCLUDED_LISTTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#include <doctok/WW8ResourceModel.hxx>

namespace com{ namespace sun { namespace star {
    namespace text{
        class XTextDocument;
    }
    namespace container{
        class XIndexReplace;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
}}}
namespace dmapper
{
class DomainMapper;
struct ListTable_Impl;
class WRITERFILTER_DLLPRIVATE ListTable :
        public doctok::Properties,
        public doctok::Table
{
    ListTable_Impl   *m_pImpl;

public:
    ListTable(
            DomainMapper& rDMapper,
            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xFactory);
    virtual ~ListTable();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

    // BinaryObj
//    virtual void data(const sal_Int8* buf, size_t len,
//                      doctok::Reference<Properties>::Pointer_t ref);

    sal_uInt32          size();
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexReplace >
            GetNumberingRules(sal_Int32 nListId);
};
typedef boost::shared_ptr< ListTable >          ListTablePtr;
}

#endif //
