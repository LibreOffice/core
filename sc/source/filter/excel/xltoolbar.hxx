/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <filter/msfilter/mstoolbar.hxx>

namespace com::sun::star::container { class XIndexContainer; }

class ScCTBWrapper;
// hmm I don't normally use these packed structures
// but... hey always good to do something different
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
    bool Read( SvStream& rS ) override;
#ifdef DEBUG_SC_EXCEL
    virtual void Print(FILE* fp) override;
#endif
};

class ScTBC : public TBBase
{
    TBCHeader tbch;
    std::shared_ptr<TBCCmd> tbcCmd; // optional
    std::shared_ptr<TBCData> tbcd;
public:
    ScTBC();
#ifdef DEBUG_SC_EXCEL
    virtual void Print( FILE* ) override;
#endif
    bool Read(SvStream &rS) override;
    bool ImportToolBarControl( ScCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >& toolbarcontainer, CustomToolBarImportHelper& helper, bool bIsMenuBar );
};

class ScCTB : public TBBase
{
    sal_uInt16 nViews;
    TB tb;
    std::vector<TBVisualData> rVisualData;
    sal_uInt32 ectbid;
    std::vector< ScTBC > rTBC;
public:
    explicit ScCTB(sal_uInt16);
#ifdef DEBUG_SC_EXCEL
    virtual void Print( FILE* ) override;
#endif
    bool Read(SvStream &rS) override;
    bool IsMenuToolbar() const;
    bool ImportCustomToolBar( ScCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenuTB( ScCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
    const OUString& GetName() { return tb.getName().getString(); }

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
#ifdef DEBUG_SC_EXCEL
    virtual void Print( FILE* ) override;
#endif
    bool Read(SvStream &rS) override;
};

class ScCTBWrapper : public TBBase
{
    CTBS ctbSet;

    std::vector< ScCTB > rCTB;

public:
    ScCTBWrapper();
    virtual ~ScCTBWrapper() override;
    bool Read(SvStream &rS) override;
#ifdef DEBUG_SC_EXCEL
    virtual void Print( FILE* ) override;
#endif
    void ImportCustomToolBar( SfxObjectShell& rDocSh );
    ScCTB* GetCustomizationData( const OUString& name );
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
