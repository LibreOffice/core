/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_FILTER_MSFILTER_MSTOOLBAR_HXX
#define INCLUDED_FILTER_MSFILTER_MSTOOLBAR_HXX
#include <filter/msfilter/msfilterdllapi.h>
#include <sot/storage.hxx>
#include <ostream>
#include <memory>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/container/XIndexContainer.hpp>
#include <com/sun/star/ui/XUIConfigurationManagerSupplier.hpp>
#include <com/sun/star/ui/ImageType.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <vcl/bitmap.hxx>
#include <cppuhelper/implbase1.hxx>

class TBCHeader;


class MSOCommandConvertor
{
public:
    virtual ~MSOCommandConvertor() {}
    virtual OUString MSOCommandToOOCommand( sal_Int16 msoCmd ) = 0;
    virtual OUString MSOTCIDToOOCommand( sal_Int16 msoTCID ) = 0;
};

class SfxObjectShell;

class MSFILTER_DLLPUBLIC CustomToolBarImportHelper
{
    struct iconcontrolitem
    {
        OUString sCommand;
        css::uno::Reference< css::graphic::XGraphic > image;
    };
    std::vector< iconcontrolitem > iconcommands;
    std::auto_ptr< MSOCommandConvertor > pMSOCmdConvertor;
    css::uno::Reference< css::ui::XUIConfigurationManagerSupplier > m_xCfgSupp;
    css::uno::Reference< css::ui::XUIConfigurationManager > m_xAppCfgMgr;
    SfxObjectShell& mrDocSh;
    void ScaleImage( css::uno::Reference< css::graphic::XGraphic >& xGraphic, long nNewSize );
public:
    CustomToolBarImportHelper( SfxObjectShell& rDocSh, const css::uno::Reference< css::ui::XUIConfigurationManager >& rxAppCfgMgr );

    void setMSOCommandMap( MSOCommandConvertor* pCnvtr ) { pMSOCmdConvertor.reset( pCnvtr ); }
    css::uno::Reference< css::ui::XUIConfigurationManager > getCfgManager();
    css::uno::Reference< css::ui::XUIConfigurationManager > getAppCfgManager();


    css::uno::Any createCommandFromMacro( const OUString& sCmd );

    void addIcon( const css::uno::Reference< css::graphic::XGraphic >& xImage, const OUString& sString );
    void applyIcons();
    OUString MSOCommandToOOCommand( sal_Int16 msoCmd );
    OUString MSOTCIDToOOCommand( sal_Int16 msoTCID );
    SfxObjectShell& GetDocShell() { return mrDocSh; }
    bool createMenu( const OUString& rName, const css::uno::Reference< css::container::XIndexAccess >& xMenuDesc, bool bPersist );
};

class MSFILTER_DLLPUBLIC TBBase
{
friend class Indent;
    static int nIndent; // num spaces to indent before printing
protected:
    void indent_printf(FILE* fp, const char* format, ... );
    sal_uInt32 nOffSet; // usually for debug we can store the offset in the stream to this record
public:
    TBBase() : nOffSet( 0 ) {}
    virtual ~TBBase(){}

    virtual bool Read(SvStream &rS) = 0;
    virtual void Print( FILE* ) {} // #FIXME remove this an implement the debug routines in all the classes below to enable some sort of readable output
    sal_uInt32 GetOffset() { return nOffSet; }
};

class Indent
{
public:
    Indent( bool binit = false )
    {
        if ( binit )
            init();
        else
            TBBase::nIndent = TBBase::nIndent + 2;
    }
    ~Indent() { TBBase::nIndent = TBBase::nIndent - 2; }
    void init() { TBBase::nIndent = 0; }
};


class MSFILTER_DLLPUBLIC WString : public TBBase
{
    OUString sString;

public:
    WString(){};
    ~WString(){};
    bool Read(SvStream &rS) SAL_OVERRIDE;
    OUString getString(){ return sString; }
};

class MSFILTER_DLLPUBLIC TBCExtraInfo : public TBBase
{
    WString wstrHelpFile;
    sal_Int32 idHelpContext;
    WString wstrTag;
    WString wstrOnAction;
    WString wstrParam;
    sal_Int8 tbcu;
    sal_Int8 tbmg;

