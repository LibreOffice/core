#ifndef INCLUDED_FONTTABLE_HXX
#define INCLUDED_FONTTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <doctok/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace dmapper
{
using namespace std;

struct FontTable_Impl;
struct FontEntry
{
    ::rtl::OUString sFontName;
    ::rtl::OUString sFontName1;
    bool            bTrueType;
    sal_Int16       nPitchRequest;
    sal_Int32       nTextEncoding;
    sal_Int32       nFontFamilyId;
    sal_Int32       nBaseWeight;
    sal_Int32       nAltFontIndex;
    ::rtl::OUString sPanose;
    ::rtl::OUString sFontSignature;
    ::rtl::OUString sAlternativeFont;
    FontEntry() :
        bTrueType(false),
        nPitchRequest( 0 ),
        nTextEncoding( 0 ),
        nFontFamilyId( 0 ),
        nBaseWeight( 0 ),
        nAltFontIndex( 0 )
        {}
};
class WRITERFILTER_DLLPRIVATE FontTable : public doctok::Properties, public doctok::Table
                    /*,public doctok::BinaryObj*/, public doctok::Stream
{
    FontTable_Impl   *m_pImpl;

public:
    FontTable();
    virtual ~FontTable();

    // Properties
    virtual void attribute(doctok::Id Name, doctok::Value & val);
    virtual void sprm(doctok::Sprm & sprm);

    // Table
    virtual void entry(int pos, doctok::Reference<Properties>::Pointer_t ref);

    // BinaryObj
//    virtual void data(const sal_Int8* buf, size_t len,
//                      doctok::Reference<Properties>::Pointer_t ref);

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

    const FontEntry*    getFontEntry(sal_uInt32 nIndex);
    sal_uInt32          size();
};
typedef boost::shared_ptr< FontTable >          FontTablePtr;
}

#endif //
