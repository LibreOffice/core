/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: FontTable.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-10 11:38:45 $
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

#ifndef INCLUDED_FONTTABLE_HXX
#define INCLUDED_FONTTABLE_HXX

#ifndef INCLUDED_WRITERFILTERDLLAPI_H
#include <WriterFilterDllApi.hxx>
#endif
#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/lang/XComponent.hpp>

namespace writerfilter {
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

    const FontEntry*    getFontEntry(sal_uInt32 nIndex);
    sal_uInt32          size();
};
typedef boost::shared_ptr< FontTable >          FontTablePtr;
}}

#endif //
