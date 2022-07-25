/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SW_SOURCE_FILTER_WW8_WW8TOOLBAR_HXX
#define INCLUDED_SW_SOURCE_FILTER_WW8_WW8TOOLBAR_HXX

#include <com/sun/star/container/XIndexContainer.hpp>
#include <filter/msfilter/mstoolbar.hxx>
#include <memory>


class SfxObjectShell;
class SwCTBWrapper;

class Xst : public TBBase
{
    OUString m_sString;

public:
    Xst(){}
    bool Read(SvStream &rS) override;
    const OUString& getString() const { return m_sString; }
};

class SwTBC : public TBBase
{
    TBCHeader m_tbch;
    std::shared_ptr< sal_uInt32 > m_cid; // optional
    std::shared_ptr<TBCData> m_tbcd;

public:
    SwTBC();
    bool Read(SvStream &rS) override;
    bool ImportToolBarControl( SwCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper&, bool );
    OUString GetCustomText();
};

class SwCTB : public TBBase
{
    Xst m_name;
    sal_Int32 m_cbTBData;
    TB m_tb;
    std::vector<TBVisualData> m_rVisualData;
    sal_Int32 m_iWCTBl;
    sal_uInt16 m_reserved;
    sal_uInt16 m_unused;
    sal_Int32 m_cCtls;
    std::vector< SwTBC > m_rTBC;

