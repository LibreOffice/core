#ifndef INCLUDED_DOMAINMAPPER_HXX
#define INCLUDED_DOMAINMAPPER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <doctok/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace dmapper
{
using namespace std;

class PropertyMap;
class FontTable;
class ListTable;

class DomainMapper_Impl;

// different context types require different sprm handling (e.g. names)
enum SprmType
{
    SPRM_DEFAULT,
    SPRM_LIST
};
class WRITERFILTER_DLLPUBLIC DomainMapper : public doctok::Properties, public doctok::Table,
                    public doctok::BinaryObj, public doctok::Stream
{
    DomainMapper_Impl   *m_pImpl;

public:
    DomainMapper(::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xModel);
    virtual ~DomainMapper();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len,
                      doctok::Reference<Properties>::Pointer_t ref);

    // Stream
    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(doctok::Reference<Properties>::Pointer_t ref);
    virtual void table(doctok::Id name,
                       doctok::Reference<Table>::Pointer_t ref);
    virtual void substream(doctok::Id name,
                           ::doctok::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & info);

    void sprm( doctok::Sprm& sprm, ::boost::shared_ptr<PropertyMap> pContext, SprmType = SPRM_DEFAULT );
};
}

#endif //
