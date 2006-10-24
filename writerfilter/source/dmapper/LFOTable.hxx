#ifndef INCLUDED_LFOTABLE_HXX
#define INCLUDED_LFOTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <doctok/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>
//#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
//#include <PropertyMap.hxx>
//#endif
//#include <vector>

namespace dmapper
{

struct LFOTable_Impl;
class WRITERFILTER_DLLPRIVATE LFOTable : public doctok::Properties, public doctok::Table
{
    LFOTable_Impl   *m_pImpl;
public:
    LFOTable();
    virtual ~LFOTable();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

//    sal_uInt32          size();
    sal_Int32       GetListID(sal_uInt32 nLFO);
};
typedef boost::shared_ptr< LFOTable >          LFOTablePtr;
}

#endif //
