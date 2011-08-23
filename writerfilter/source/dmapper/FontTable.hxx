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

#ifndef INCLUDED_FONTTABLE_HXX
#define INCLUDED_FONTTABLE_HXX

#include <boost/shared_ptr.hpp>
#include <WriterFilterDllApi.hxx>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace writerfilter {
namespace dmapper
{

struct FontTable_Impl;
struct FontEntry
{
    typedef boost::shared_ptr<FontEntry> Pointer_t;
    
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
class WRITERFILTER_DLLPRIVATE FontTable : public Properties, public Table
                    /*,public BinaryObj*/, public Stream
{
    FontTable_Impl   *m_pImpl;

public:
    FontTable();
    virtual ~FontTable();

    // Properties
    virtual void attribute(Id Name, Value & val);
    virtual void sprm(Sprm & sprm);

    // Table
    virtual void entry(int pos, writerfilter::Reference<Properties>::Pointer_t ref);

    // BinaryObj
//    virtual void data(const sal_Int8* buf, size_t len,
//                      writerfilter::Reference<Properties>::Pointer_t ref);

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
    virtual void startShape( ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape > xShape );
    virtual void endShape( );

    const FontEntry::Pointer_t  getFontEntry(sal_uInt32 nIndex);
    sal_uInt32          size();
};
typedef boost::shared_ptr< FontTable >          FontTablePtr;
}}

#endif //
