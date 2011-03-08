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
#ifndef _WW8TOOLBAR_HXX
#define _WW8TOOLBAR_HXX

#include <filter/msfilter/mstoolbar.hxx>

namespace css = ::com::sun::star;

class Xst : public TBBase
{
    rtl::OUString sString;
public:
    Xst(){}
    bool Read(SvStream *pS);
    rtl::OUString getString() { return sString; }
    void Print( FILE* fp );
};

class CTBWrapper;
class TBC : public TBBase
{
    TBCHeader tbch;
    boost::shared_ptr< sal_uInt32 > cid; // optional
    boost::shared_ptr<TBCData> tbcd;
public:
    TBC();
    ~TBC();
    bool Read(SvStream *pS);
    void Print( FILE* );
    bool ImportToolBarControl( CTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper&, bool );
    rtl::OUString GetCustomText();
};

class CTB : public TBBase
{
    Xst name;
    sal_Int32 cbTBData;
    TB tb;
    std::vector<TBVisualData> rVisualData;
    sal_Int32 iWCTBl;
    sal_uInt16 reserved;
    sal_uInt16 unused;
    sal_Int32 cCtls;
    std::vector< TBC > rTBC;

    CTB(const CTB&);
    CTB& operator = ( const CTB&);
public:
    CTB();
    ~CTB();
    bool Read(SvStream *pS);
    void Print( FILE* fp );
    bool IsMenuToolbar();
    bool ImportCustomToolBar( CTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenuTB( CTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
    rtl::OUString GetName() { return tb.getName().getString(); }
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
};

class CTBWrapper;
class Customization : public TBBase
{
friend class CTBWrapper;
    sal_Int32 tbidForTBD;
    sal_uInt16 reserved1;
    sal_Int16 ctbds;
    CTBWrapper* pWrapper;
    boost::shared_ptr< CTB > customizationDataCTB;
    std::vector< TBDelta > customizationDataTBDelta;
    bool bIsDroppedMenuTB;
    bool ImportMenu( CTBWrapper&, const css::uno::Reference< css::container::XIndexContainer >&, CustomToolBarImportHelper& );
public:
    Customization( CTBWrapper* rapper );
    ~Customization();
    bool Read(SvStream *pS);
    bool ImportCustomToolBar( CTBWrapper&, CustomToolBarImportHelper& );
    bool ImportMenu( CTBWrapper&, CustomToolBarImportHelper& );
    void Print( FILE* );
    sal_Int32 GetTBIDForTB(){ return tbidForTBD; }
    CTB*  GetCustomizationData() { return customizationDataCTB.get(); };
};

class SfxObjectShell;

class CTBWrapper : public Tcg255SubStruct
{
    // reserved1 is the ch field of Tcg255SubStruct
    sal_uInt16 reserved2;
    sal_uInt8 reserved3;
    sal_uInt16 reserved4;
    sal_uInt16 reserved5;

    sal_Int16 cbTBD;
    sal_Int16 cCust;

    sal_Int32 cbDTBC;

    std::vector< TBC > rtbdc; //
    std::vector< Customization > rCustomizations; // array of Customizations
    std::vector< sal_Int16 > dropDownMenuIndices; // array of indexes of Customization toolbars that are dropped by a menu
    CTBWrapper(const CTBWrapper&);
    CTBWrapper& operator = ( const CTBWrapper&);
public:
    CTBWrapper( bool bReadId = true );
    ~CTBWrapper();
    void InsertDropIndex( sal_Int32 aIndex ) { dropDownMenuIndices.push_back( aIndex ); }
    TBC* GetTBCAtOffset( sal_uInt32 nStreamOffset );
    bool Read(SvStream *pS);
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );

    Customization* GetCustomizaton( sal_Int16 index );
    CTB* GetCustomizationData( const rtl::OUString& name );
    void Print( FILE* );
};

class MCD : public TBBase
{
    sal_Int8 reserved1; //  A signed integer that MUST be 0x56.
    sal_uInt8 reserved2; // MUST be 0.
    sal_uInt16 ibst; // Unsigned integer that specifies the name of the macro. Macro name is specified by MacroName.xstz of the MacroName entry in the MacroNames such that MacroName.ibst equals ibst. MacroNames MUST contain such an entry.
    sal_uInt16 ibstName; // An unsigned integer that specifies the index into the Command String Table (TcgSttbf.sttbf) where the macro‘s name and arguments are specified.
    sal_uInt16 reserved3; // An unsigned integer that MUST be 0xFFFF.
    sal_uInt32 reserved4; //MUST be ignored.
    sal_uInt32 reserved5; //MUST be 0.
    sal_uInt32 reserved6; //MUST be ignored.
    sal_uInt32 reserved7; //MUST be ignored

    MCD(const MCD&);
    MCD& operator = ( const MCD&);
public:
    MCD();
    ~MCD(){}
    bool Read(SvStream *pS);
    void Print( FILE* );
};

class PlfMcd : public Tcg255SubStruct
{
    sal_Int32 iMac;
    MCD* rgmcd; // array of MCD's
    PlfMcd(const PlfMcd&);
    PlfMcd& operator = ( const PlfMcd&);
public:
    PlfMcd( bool bReadId = true );
    ~PlfMcd();
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
    void Print( FILE* );
};

class TcgSttbfCore : public TBBase
{
struct SBBItem
{
    sal_uInt16 cchData;
    rtl::OUString data;
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
    void Print( FILE* );
};

class Tcg255 : public TBBase
{
    std::vector< Tcg255SubStruct* > rgtcgData; // array of sub structures
    Tcg255(const Tcg255&);
    Tcg255& operator = ( const Tcg255&);
    bool processSubStruct( sal_uInt8 nId, SvStream*  );
public:
    Tcg255();
    ~Tcg255();
    bool Read(SvStream *pS);
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
    bool Read(SvStream *pS);
    bool ImportCustomToolBar( SfxObjectShell& rDocSh );
    void Print( FILE* );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
