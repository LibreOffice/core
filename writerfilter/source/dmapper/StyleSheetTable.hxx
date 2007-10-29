/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: StyleSheetTable.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-29 15:27:25 $
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
#ifndef INCLUDED_STYLESHEETTABLE_HXX
#define INCLUDED_STYLESHEETTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <com/sun/star/lang/XComponent.hpp>
#ifndef INCLUDED_DMAPPER_PROPERTYMAP_HXX
#include <PropertyMap.hxx>
#endif
#ifndef INCLUDED_FONTTABLE_HXX
#include <FontTable.hxx>
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
    ::rtl::OUString sStyleIdentifierI;
    ::rtl::OUString sStyleIdentifierD;
    bool            bIsDefaultStyle;
    bool            bInvalidHeight;
    bool            bHasUPE; //universal property expansion
    StyleType       nStyleTypeCode; //sgc
    ::rtl::OUString sBaseStyleIdentifier;
    ::rtl::OUString sNextStyleIdentifier;
    ::rtl::OUString sStyleName;
    ::rtl::OUString sStyleName1;
    PropertyMapPtr  pProperties;
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

    void ApplyStyleSheets(::com::sun::star::uno::Reference< ::com::sun::star::text::XTextDocument> xTextDocument, FontTablePtr rFontTable);
    const StyleSheetEntry* FindStyleSheetByISTD(const ::rtl::OUString sIndex);
    // returns the parent of the one with the given name - if empty the parent of the current style sheet is returned
    const StyleSheetEntry* FindParentStyleSheet(::rtl::OUString sBaseStyle);

    ::rtl::OUString ConvertStyleName( const ::rtl::OUString& rWWName/*, bool bParagraphStyle*/ );
    ::rtl::OUString GetStyleIdFromIndex(const sal_uInt32 sti);
private:
    void resolveAttributeProperties(doctok::Value & val);
    void resolveSprmProps(doctok::Sprm & sprm_);
};
}

#endif //
