/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
class WRITERFILTER_DLLPUBLIC DomainMapper : public Properties, public Table,
                    public BinaryObj, public Stream
{
    DomainMapper_Impl   *m_pImpl;

public:
    DomainMapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& xContext,
                                ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > xInputStream,
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
    virtual void markLastParagraphInSection();
    virtual void startCharacterGroup();
    virtual void endCharacterGroup();
    virtual void startShape( ::com::sun::star::uno::Reference< com::sun::star::drawing::XShape > xShape );
    virtual void endShape( );

    virtual void text(const sal_uInt8 * data, size_t len);
    virtual void utext(const sal_uInt8 * data, size_t len);
    virtual void props(writerfilter::Reference<Properties>::Pointer_t ref);
    virtual void table(Id name,
                       writerfilter::Reference<Table>::Pointer_t ref);
    virtual void substream(Id name,
                           ::writerfilter::Reference<Stream>::Pointer_t ref);
    virtual void info(const string & info);

    void sprm( Sprm& sprm, ::boost::shared_ptr<PropertyMap> pContext, SprmType = SPRM_DEFAULT );

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
