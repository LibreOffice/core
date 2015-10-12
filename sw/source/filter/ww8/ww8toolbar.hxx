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

#include <filter/msfilter/mstoolbar.hxx>

class Xst : public TBBase
{
    OUString sString;

public:
    Xst(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    OUString getString() { return sString; }
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* fp ) SAL_OVERRIDE;
#endif
};

class SwCTBWrapper;

class SwTBC : public TBBase
{
    TBCHeader tbch;
    std::shared_ptr< sal_uInt32 > cid; // optional
    std::shared_ptr<TBCData> tbcd;

public:
    SwTBC();
    virtual ~SwTBC();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
    bool ImportToolBarControl( SwCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper&, bool );
    OUString GetCustomText();
};

class SwCTB : public TBBase
{
    Xst name;
    sal_Int32 cbTBData;
    TB tb;
    std::vector<TBVisualData> rVisualData;
    sal_Int32 iWCTBl;
    sal_uInt16 reserved;
    sal_uInt16 unused;
    sal_Int32 cCtls;
    std::vector< SwTBC > rTBC;

    SwCTB(const SwCTB&) = delete;
    SwCTB& operator = ( const SwCTB&) = delete;

public:
    SwCTB();
    virtual ~SwCTB();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* fp ) SAL_OVERRIDE;
