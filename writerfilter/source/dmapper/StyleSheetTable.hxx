#ifndef INCLUDED_STYLESHEETTABLE_HXX
#define INCLUDED_STYLESHEETTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_WW8_RESOURCE_MODEL_HXX
#include <doctok/WW8ResourceModel.hxx>
#endif

namespace com{ namespace sun { namespace star { namespace text{
    class XTextDocument;
}}}}
namespace dmapper
{
enum StyleType
{
    STYLE_TYPE_UNKNOWN,
    STYLE_TYPE_PARA,
    STYLE_TYPE_CHAR,
    STYLE_TYPE_TABLE,
    STYLE_LIST
};

struct StyleSheetTable_Impl;
struct StyleSheetEntry
{
    sal_Int32       nStyleIdentifierI;
    sal_Int32       nStyleIdentifierD;
    bool            bInvalidHeight;
    bool            bHasUPE; //universal property expansion
    StyleType       nStyleTypeCode; //sgc
    sal_Int32       nBaseStyleIdentifier;
    sal_Int32       nNextStyleIdentifier;
    ::rtl::OUString sStyleName;
    ::rtl::OUString sStyleName1;
    PropertyMapPtr  pProperties;
    sal_Int16       nPropertyCalls; //determines paragraph (2)/character style(1)
    StyleSheetEntry();
};
class DomainMapper;
class WRITERFILTER_DLLPRIVATE StyleSheetTable :
        public doctok::Properties,
        public doctok::Table
{
    StyleSheetTable_Impl   *m_pImpl;

public:
    StyleSheetTable( DomainMapper& rDMapper );
    virtual ~StyleSheetTable();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

    void ApplyStyleSheets(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument> xTextDocument);
    const StyleSheetEntry* FindStyleSheetByISTD(sal_Int32 nIndex);
    // returns the parent of the one with the given name - if empty the parent of the current style sheet is returned
    const StyleSheetEntry* FindParentStyleSheet(sal_Int32 nBaseStyleIdentifier);
};
}

#endif //
