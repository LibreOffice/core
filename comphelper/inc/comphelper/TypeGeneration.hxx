/*************************************************************************
 *
 *  $RCSfile: TypeGeneration.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-17 13:27:12 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COMPHELPER_TYPEGENERATION_HXX_
#define _COMPHELPER_TYPEGENERATION_HXX_

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#define CPPU_E2T(type)      ((com::sun::star::uno::Type*)type)

namespace com { namespace sun { namespace star { namespace uno {
    class Type;
} } } }
namespace comphelper
{
    enum CppuTypes
    {
        CPPUTYPE_UNKNOWN,           // 0 == unknown == error!!!!

        CPPUTYPE_BOOLEAN,           //getBooleanCppuType()
        CPPUTYPE_INT8,              //getCppuType( (sal_Int8*)0 )
        CPPUTYPE_INT16,             //getCppuType( (sal_Int16*)0 )
        CPPUTYPE_INT32,             //getCppuType( (sal_Int32*)0 )
        CPPUTYPE_DOUBLE,            //getCppuType( (double*)0 )
        CPPUTYPE_FLOAT,             //getCppuType( (float*)0 )
        CPPUTYPE_OUSTRING,          //getCppuType( (OUString*)0 )

        CPPUTYPE_FONTSLANT,         //getCppuType( (FontSlant*)0 )
        CPPUTYPE_LOCALE,            //getCppuType( (Locale*)0 )
        CPPUTYPE_PROPERTYVALUE,     //getCppuType( (Sequence<PropertyValue>*)0 )
        CPPUTYPE_PROPERTYVALUES,    //getCppuType( (Sequence<PropertyValues>*)0 )
        CPPUTYPE_BORDERLINE,        //getCppuType( (table::BorderLine*)0 )
        CPPUTYPE_BREAK,             //getCppuType( (style::BreakType*)0 )
        CPPUTYPE_GRAPHICLOC,        //getCppuType( (style::GraphicLocation*)0 )
        CPPUTYPE_DROPCAPFMT,        //getCppuType( (style::DropCapFormat*)0 )
        CPPUTYPE_LINESPACE,         //getCppuType( (style::LineSpacing*)0 )
        CPPUTYPE_AWTSIZE,           //getCppuType( (awt::Size*)0 )
        CPPUTYPE_SHADOWFMT,         //getCppuType( (table::ShadowFormat*)0 )
        CPPUTYPE_TBLCOLSEP,         //getCppuType( (Sequence<text::TableColumnSeparator>*)0 )
        CPPUTYPE_PNTSEQSEQ,         //getCppuType( (PointSequenceSequence*)0 )
        CPPUTYPE_DOCIDXMRK,         //getCppuType( (Sequence< Reference< XDocumentIndexMark > >*)0 )
        CPPUTYPE_SEQINT8,           //getCppuType( (Sequence<sal_Int8>*)0 )
        CPPUTYPE_SEQTABSTOP,        //getCppuType( (Sequence<style::TabStop>*)0 )
        CPPUTYPE_SEQANCHORTYPE,     //getCppuType( (Sequence<text::TextContentAnchorType>*)0 )
        CPPUTYPE_SEQDEPTXTFLD,      //getCppuType( (Sequence<Reference<XDependentTextField> >*)0 )
        CPPUTYPE_TXTCNTANCHOR,      //getCppuType( (text::TextContentAnchorType*)0 )
        CPPUTYPE_WRAPTXTMODE,       //getCppuType( (text::WrapTextMode*)0 )
        CPPUTYPE_COLORMODE,         //getCppuType( (drawing::ColorMode*)0 )
        CPPUTYPE_PAGESTYLELAY,      //getCppuType( (style::PageStyleLayout*)0 )
        CPPUTYPE_VERTALIGN,         //getCppuType( (style::VerticalAlignment*)0 )
        CPPUTYPE_TABLEBORDER,       //getCppuType( (table::TableBorder*)0 )
        CPPUTYPE_GRFCROP,           //getCppuType( (text::GraphicCrop*)0 )
        CPPUTYPE_SECTFILELNK,       //getCppuType( (text::SectionFileLink*)0 )
        CPPUTYPE_PAGENUMTYPE,       //getCppuType( (const PageNumberType*)0 )
        CPPUTYPE_DATETIME,          //getCppuType( (util::DateTime*)0 )
        CPPUTYPE_DATE,              //getCppuType( (util::Date*)0 )

        CPPUTYPE_REFINTERFACE,      //getCppuType( (Reference<XInterface>*)0 )
        CPPUTYPE_REFIDXREPL,        //getCppuType( (Reference<container::XIndexReplace>*)0 )
        CPPUTYPE_REFNAMECNT,        //getCppuType( (Reference<container::XNameContainer>*)0 )
        CPPUTYPE_REFTEXTFRAME,      //getCppuType( (Reference<text::XTextFrame>*)0 )
        CPPUTYPE_REFTEXTSECTION,    //getCppuType( (Reference<text::XTextSection>*)0 )
        CPPUTYPE_REFFOOTNOTE,       //getCppuType( (Reference<text::XFootnote>*)0 )
        CPPUTYPE_REFTEXT,           //getCppuType( (Reference<text::XText>*)0 )
        CPPUTYPE_REFTEXTCOL,        //getCppuType( (Reference<text::XTextColumns>*)0 )

        CPPUTYPE_REFFORBCHARS,      //getCppuType( (Reference<XForbiddenCharacters>*)0)
        CPPUTYPE_REFIDXCNTNR,       //getCppuType( (Reference<XIndexContainer>*)0)
        CPPUTYPE_REFTEXTCNTNT,      //getCppuType( (Reference<XTextContent>*)0)
        CPPUTYPE_REFBITMAP,         //getCppuType( (Reference<awt::XBitmap>*)0)
        CPPUTYPE_REFNMREPLACE,      //getCppuType( (Reference<container::XNameReplace>*)0)
        CPPUTYPE_REFCELL,           //getCppuType( (Reference<table::XCell>*)0)
        CPPUTYPE_REFDOCINDEX,       //getCppuType( (Reference<text::XDocumentIndex>*)0)
        CPPUTYPE_REFDOCIDXMRK,      //getCppuType( (Reference<text::XDocumentIndexMark>*)0)
        CPPUTYPE_REFTXTFIELD,       //getCppuType( (Reference<text::XTextField>*)0)
        CPPUTYPE_REFTXTRANGE,       //getCppuType( (Reference<text::XTextRange>*)0)
        CPPUTYPE_REFTXTTABLE,       //getCppuType( (Reference<text::XTextTable>*)0)
        CPPUTYPE_AWTPOINT,          //getCppuType( (awt::Point*)0 )
        CPPUTYPE_REFLIBCONTAINER,   //getCppuType( (Reference< script::XLibraryContainer >*)0)
        CPPUTYPE_SEQANY,            //getCppuType( (Sequence< uno::Any >*)0)
        CPPUTYPE_REFRESULTSET,      //getCppuType( (Reference< sdbc::XResultSet >*)0)
        CPPUTYPE_REFCONNECTION,     //getCppuType( (Reference< sdbc::XConnection >*)0)
        CPPUTYPE_REFMODEL,          //getCppuType( (Reference< frame::XModel >*)0)

        CPPUTYPE_OUSTRINGS,         //getCppuType( (Sequence<OUString>*)0 )
        CPPUTYPE_REFCOMPONENT,      //getCppuType( (Reference< lang::XComponent >*)0 )
        CPPUTYPE_END
    };
    void GenerateCppuType ( sal_uInt16 eType, const com::sun::star::uno::Type*& pType );
}
#endif
