/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _WW8TOOLBAR_HXX
#define _WW8TOOLBAR_HXX

#include <filter/msfilter/mstoolbar.hxx>

class Xst : public TBBase
{
    OUString sString;
public:
    Xst(){}
    bool Read(SvStream &rS);
    OUString getString() { return sString; }
    void Print( FILE* fp );
};

class SwCTBWrapper;
class SwTBC : public TBBase
{
    TBCHeader tbch;
    boost::shared_ptr< sal_uInt32 > cid; // optional
    boost::shared_ptr<TBCData> tbcd;
public:
    SwTBC();
    ~SwTBC();
    bool Read(SvStream &rS);
    void Print( FILE* );
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

    SwCTB(const SwCTB&);
    SwCTB& operator = ( const SwCTB&);
public:
    SwCTB();
    ~SwCTB();
    bool Read(SvStream &rS);
    void Print( FILE* fp );
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
    ~TBDelta(){}
    bool Read(SvStream &rS);
    void Print( FILE* );
    bool ControlIsModified();
    bool ControlIsInserted();
    bool ControlIsChanged();
    bool ControlDropsToolBar();
    sal_Int32 TBCStreamOffset();
    sal_Int16 CustomizationIndex();

};

class Tcg255SubStruct : public TBBase
{
friend class Tcg255;
    bool mbReadId;
    Tcg255SubStruct(const Tcg255SubStruct&);
    Tcg255SubStruct& operator = ( const Tcg255SubStruct&);
protected:
    sal_uInt8 ch;
public:
    Tcg255SubStruct( bool bReadId );
    ~Tcg255SubStruct(){}
    virtual sal_uInt8 id() const { return ch; }
    bool Read(SvStream &rS);
};

class SwCTBWrapper;
class Customization : public TBBase
{
friend class SwCTBWrapper;
    sal_Int32 tbidForTBD;
    sal_uInt16 reserved1;
    sal_Int16 ctbds;
    SwCTBWrapper* pWrapper;
    boost::shared_ptr< SwCTB > customizationDataCTB;
    std::vector< TBDelta > customizationDataTBDelta;
    bool bIsDroppedMenuTB;
    bool ImportMenu( SwCTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
public:
    Customization( SwCTBWrapper* rapper );
    ~Customization();
    bool Read(SvStream &rS);
    bool ImportCustomToolBar( SwCTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenu( SwCTBWrapper&, CustomToolBarImportHelper& );
    void Print( FILE* );
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
    sal_Int16 cCust;

    sal_Int32 cbDTBC;

    std::vector< SwTBC > rtbdc; //
    std::vector< Customization > rCustomizations; // array of Customizations
    std::vector< sal_Int16 > dropDownMenuIndices; // array of indexes of Customization toolbars that are dropped by a menu
    SwCTBWrapper(const SwCTBWrapper&);
    SwCTBWrapper& operator = ( const SwCTBWrapper&);
public:
    SwCTBWrapper( bool bReadId = true );
    ~SwCTBWrapper();
    void InsertDropIndex( sal_Int32 aIndex ) { dropDownMenuIndices.push_back( aIndex ); }
    SwTBC* GetTBCAtOffset( sal_uInt32 nStreamOffset );
    bool Read(SvStream &rS);
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );

    Customization* GetCustomizaton( sal_Int16 index );
    SwCTB* GetCustomizationData( const OUString& name );
    void Print( FILE* );
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
    bool Read(SvStream &rS);
    void Print( FILE* );
};

class PlfMcd : public Tcg255SubStruct
{
    sal_Int32 iMac;
    std::vector<MCD> rgmcd; // array of MCD's
    PlfMcd(const PlfMcd&);
    PlfMcd& operator = ( const PlfMcd&);
public:
    PlfMcd( bool bReadId = true );
    bool Read(SvStream &rS);
    void Print( FILE* );
};

class Acd : public TBBase
{
    sal_Int16 ibst;
    sal_uInt16 fciBasedOnABC; //  fciBasedOn(13 bits) A(1bit)B(1bit)C(1Bit)
    Acd(const Acd&);
    Acd& operator = ( const Acd&);
public:
    Acd();
    ~Acd(){}
    bool Read(SvStream &rS);
    void Print( FILE* );
};

class PlfAcd: public Tcg255SubStruct
{
    sal_Int32 iMac;
    Acd* rgacd;
    PlfAcd(const PlfAcd&);
    PlfAcd& operator = ( const PlfAcd&);
public:
    PlfAcd( bool bReadId = true );
    ~PlfAcd();
    bool Read(SvStream &rS);
    void Print(FILE*);
};

class Kme : public TBBase
{
    sal_Int16 reserved1; //MUST be zero.
    sal_Int16 reserved2; //MUST be zero.
    sal_uInt16 kcm1; //A Kcm that specifies the primary shortcut key.
    sal_uInt16 kcm2; //A Kcm that specifies the secondary shortcut key, or 0x00FF if there is no secondary shortcut key.
    sal_uInt16 kt; //A Kt that specifies the type of action to be taken when the key combination is pressed.
    sal_uInt32 param; //The meaning of this field depends on the value of kt

