/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_DOMAINMAPPER_HXX
#define INCLUDED_DOMAINMAPPER_HXX

#include <WriterFilterDllApi.hxx>
#include <resourcemodel/LoggedResources.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/text/FontEmphasis.hpp>
#include <com/sun/star/style/TabAlign.hpp>

#include <map>
#include <vector>

namespace com{ namespace sun {namespace star{
    namespace beans{
        struct PropertyValue;
    }
    namespace io{
        class XInputStream;
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

class PropertyMap;
class DomainMapper_Impl;
class ListsManager;
class StyleSheetTable;

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
class WRITERFILTER_DLLPUBLIC DomainMapper : public LoggedProperties, public LoggedTable,
                    public BinaryObj, public LoggedStream
{
    DomainMapper_Impl   *m_pImpl;

public:
    DomainMapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                                ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
                                ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > xModel,
                                SourceDocumentType eDocumentType );
    virtual ~DomainMapper();

    // Stream
    virtual void markLastParagraphInSection();

    // BinaryObj
    virtual void data(const sal_uInt8* buf, size_t len,
                      writerfilter::Reference<Properties>::Pointer_t ref);

    void sprmWithProps( Sprm& sprm, ::boost::shared_ptr<PropertyMap> pContext, SprmType = SPRM_DEFAULT );

    void PushStyleSheetProperties( ::boost::shared_ptr<PropertyMap> pStyleProperties, bool bAffectTableMngr = false );
    void PopStyleSheetProperties( bool bAffectTableMngr = false );

    void PushListProperties( ::boost::shared_ptr<PropertyMap> pListProperties );
    void PopListProperties();

    bool IsOOXMLImport() const;
    ::com::sun::star::uno::Reference < ::com::sun::star::lang::XMultiServiceFactory > GetTextFactory() const;
    void  AddListIDToLFOTable( sal_Int32 nAbstractNumId );
    ::com::sun::star::uno::Reference< ::com::sun::star::text::XTextRange > GetCurrentTextRange();

    ::rtl::OUString getOrCreateCharStyle( PropertyValueVector_t& rCharProperties );
    boost::shared_ptr< ListsManager > GetListTable( );
    boost::shared_ptr< StyleSheetTable > GetStyleSheetTable( );

private:
    // Stream
    virtual void lcl_startSectionGroup();
    virtual void lcl_endSectionGroup();
    virtual void lcl_startParagraphGroup();
    virtual void lcl_endParagraphGroup();
    virtual void lcl_startCharacterGroup();
    virtual void lcl_endCharacterGroup();
    virtual void lcl_startShape( ::com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape );
    virtual void lcl_endShape( );

    virtual void lcl_text(const sal_uInt8 * data, size_t len);
    virtual void lcl_utext(const sal_uInt8 * data, size_t len);
    virtual void lcl_props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void lcl_table(Id name,
                           writerfilter::Reference<Table>::Pointer_t ref);
    virtual void lcl_substream(Id name,
                               ::writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void lcl_info(const string & info);

    // Properties
    virtual void lcl_attribute(Id Name, Value & val);
    virtual void lcl_sprm(Sprm & sprm);

    // Table
    virtual void lcl_entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    void handleUnderlineType(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext);
    void handleParaJustification(const sal_Int32 nIntValue, const ::boost::shared_ptr<PropertyMap> pContext, const bool bExchangeLeftRight);
    bool getColorFromIndex(const sal_Int32 nIndex, sal_Int32 &nColor);
    sal_Int16 getEmphasisValue(const sal_Int32 nIntValue);
    rtl::OUString getBracketStringFromEnum(const sal_Int32 nIntValue, const bool bIsPrefix = true);
    com::sun::star::style::TabAlign getTabAlignFromValue(const sal_Int32 nIntValue);
    sal_Unicode getFillCharFromValue(const sal_Int32 nIntValue);
    sal_Int32 mnBackgroundColor;
    bool mbIsHighlightSet;
};

} // namespace dmapper
} // namespace writerfilter
#endif //