    TBCExtraInfo(const TBCExtraInfo&);
    TBCExtraInfo& operator = ( const TBCExtraInfo&);
public:
    TBCExtraInfo();
    ~TBCExtraInfo(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    OUString getOnAction();
};

class MSFILTER_DLLPUBLIC TBCGeneralInfo  : public TBBase
{
    sal_uInt8 bFlags;
    WString customText;
    WString descriptionText;
    WString tooltip;
    TBCExtraInfo extraInfo;

public:
    TBCGeneralInfo();
    ~TBCGeneralInfo() {}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    bool ImportToolBarControlData( CustomToolBarImportHelper&, std::vector< css::beans::PropertyValue >& );
    OUString CustomText() { return customText.getString(); }
    OUString DescriptionText() { return descriptionText.getString(); }
    OUString Tooltip() { return tooltip.getString(); }
};

class MSFILTER_DLLPUBLIC TBCBitMap : public TBBase
{
friend class TBCBSpecific; // #FIXME hacky access, need to fix
    sal_Int32 cbDIB;
    Bitmap mBitMap;
public:
    TBCBitMap();
    ~TBCBitMap();
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    Bitmap& getBitMap();
};

class MSFILTER_DLLPUBLIC TBCMenuSpecific : public TBBase
{
    sal_Int32 tbid;
    boost::shared_ptr< WString > name; //exist only if tbid equals 0x00000001
public:
    TBCMenuSpecific();
    ~TBCMenuSpecific(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    OUString Name();
};

class MSFILTER_DLLPUBLIC TBCCDData : public TBBase
{
    sal_Int16 cwstrItems; //Signed integer that specifies the number of items in wstrList. MUST be positive.
    std::vector< WString > wstrList;  // Zero-based index array of WString structures. Number of elements MUST be equal to cwstrItems.
    sal_Int16 cwstrMRU; // Signed integer that specifies the number of most recently used string
    sal_Int16 iSel ; // Signed integer that specifies the zero-based index of the selected item in the wstrList field. MUST be equal to 0xFFFF (-1) or greater than or equal to 0x0000.
    sal_Int16 cLines; // Signed integer that specifies the suggested number of lines that the toolbar control will display at any time when displaying the elements of wstrList of available items.
    sal_Int16 dxWidth; // Signed integer that specifies the width in pixels that the interior of the dropdown has. This excludes the width of the toolbar control border and scroll bar.
    WString wstrEdit; //Structure of type WString. Editable text for editable area of the ComboBox toolbar control.

public:
    TBCCDData();
    ~TBCCDData();
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
};

class TBCComboDropdownSpecific : public TBBase
{
    boost::shared_ptr< TBCCDData > data;
public:
    TBCComboDropdownSpecific( const TBCHeader& header );
    TBCComboDropdownSpecific(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
};

class TBCBSpecific :  public TBBase
{
    sal_uInt8 bFlags;
    boost::shared_ptr< TBCBitMap > icon; // optional
    boost::shared_ptr< TBCBitMap > iconMask; // optional
    boost::shared_ptr< sal_uInt16 > iBtnFace; // optional
    boost::shared_ptr< WString > wstrAcc; // optional

public:
    TBCBSpecific();
    ~TBCBSpecific(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    // #TODO just add a getGraphic member here
    TBCBitMap* getIcon();
    TBCBitMap* getIconMask();
    sal_uInt16* getBtnFace() { return iBtnFace.get(); }
};

/* TBCHeader.tct                   controlSpecificInfo type

0x01 (Button control)              TBCBSpecific
0x10 (ExpandingGrid control)       TBCBSpecific
0x0A (Popup control)               TBCMenuSpecific
0x0C (ButtonPopup control)         TBCMenuSpecific
0x0D (SplitButtonPopup control)    TBCMenuSpecific
0x0E (SplitButtonMRUPopup control) TBCMenuSpecific
0x02 (Edit control)                TBCComboDropdow nSpecific
0x04 (ComboBox control)            TBCComboDropdow nSpecific
0x14 (GraphicCombo control)        TBCComboDropdow nSpecific
0x03 (DropDown control)            TBCComboDropdow nSpecific
0x06 (SplitDropDown control)       TBCComboDropdow nSpecific
0x09 (GraphicDropDown control)     TBCComboDropdow nSpecific
0x07 (OCXDropDown control)         controlSpecificInfo MUST NOT exist
0x0F (Label control)               controlSpecificInfo MUST NOT exist
0x12 (Grid control)                controlSpecificInfo MUST NOT exist
0x13 (Gauge control)               controlSpecificInfo MUST NOT exist
0x16 (ActiveX control)             controlSpecificInfo MUST NOT exist

*/
class MSFILTER_DLLPUBLIC TBCHeader : public TBBase
{
    sal_Int8 bSignature;
    sal_Int8 bVersion;
    sal_uInt8 bFlagsTCR;
    sal_uInt8 tct;
    sal_uInt16 tcid;
    sal_uInt32 tbct;
    sal_uInt8 bPriority;
    boost::shared_ptr< sal_uInt16 > width;  //optional
    boost::shared_ptr< sal_uInt16 > height; //optional

public:
    TBCHeader();
    ~TBCHeader();
    sal_uInt8 getTct() const { return tct; }
    sal_uInt16 getTcID() const { return tcid; }
    bool isVisible() { return !( bFlagsTCR & 0x1 ); }
    bool isBeginGroup() { return ( bFlagsTCR & 0x2 ); }
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    sal_uInt32 getTbct() { return tbct; };
};

class MSFILTER_DLLPUBLIC TBCData : public TBBase
{
    TBCHeader rHeader;
    TBCGeneralInfo controlGeneralInfo;
    boost::shared_ptr< TBBase > controlSpecificInfo; // can be one of TBCBSpecific, TBCMenuSpecific or TBCComboDropdow nSpecific depending on the control type specified by TBCHeader.tct
    TBCData(const TBCData&);
    TBCData& operator = ( const TBCData&);
public:
    TBCData( const TBCHeader& Header );
    ~TBCData(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    bool ImportToolBarControl( CustomToolBarImportHelper&, std::vector< css::beans::PropertyValue >&, bool& bBeginGroup, bool bIsMenuBar );
    TBCGeneralInfo& getGeneralInfo() { return controlGeneralInfo; }
    TBCMenuSpecific* getMenuSpecific();
};

class MSFILTER_DLLPUBLIC TB : public TBBase
{
    sal_uInt8 bSignature;// Signed integer that specifies the toolbar signature number. MUST be 0x02.
    sal_uInt8 bVersion; // Signed integer that specifies the toolbar version number. MUST be 0x01.
    sal_Int16 cCL; // Signed integer that SHOULD  specify the number of toolbar controls contained in this toolbar.
    sal_Int32 ltbid;// Signed integer that specifies the toolbar ID. MUST be 0x0001 (custom toolbar ID).
    sal_uInt32 ltbtr;// Unsigned integer of type TBTRFlags that specifies the toolbar type and toolbar restrictions.
    sal_uInt16 cRowsDefault;// Unsigned integer that specifies the number of preferred rows for the toolbar when the toolbar is not docked. MUST be less than or equal to 255.
    sal_uInt16 bFlags; //Unsigned integer of type TBFlags.
    WString name; //Structure of type WString that specifies the toolbar name
public:
    TB();
    ~TB(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
    sal_Int16 getcCL(){ return cCL; }
    WString& getName(){ return name; }
    bool IsEnabled();
    bool IsMenuToolbar(){ return ( ( ltbtr & 0x2000000 ) == 0x2000000 ); }
};

class MSFILTER_DLLPUBLIC SRECT : public TBBase
{
public:
    SRECT() : left(0), top(0), right(0), bottom(0) {}
    sal_Int16 left;
    sal_Int16 top;
    sal_Int16 right;
    sal_Int16 bottom;
    bool Read( SvStream &rS ) SAL_OVERRIDE { rS.ReadInt16( left ).ReadInt16( top ).ReadInt16( right ).ReadInt16( bottom ); return true; }
    void Print( FILE* fo ) SAL_OVERRIDE;
};

typedef cppu::WeakImplHelper1< css::container::XIndexContainer > PropertyValueIndexContainer_BASE;

class MSFILTER_DLLPUBLIC TBVisualData : public TBBase
{
    sal_Int8 tbds;
    sal_Int8 tbv;
    sal_Int8 tbdsDock;
    sal_Int8 iRow;

    SRECT rcDock;
    SRECT rcFloat;

public:
    TBVisualData();
    ~TBVisualData(){}
    bool Read(SvStream &rS) SAL_OVERRIDE;
    void Print( FILE* ) SAL_OVERRIDE;
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