    Kme(const Kme&);
    Kme& operator = ( const Kme&);
public:
    Kme();
    ~Kme();
    bool Read(SvStream &rS);
    void Print( FILE* );
};

class PlfKme : public Tcg255SubStruct
{
    sal_Int32 iMac;
    Kme* rgkme;
    PlfKme(const PlfKme&);
    PlfKme& operator = ( const PlfKme&);
public:
    PlfKme( bool bReadId = true );
    ~PlfKme();
    bool Read(SvStream &rS);
    void Print( FILE* );
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
    TcgSttbfCore(const TcgSttbfCore&);
    TcgSttbfCore& operator = ( const TcgSttbfCore&);
public:
    TcgSttbfCore();
    ~TcgSttbfCore();
    bool Read(SvStream &rS);
    void Print( FILE* fp );
};

class TcgSttbf : public Tcg255SubStruct
{
    TcgSttbfCore sttbf;
    TcgSttbf(const TcgSttbf&);
    TcgSttbf& operator = ( const TcgSttbf&);
public:
    TcgSttbf( bool bReadId = true );
    ~TcgSttbf(){}
    bool Read(SvStream &rS);
    void Print( FILE* fp );
};

class Xstz : public TBBase
{
    Xst xst; //An Xst specifying the string with its pre-pended length.
    sal_uInt16 chTerm;

    Xstz(const Xstz&);
    Xstz& operator = ( const Xstz&);
public:
    Xstz();
    ~Xstz(){}
    bool Read(SvStream &rS);
    void Print( FILE* fp );
};

class MacroName : public TBBase
{
    sal_uInt16 ibst; //An unsigned integer that specifies the index of the current entry in the macro name table. MUST NOT be the same as the index of any other entry.
    Xstz xstz;
    MacroName(const MacroName&);
    MacroName& operator = ( const MacroName&);
public:
    MacroName();
    ~MacroName(){}
    bool Read(SvStream &rS);
    void Print( FILE* );
};

class MacroNames : public Tcg255SubStruct
{
    sal_uInt16 iMac; //An unsigned integer that specifies the number of MacroName structures in rgNames.
    MacroName* rgNames;

    MacroNames(const MacroNames&);
    MacroNames& operator = ( const MacroNames&);
public:
    MacroNames( bool bReadId = true );
    ~MacroNames();
    bool Read(SvStream &rS);
    void Print( FILE* );
};

class Tcg255 : public TBBase
{
    std::vector< Tcg255SubStruct* > rgtcgData; // array of sub structures
    Tcg255(const Tcg255&);
    Tcg255& operator = ( const Tcg255&);
    bool processSubStruct( sal_uInt8 nId, SvStream& );
public:
    Tcg255();
    ~Tcg255();
    bool Read(SvStream &rS);
    void Print( FILE* );
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
};

class Tcg: public TBBase
{
    sal_Int8 nTcgVer;
    std::auto_ptr< Tcg255 > tcg;
    Tcg(const Tcg&);
    Tcg& operator = ( const Tcg&);
public:
    Tcg();
    ~Tcg(){}
    bool Read(SvStream &rS);
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
    void Print( FILE* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
