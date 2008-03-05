/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DomainMapper.hxx,v $
 *
 *  $Revision: 1.21 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 16:47:00 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INCLUDED_DOMAINMAPPER_HXX
#define INCLUDED_DOMAINMAPPER_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/style/TabAlign.hpp>

#include <map>
#include <vector>

namespace com{ namespace sun {namespace star{
    namespace beans{
        struct PropertyValue;
    }
    namespace uno{
        class XComponentContext;
    }
    namespace lang{
        class XMultiServiceFactory;
    }
    namespace text{
        class XTextRange;
    }
}}}

typedef std::vector< com::sun::star::beans::PropertyValue > PropertyValueVector_t;

namespace writerfilter {
namespace dmapper
{
using namespace std;

class PropertyMap;
class DomainMapper_Impl;

// different context types require different sprm handling (e.g. names)
enum SprmType
{
    SPRM_DEFAULT,
    SPRM_LIST
};
enum SourceDocumentType
{
    DOCUMENT_DOC,
    DOCUMENT_OOXML,
    DOCUMENT_RTF
};
class WRITERFILTER_DLLPUBLIC DomainMapper : public Properties, public Table,
                    public BinaryObj, public Stream
{
    DomainMapper_Impl   *m_pImpl;

public:
    DomainMapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xModel,
                                SourceDocumentType eDocumentType );
    virtual ~DomainMapper();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len,
                      writerfilter::Reference<Properties>::Pointer_t ref);

    // Stream
    virtual void startSectionGroup();
    virtual void endSectionGroup();
    virtual void startParagraphGroup();
    virtual void endParagraphGroup();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void table(Id name,
                       writerfilter::Reference<Table>::Pointer_t ref);
    virtual void substream(Id name,
                           ::writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & info);

    void sprm( Sprm& sprm, ::boost::shared_ptr<PropertyMap> pContext, SprmType = SPRM_DEFAULT );

    void PushStyleSheetProperties( ::boost::shared_ptr<PropertyMap> pStyleProperties );
    void PopStyleSheetProperties();

    void PushListProperties( ::boost::shared_ptr<PropertyMap> pListProperties );
    void PopListProperties();

    bool IsOOXMLImport() const;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > GetTextFactory() const;
    void  AddListIDToLFOTable( sal_Int32 nAbstractNumId );
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > GetCurrentTextRange();

    ::rtl::OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties );

private:
    void handleUnderlineType(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext);
    void handleParaJustification(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext, const bool bExchangeLeftRight);
    bool getColorFromIndex(const sal_Int32 nIndex, sal_Int32 &nColor);
    sal_Int16 getEmphasisValue(const sal_Int32 nIntValue);
    rtl::OUString getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix = true);
    com::sun::star::style::TabAlign getTabAlignFromValue(const sal_Int32 nIntValue);
    sal_Unicode getFillCharFromValue(const sal_Int32 nIntValue);
    void resolveAttributeProperties(Value & val);
    void resolveSprmProps(Sprm & sprm_);
    sal_Int32 mnBackgroundColor;
    bool mbIsHighlightSet;
};

} // namespace dmapper
} // namespace writerfilter
#endif //