    SwCTB(const SwCTB&) = delete;
    SwCTB& operator = ( const SwCTB&) = delete;

public:
    SwCTB();
    virtual ~SwCTB() override;
    bool Read(SvStream &rS) override;
    bool IsMenuToolbar() const;
    bool ImportCustomToolBar( SwCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenuTB( SwCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
    OUString const & GetName() { return m_tb.getName().getString(); }
};

class TBDelta : public TBBase
{
    sal_uInt8 m_doprfatendFlags;

    sal_uInt8 m_ibts;
    sal_Int32 m_cidNext;
    sal_Int32 m_cid;
    sal_Int32 m_fc;
    sal_uInt16 m_CiTBDE; // careful of this ( endian matters etc. )
    sal_uInt16 m_cbTBC;

public:
    TBDelta();
    bool Read(SvStream &rS) override;
    bool ControlIsInserted();
    bool ControlDropsToolBar();
    sal_Int32 TBCStreamOffset() { return m_fc;}
    sal_Int16 CustomizationIndex();
};

class Tcg255SubStruct : public TBBase
{
    friend class Tcg255;

    Tcg255SubStruct(const Tcg255SubStruct&) = delete;
    Tcg255SubStruct& operator = ( const Tcg255SubStruct&) = delete;

protected:
    sal_uInt8 m_ch;

public:
    explicit Tcg255SubStruct();
    sal_uInt8 id() const { return m_ch; }
    bool Read(SvStream &rS) override;
};

class Customization : public TBBase
{
    friend class SwCTBWrapper;

    sal_Int32 m_tbidForTBD;
    sal_uInt16 m_reserved1;
    sal_uInt16 m_ctbds;
    SwCTBWrapper* m_pWrapper;
    std::shared_ptr< SwCTB > m_customizationDataCTB;
    std::vector< TBDelta > m_customizationDataTBDelta;
    bool m_bIsDroppedMenuTB;

public:
    explicit Customization( SwCTBWrapper* rapper );
    bool Read(SvStream &rS) override;
    bool ImportCustomToolBar( SwCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenu( SwCTBWrapper&, CustomToolBarImportHelper& );
    SwCTB*  GetCustomizationData() { return m_customizationDataCTB.get(); };
};

class SwCTBWrapper : public Tcg255SubStruct
{
    // reserved1 is the ch field of Tcg255SubStruct
    sal_uInt16 m_reserved2;
    sal_uInt8 m_reserved3;
    sal_uInt16 m_reserved4;
    sal_uInt16 m_reserved5;

    sal_Int16 m_cbTBD;
    sal_uInt16 m_cCust;

    sal_Int32 m_cbDTBC;

    std::vector< SwTBC > m_rtbdc;
    std::vector< Customization > m_rCustomizations; // array of Customizations
    std::vector< sal_Int16 > m_dropDownMenuIndices; // array of indexes of Customization toolbars that are dropped by a menu
    SwCTBWrapper(const SwCTBWrapper&) = delete;
    SwCTBWrapper& operator = ( const SwCTBWrapper&) = delete;

public:
    explicit SwCTBWrapper();
    virtual ~SwCTBWrapper() override;
    void InsertDropIndex( sal_Int32 aIndex ) { m_dropDownMenuIndices.push_back( aIndex ); }
    SwTBC* GetTBCAtOffset( sal_uInt32 nStreamOffset );
    bool Read(SvStream &rS) override;
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );

    Customization* GetCustomizaton( sal_Int16 index );
    SwCTB* GetCustomizationData( const OUString& name );
};

class MCD : public TBBase
{
    sal_Int8 m_reserved1; //  A signed integer that MUST be 0x56.
    sal_uInt8 m_reserved2; // MUST be 0.
    sal_uInt16 m_ibst; // Unsigned integer that specifies the name of the macro. Macro name is specified by MacroName.xstz of the MacroName entry in the MacroNames such that MacroName.ibst equals ibst. MacroNames MUST contain such an entry.
    sal_uInt16 m_ibstName; // An unsigned integer that specifies the index into the Command String Table (TcgSttbf.sttbf) where the macro's name and arguments are specified.
    sal_uInt16 m_reserved3; // An unsigned integer that MUST be 0xFFFF.
    sal_uInt32 m_reserved4; //MUST be ignored.
    sal_uInt32 m_reserved5; //MUST be 0.
    sal_uInt32 m_reserved6; //MUST be ignored.
    sal_uInt32 m_reserved7; //MUST be ignored

public:
    MCD();
    bool Read(SvStream &rS) override;
};

class PlfMcd : public Tcg255SubStruct
{
    sal_Int32 m_iMac;
    std::vector<MCD> m_rgmcd; // array of MCD's
    PlfMcd(const PlfMcd&) = delete;
    PlfMcd& operator = ( const PlfMcd&) = delete;

public:
    explicit PlfMcd();
    bool Read(SvStream &rS) override;
};

class Acd : public TBBase
{
    sal_Int16 m_ibst;
    sal_uInt16 m_fciBasedOnABC; //  fciBasedOn(13 bits) A(1bit)B(1bit)C(1Bit)
    Acd(const Acd&) = delete;
    Acd& operator = ( const Acd&) = delete;

public:
    Acd();
    bool Read(SvStream &rS) override;
};

class PlfAcd: public Tcg255SubStruct
{
    sal_Int32 m_iMac;
    std::unique_ptr<Acd[]> m_rgacd;
    PlfAcd(const PlfAcd&) = delete;
    PlfAcd& operator = ( const PlfAcd&) = delete;

public:
    explicit PlfAcd();
    virtual ~PlfAcd() override;
    bool Read(SvStream &rS) override;
};

class Kme : public TBBase
{
    sal_Int16 m_reserved1; //MUST be zero.
    sal_Int16 m_reserved2; //MUST be zero.
    sal_uInt16 m_kcm1; //A Kcm that specifies the primary shortcut key.
    sal_uInt16 m_kcm2; //A Kcm that specifies the secondary shortcut key, or 0x00FF if there is no secondary shortcut key.
    sal_uInt16 m_kt; //A Kt that specifies the type of action to be taken when the key combination is pressed.
    sal_uInt32 m_param; //The meaning of this field depends on the value of kt

    Kme(const Kme&) = delete;
    Kme& operator = ( const Kme&) = delete;

public:
    Kme();
    virtual ~Kme() override;
    bool Read(SvStream &rS) override;
};

class PlfKme : public Tcg255SubStruct
{
    sal_Int32 m_iMac;
    std::unique_ptr<Kme[]> m_rgkme;
    PlfKme(const PlfKme&) = delete;
    PlfKme& operator = ( const PlfKme&) = delete;

public:
    explicit PlfKme();
    virtual ~PlfKme() override;
    bool Read(SvStream &rS) override;
};

class TcgSttbfCore : public TBBase
{
    struct SBBItem
    {
        sal_uInt16 cchData;
        OUString data;
        sal_uInt16 extraData;
        SBBItem() : cchData(0), extraData(0){}
    };

    sal_uInt16 m_fExtend;
    sal_uInt16 m_cData;
    sal_uInt16 m_cbExtra;
    std::unique_ptr<SBBItem[]> m_dataItems;
    TcgSttbfCore(const TcgSttbfCore&) = delete;
    TcgSttbfCore& operator = ( const TcgSttbfCore&) = delete;

public:
    TcgSttbfCore();
    virtual ~TcgSttbfCore() override;
    bool Read(SvStream &rS) override;
};

class TcgSttbf : public Tcg255SubStruct
{
    TcgSttbfCore m_sttbf;
    TcgSttbf(const TcgSttbf&) = delete;
    TcgSttbf& operator = ( const TcgSttbf&) = delete;

public:
    explicit TcgSttbf();
    bool Read(SvStream &rS) override;
};

class Xstz : public TBBase
{
    Xst m_xst; //An Xst specifying the string with its pre-pended length.
    sal_uInt16 m_chTerm;

    Xstz(const Xstz&) = delete;
    Xstz& operator = ( const Xstz&) = delete;

public:
    Xstz();
    bool Read(SvStream &rS) override;
};

class MacroName : public TBBase
{
    sal_uInt16 m_ibst; //An unsigned integer that specifies the index of the current entry in the macro name table. MUST NOT be the same as the index of any other entry.
    Xstz m_xstz;
    MacroName(const MacroName&) = delete;
    MacroName& operator = ( const MacroName&) = delete;

public:
    MacroName();
    bool Read(SvStream &rS) override;
};

class MacroNames : public Tcg255SubStruct
{
    sal_uInt16 m_iMac; //An unsigned integer that specifies the number of MacroName structures in rgNames.
    std::unique_ptr<MacroName[]> m_rgNames;

    MacroNames(const MacroNames&) = delete;
    MacroNames& operator = ( const MacroNames&) = delete;

public:
    explicit MacroNames();
    virtual ~MacroNames() override;
    bool Read(SvStream &rS) override;
};

class Tcg255 : public TBBase
{
    std::vector< std::unique_ptr<Tcg255SubStruct> > m_rgtcgData; // array of sub structures
    Tcg255(const Tcg255&) = delete;
    Tcg255& operator = ( const Tcg255&) = delete;
    bool processSubStruct( sal_uInt8 nId, SvStream& );

public:
    Tcg255();
    virtual ~Tcg255() override;
    bool Read(SvStream &rS) override;
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
};

class Tcg: public TBBase
{
    sal_Int8 m_nTcgVer;
    std::unique_ptr< Tcg255 > m_tcg;
    Tcg(const Tcg&) = delete;
    Tcg& operator = ( const Tcg&) = delete;

public:
    Tcg();
    bool Read(SvStream &rS) override;
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
