/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _XLTOOLBAR_HXX
#define _XLTOOLBAR_HXX

#include <filter/msfilter/mstoolbar.hxx>

class ScCTBWrapper;
// hmm I don't normally use these packed structures
// but.. hey always good to do something different
class TBCCmd : public TBBase
{
public:
    TBCCmd() : cmdID(0), A(false), B(false), cmdType(0), C(false), reserved3(0)
    {}
    sal_uInt16 cmdID;
    bool A:1;
    bool B:1;
    sal_uInt16 cmdType:5;
    bool C:1;
    sal_uInt16 reserved3:8;
    bool Read( SvStream& rS ) SAL_OVERRIDE;
    void Print(FILE* fp) SAL_OVERRIDE;
};

class ScTBC : public TBBase
{
    TBCHeader tbch;
    boost::shared_ptr<TBCCmd> tbcCmd; // optional
    boost::shared_ptr<TBCData> tbcd;
public:
    ScTBC();
    virtual ~ScTBC(){}
    void Print( FILE* ) SAL_OVERRIDE;
    bool Read(SvStream &rS) SAL_OVERRIDE;
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
    ScCTB(sal_uInt16);
    virtual ~ScCTB(){}
    void Print( FILE* ) SAL_OVERRIDE;
    bool Read(SvStream &rS) SAL_OVERRIDE;
    bool IsMenuToolbar();
    bool ImportCustomToolBar( ScCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenuTB( ScCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
    OUString GetName() { return tb.getName().getString(); }


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
    virtual ~CTBS(){}
    void Print( FILE* ) SAL_OVERRIDE;
    bool Read(SvStream &rS) SAL_OVERRIDE;
};

class ScCTBWrapper : public TBBase
{
    CTBS ctbSet;

    std::vector< ScCTB > rCTB;

public:
    ScCTBWrapper();
    virtual ~ScCTBWrapper();
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
    ScCTB* GetCustomizationData( const OUString& name );
};


#endif //_XLTOOLBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