#endif
    bool IsMenuToolbar();
    bool ImportCustomToolBar( SwCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenuTB( SwCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
    OUString GetName() { return tb.getName().getString(); }
};

class TBDelta : public TBBase
{
    sal_uInt8 doprfatendFlags;

    sal_uInt8 ibts;
    sal_Int32 cidNext;
    sal_Int32 cid;
    sal_Int32 fc;
    sal_uInt16 CiTBDE; // careful of this ( endian matters etc. )
    sal_uInt16 cbTBC;

public:
    TBDelta();
    virtual ~TBDelta(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
    bool ControlIsInserted();
    bool ControlDropsToolBar();
    sal_Int32 TBCStreamOffset() { return fc;}
    sal_Int16 CustomizationIndex();
};

class Tcg255SubStruct : public TBBase
{
    friend class Tcg255;

    bool mbReadId;
    Tcg255SubStruct(const Tcg255SubStruct&) = delete;
    Tcg255SubStruct& operator = ( const Tcg255SubStruct&) = delete;

protected:
    sal_uInt8 ch;

public:
    explicit Tcg255SubStruct( bool bReadId );
    virtual ~Tcg255SubStruct(){}
    sal_uInt8 id() const { return ch; }
    bool Read(SvStream &rS) SAL_OVERRIDE;
};

class SwCTBWrapper;

class Customization : public TBBase
{
    friend class SwCTBWrapper;

    sal_Int32 tbidForTBD;
    sal_uInt16 reserved1;
    sal_uInt16 ctbds;
    SwCTBWrapper* pWrapper;
    std::shared_ptr< SwCTB > customizationDataCTB;
    std::vector< TBDelta > customizationDataTBDelta;
    bool bIsDroppedMenuTB;

public:
    explicit Customization( SwCTBWrapper* rapper );
    virtual ~Customization();
    bool Read(SvStream &rS) SAL_OVERRIDE;
    bool ImportCustomToolBar( SwCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenu( SwCTBWrapper&, CustomToolBarImportHelper& );
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
    sal_Int32 GetTBIDForTB(){ return tbidForTBD; }
    SwCTB*  GetCustomizationData() { return customizationDataCTB.get(); };
};

class SfxObjectShell;

class SwCTBWrapper : public Tcg255SubStruct
{
    // reserved1 is the ch field of Tcg255SubStruct
    sal_uInt16 reserved2;
    sal_uInt8 reserved3;
    sal_uInt16 reserved4;
    sal_uInt16 reserved5;

    sal_Int16 cbTBD;
    sal_uInt16 cCust;

    sal_Int32 cbDTBC;

    std::vector< SwTBC > rtbdc;
    std::vector< Customization > rCustomizations; // array of Customizations
    std::vector< sal_Int16 > dropDownMenuIndices; // array of indexes of Customization toolbars that are dropped by a menu
    SwCTBWrapper(const SwCTBWrapper&) = delete;
    SwCTBWrapper& operator = ( const SwCTBWrapper&) = delete;

public:
    explicit SwCTBWrapper( bool bReadId = true );
    virtual ~SwCTBWrapper();
    void InsertDropIndex( sal_Int32 aIndex ) { dropDownMenuIndices.push_back( aIndex ); }
    SwTBC* GetTBCAtOffset( sal_uInt32 nStreamOffset );
    bool Read(SvStream &rS) SAL_OVERRIDE;
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );

    Customization* GetCustomizaton( sal_Int16 index );
    SwCTB* GetCustomizationData( const OUString& name );
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class MCD : public TBBase
{
    sal_Int8 reserved1; //  A signed integer that MUST be 0x56.
    sal_uInt8 reserved2; // MUST be 0.
    sal_uInt16 ibst; // Unsigned integer that specifies the name of the macro. Macro name is specified by MacroName.xstz of the MacroName entry in the MacroNames such that MacroName.ibst equals ibst. MacroNames MUST contain such an entry.
    sal_uInt16 ibstName; // An unsigned integer that specifies the index into the Command String Table (TcgSttbf.sttbf) where the macro's name and arguments are specified.
    sal_uInt16 reserved3; // An unsigned integer that MUST be 0xFFFF.
    sal_uInt32 reserved4; //MUST be ignored.
    sal_uInt32 reserved5; //MUST be 0.
    sal_uInt32 reserved6; //MUST be ignored.
    sal_uInt32 reserved7; //MUST be ignored

public:
    MCD();
    MCD(const MCD&);
    MCD& operator = ( const MCD&);
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class PlfMcd : public Tcg255SubStruct
{
    sal_Int32 iMac;
    std::vector<MCD> rgmcd; // array of MCD's
    PlfMcd(const PlfMcd&) = delete;
    PlfMcd& operator = ( const PlfMcd&) = delete;

public:
    explicit PlfMcd( bool bReadId = true );
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class Acd : public TBBase
{
    sal_Int16 ibst;
    sal_uInt16 fciBasedOnABC; //  fciBasedOn(13 bits) A(1bit)B(1bit)C(1Bit)
    Acd(const Acd&) = delete;
    Acd& operator = ( const Acd&) = delete;

public:
    Acd();
    virtual ~Acd(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class PlfAcd: public Tcg255SubStruct
{
    sal_Int32 iMac;
    Acd* rgacd;
    PlfAcd(const PlfAcd&) = delete;
    PlfAcd& operator = ( const PlfAcd&) = delete;

public:
    explicit PlfAcd( bool bReadId = true );
    virtual ~PlfAcd();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print(FILE*) SAL_OVERRIDE;
#endif
};

class Kme : public TBBase
{
    sal_Int16 reserved1; //MUST be zero.
    sal_Int16 reserved2; //MUST be zero.
    sal_uInt16 kcm1; //A Kcm that specifies the primary shortcut key.
    sal_uInt16 kcm2; //A Kcm that specifies the secondary shortcut key, or 0x00FF if there is no secondary shortcut key.
    sal_uInt16 kt; //A Kt that specifies the type of action to be taken when the key combination is pressed.
    sal_uInt32 param; //The meaning of this field depends on the value of kt

    Kme(const Kme&) = delete;
    Kme& operator = ( const Kme&) = delete;

public:
    Kme();
    virtual ~Kme();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class PlfKme : public Tcg255SubStruct
{
    sal_Int32 iMac;
    Kme* rgkme;
    PlfKme(const PlfKme&) = delete;
    PlfKme& operator = ( const PlfKme&) = delete;

public:
    explicit PlfKme( bool bReadId = true );
    virtual ~PlfKme();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
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

    sal_uInt16 fExtend;
    sal_uInt16 cData;
    sal_uInt16 cbExtra;
    SBBItem* dataItems;
    TcgSttbfCore(const TcgSttbfCore&) = delete;
    TcgSttbfCore& operator = ( const TcgSttbfCore&) = delete;

public:
    TcgSttbfCore();
    virtual ~TcgSttbfCore();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* fp ) SAL_OVERRIDE;
#endif
};

class TcgSttbf : public Tcg255SubStruct
{
    TcgSttbfCore sttbf;
    TcgSttbf(const TcgSttbf&) = delete;
    TcgSttbf& operator = ( const TcgSttbf&) = delete;

public:
    explicit TcgSttbf( bool bReadId = true );
    virtual ~TcgSttbf(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* fp ) SAL_OVERRIDE;
#endif
};

class Xstz : public TBBase
{
    Xst xst; //An Xst specifying the string with its pre-pended length.
    sal_uInt16 chTerm;

    Xstz(const Xstz&) = delete;
    Xstz& operator = ( const Xstz&) = delete;

public:
    Xstz();
    virtual ~Xstz(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* fp ) SAL_OVERRIDE;
#endif
};

class MacroName : public TBBase
{
    sal_uInt16 ibst; //An unsigned integer that specifies the index of the current entry in the macro name table. MUST NOT be the same as the index of any other entry.
    Xstz xstz;
    MacroName(const MacroName&) = delete;
    MacroName& operator = ( const MacroName&) = delete;

public:
    MacroName();
    virtual ~MacroName(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class MacroNames : public Tcg255SubStruct
{
    sal_uInt16 iMac; //An unsigned integer that specifies the number of MacroName structures in rgNames.
    MacroName* rgNames;

    MacroNames(const MacroNames&) = delete;
    MacroNames& operator = ( const MacroNames&) = delete;

public:
    explicit MacroNames( bool bReadId = true );
    virtual ~MacroNames();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

class Tcg255 : public TBBase
{
    std::vector< Tcg255SubStruct* > rgtcgData; // array of sub structures
    Tcg255(const Tcg255&) = delete;
    Tcg255& operator = ( const Tcg255&) = delete;
    bool processSubStruct( sal_uInt8 nId, SvStream& );

public:
    Tcg255();
    virtual ~Tcg255();
    bool Read(SvStream &rS) SAL_OVERRIDE;
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
};

class Tcg: public TBBase
{
    sal_Int8 nTcgVer;
    std::unique_ptr< Tcg255 > tcg;
    Tcg(const Tcg&) = delete;
    Tcg& operator = ( const Tcg&) = delete;

public:
    Tcg();
    virtual ~Tcg(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
#if OSL_DEBUG_LEVEL > 1
    virtual void Print( FILE* ) SAL_OVERRIDE;
#endif
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
