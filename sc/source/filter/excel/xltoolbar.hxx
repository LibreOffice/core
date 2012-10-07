/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Noel Power <noel.power@novell.com>
 * Portions created by the Initial Developer are Copyright (C) 2010 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *       Noel Power <noel.power@novell.com>
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _XLTOOLBAR_HXX
#define _XLTOOLBAR_HXX

#include <filter/msfilter/mstoolbar.hxx>

namespace css = ::com::sun::star;

class ScCTBWrapper;
// hmm I don't normally use these packed structures
// but.. hey always good to do something different
class TBCCmd : public TBBase
{
public:
    TBCCmd() : cmdID(0), A(0), B(0), cmdType(0), C(0), reserved3(0) {}
    sal_uInt16 cmdID;
    sal_uInt16 A:1;
    sal_uInt16 B:1;
    sal_uInt16 cmdType:5;
    sal_uInt16 C:1;
    sal_uInt16 reserved3:8;
    bool Read( SvStream& rS );
    void Print(FILE* fp);
};

class ScTBC : public TBBase
{
    TBCHeader tbch;
    boost::shared_ptr<TBCCmd> tbcCmd; // optional
    boost::shared_ptr<TBCData> tbcd;
public:
    ScTBC();
    ~ScTBC(){}
    void Print( FILE* );
    bool Read(SvStream &rS);
    bool ImportToolBarControl( ScCTBWrapper&, const com::sun::star::uno::Reference< com::sun::star::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper, bool bIsMenuBar );
};

class ScCTB : public TBBase
{
    sal_uInt16 nViews;
    TB tb;
    std::vector<TBVisualData> rVisualData;
    sal_uInt32 ectbid;
    std::vector< ScTBC > rTBC;
    bool ImportCustomToolBar_Impl( ScCTBWrapper&, CustomToolBarImportHelper& );
public:
    ScCTB();
    ScCTB(sal_uInt16);
    ~ScCTB(){}
    void Print( FILE* );
    bool Read(SvStream &rS);
    bool IsMenuToolbar();
    bool ImportCustomToolBar( ScCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenuTB( ScCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
    rtl::OUString GetName() { return tb.getName().getString(); }


};

class CTBS : public TBBase
{
public:
    sal_uInt8 bSignature;
    sal_uInt8 bVersion;
    sal_uInt16 reserved1;
    sal_uInt16 reserved2;
    sal_uInt16 reserved3;
    sal_uInt16 ctb;
    sal_uInt16 ctbViews;
    sal_uInt16 ictbView;
    CTBS(const CTBS&);
    CTBS& operator = ( const CTBS&);
    CTBS();
    ~CTBS(){}
    void Print( FILE* );
    bool Read(SvStream &rS);
};

class ScCTBWrapper : public TBBase
{
    CTBS ctbSet;

    std::vector< ScCTB > rCTB;

public:
    ScCTBWrapper();
    ~ScCTBWrapper();
    bool Read(SvStream &rS);
    void Print( FILE* );
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
    ScCTB* GetCustomizationData( const rtl::OUString& name );
};


#endif //_XLTOOLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
