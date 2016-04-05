/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_OOX_OLE_AXCONTROL_HXX
#define INCLUDED_OOX_OLE_AXCONTROL_HXX

#include <oox/helper/binarystreambase.hxx>
#include <oox/helper/propertyset.hxx>
#include <oox/ole/axbinaryreader.hxx>
#include <oox/ole/olehelper.hxx>
#include <oox/dllapi.h>
#include <memory>

namespace com { namespace sun { namespace star {
    namespace awt { class XControlModel; }
    namespace container { class XIndexContainer; }
    namespace drawing { class XDrawPage; }
    namespace frame { class XModel; }
    namespace form { class XFormsSupplier; }
    namespace lang { class XMultiServiceFactory; }
} } }

namespace oox {
    class BinaryInputStream;
    class GraphicHelper;
    class PropertyMap;
}

namespace oox {
namespace ole {


#define COMCTL_GUID_SCROLLBAR_60   "{FE38753A-44A3-11D1-B5B7-0000C09000C4}"
#define COMCTL_GUID_PROGRESSBAR_50 "{0713E8D2-850A-101B-AFC0-4210102A8DA7}"
#define COMCTL_GUID_PROGRESSBAR_60 "{35053A22-8589-11D1-B16A-00C0F0283628}"

const sal_uInt16 COMCTL_VERSION_50          = 5;
const sal_uInt16 COMCTL_VERSION_60          = 6;


#define AX_GUID_COMMANDBUTTON "{D7053240-CE69-11CD-a777-00dd01143c57}"
#define AX_GUID_LABEL         "{978C9E23-D4B0-11CE-bf2d-00aa003f40d0}"
#define AX_GUID_IMAGE         "{4C599241-6926-101B-9992-00000b65c6f9}"
#define AX_GUID_TOGGLEBUTTON  "{8BD21D60-EC42-11CE-9e0d-00aa006002f3}"
#define AX_GUID_CHECKBOX      "{8BD21D40-EC42-11CE-9e0d-00aa006002f3}"
#define AX_GUID_OPTIONBUTTON  "{8BD21D50-EC42-11CE-9e0d-00aa006002f3}"
#define AX_GUID_TEXTBOX       "{8BD21D10-EC42-11CE-9e0d-00aa006002f3}"
#define AX_GUID_LISTBOX       "{8BD21D20-EC42-11CE-9e0d-00aa006002f3}"
#define AX_GUID_COMBOBOX      "{8BD21D30-EC42-11CE-9e0d-00aa006002f3}"
#define AX_GUID_SPINBUTTON    "{79176FB0-B7F2-11CE-97ef-00aa006d2776}"
#define AX_GUID_SCROLLBAR     "{DFD181E0-5E2F-11CE-a449-00aa004a803d}"
#define AX_GUID_FRAME         "{6E182020-F460-11CE-9bcd-00aa00608e01}"

// Html control GUID(s)

#define HTML_GUID_SELECT      "{5512D122-5CC6-11CF-8d67-00aa00bdce1d}"
#define HTML_GUID_TEXTBOX     "{5512D124-5CC6-11CF-8d67-00aa00bdce1d}"

const sal_uInt32 AX_SYSCOLOR_WINDOWBACK     = 0x80000005;
const sal_uInt32 AX_SYSCOLOR_WINDOWFRAME    = 0x80000006;
const sal_uInt32 AX_SYSCOLOR_WINDOWTEXT     = 0x80000008;
const sal_uInt32 AX_SYSCOLOR_BUTTONFACE     = 0x8000000F;
const sal_uInt32 AX_SYSCOLOR_BUTTONTEXT     = 0x80000012;

const sal_uInt32 AX_FLAGS_ENABLED           = 0x00000002;
const sal_uInt32 AX_FLAGS_LOCKED            = 0x00000004;
const sal_uInt32 AX_FLAGS_OPAQUE            = 0x00000008;
const sal_uInt32 AX_FLAGS_COLUMNHEADS       = 0x00000400;
const sal_uInt32 AX_FLAGS_ENTIREROWS        = 0x00000800;
const sal_uInt32 AX_FLAGS_EXISTINGENTRIES   = 0x00001000;
const sal_uInt32 AX_FLAGS_CAPTIONLEFT       = 0x00002000;
const sal_uInt32 AX_FLAGS_EDITABLE          = 0x00004000;
const sal_uInt32 AX_FLAGS_IMEMODE_MASK      = 0x00078000;
const sal_uInt32 AX_FLAGS_DRAGENABLED       = 0x00080000;
const sal_uInt32 AX_FLAGS_ENTERASNEWLINE    = 0x00100000;
const sal_uInt32 AX_FLAGS_KEEPSELECTION     = 0x00200000;
const sal_uInt32 AX_FLAGS_TABASCHARACTER    = 0x00400000;
const sal_uInt32 AX_FLAGS_WORDWRAP          = 0x00800000;
const sal_uInt32 AX_FLAGS_BORDERSSUPPRESSED = 0x02000000;
const sal_uInt32 AX_FLAGS_SELECTLINE        = 0x04000000;
const sal_uInt32 AX_FLAGS_SINGLECHARSELECT  = 0x08000000;
const sal_uInt32 AX_FLAGS_AUTOSIZE          = 0x10000000;
const sal_uInt32 AX_FLAGS_HIDESELECTION     = 0x20000000;
const sal_uInt32 AX_FLAGS_MAXLENAUTOTAB     = 0x40000000;
const sal_uInt32 AX_FLAGS_MULTILINE         = 0x80000000;

const sal_Int32 AX_BORDERSTYLE_NONE         = 0;
const sal_Int32 AX_BORDERSTYLE_SINGLE       = 1;

const sal_Int32 AX_SPECIALEFFECT_FLAT       = 0;
const sal_Int32 AX_SPECIALEFFECT_RAISED     = 1;
const sal_Int32 AX_SPECIALEFFECT_SUNKEN     = 2;
const sal_Int32 AX_SPECIALEFFECT_ETCHED     = 3;
const sal_Int32 AX_SPECIALEFFECT_BUMPED     = 6;

const sal_Int32 AX_PICSIZE_CLIP             = 0;
const sal_Int32 AX_PICSIZE_STRETCH          = 1;
const sal_Int32 AX_PICSIZE_ZOOM             = 3;

const sal_Int32 AX_PICALIGN_TOPLEFT         = 0;
const sal_Int32 AX_PICALIGN_TOPRIGHT        = 1;
const sal_Int32 AX_PICALIGN_CENTER          = 2;
const sal_Int32 AX_PICALIGN_BOTTOMLEFT      = 3;
const sal_Int32 AX_PICALIGN_BOTTOMRIGHT     = 4;

const sal_Int32 AX_DISPLAYSTYLE_TEXT        = 1;
const sal_Int32 AX_DISPLAYSTYLE_LISTBOX     = 2;
const sal_Int32 AX_DISPLAYSTYLE_COMBOBOX    = 3;
const sal_Int32 AX_DISPLAYSTYLE_CHECKBOX    = 4;
const sal_Int32 AX_DISPLAYSTYLE_OPTBUTTON   = 5;
const sal_Int32 AX_DISPLAYSTYLE_TOGGLE      = 6;
const sal_Int32 AX_DISPLAYSTYLE_DROPDOWN    = 7;

const sal_Int32 AX_SELECTION_SINGLE         = 0;
const sal_Int32 AX_SELECTION_MULTI          = 1;
const sal_Int32 AX_SELECTION_EXTENDED       = 2;

const sal_Int32 AX_SHOWDROPBUTTON_NEVER     = 0;
const sal_Int32 AX_SHOWDROPBUTTON_FOCUS     = 1;
const sal_Int32 AX_SHOWDROPBUTTON_ALWAYS    = 2;

const sal_Int32 AX_SCROLLBAR_NONE           = 0x00;
const sal_Int32 AX_SCROLLBAR_HORIZONTAL     = 0x01;
const sal_Int32 AX_SCROLLBAR_VERTICAL       = 0x02;


/** Enumerates all UNO API control types supported by these filters. */
enum ApiControlType
{
    API_CONTROL_BUTTON,
    API_CONTROL_FIXEDTEXT,
    API_CONTROL_IMAGE,
    API_CONTROL_CHECKBOX,
    API_CONTROL_RADIOBUTTON,
    API_CONTROL_EDIT,
    API_CONTROL_NUMERIC,
    API_CONTROL_LISTBOX,
    API_CONTROL_COMBOBOX,
    API_CONTROL_SPINBUTTON,
    API_CONTROL_SCROLLBAR,
    API_CONTROL_TABSTRIP, //11
    API_CONTROL_PROGRESSBAR,
    API_CONTROL_GROUPBOX,
    API_CONTROL_FRAME, // 14
    API_CONTROL_PAGE,  // 15
    API_CONTROL_MULTIPAGE, // 16
    API_CONTROL_DIALOG // 17
};


/** Specifies how a form control supports transparent background. */
enum ApiTransparencyMode
{
    API_TRANSPARENCY_NOTSUPPORTED,      ///< Control does not support transparency.
    API_TRANSPARENCY_VOID,              ///< Transparency is enabled by missing fill color.
    API_TRANSPARENCY_PAINTTRANSPARENT   ///< Transparency is enabled by the 'PaintTransparent' property.
};

/** Specifies how a form control supports the DefaultState property. */
enum ApiDefaultStateMode
{
    API_DEFAULTSTATE_BOOLEAN,           ///< Control does not support tri-state, state is given as boolean.
    API_DEFAULTSTATE_SHORT,             ///< Control does not support tri-state, state is given as short.
    API_DEFAULTSTATE_TRISTATE           ///< Control supports tri-state, state is given as short.
};


/** A base class with useful helper functions for something that is able to
    convert ActiveX and ComCtl form controls.
 */
class OOX_DLLPUBLIC ControlConverter
{
public:
    explicit            ControlConverter(
                            const css::uno::Reference< css::frame::XModel >& rxDocModel,
                            const GraphicHelper& rGraphicHelper,
                            bool bDefaultColorBgr = true );
    virtual             ~ControlConverter();

    // Generic conversion -----------------------------------------------------

    /** Converts the passed position in 1/100 mm to UNO properties. */
    void                convertPosition(
                            PropertyMap& rPropMap,
                            const AxPairData& rPos ) const;

    /** Converts the passed size in 1/100 mm to UNO properties. */
    void                convertSize(
                            PropertyMap& rPropMap,
                            const AxPairData& rSize ) const;

    /** Converts the passed encoded OLE color to UNO properties. */
    void                convertColor(
                            PropertyMap& rPropMap,
                            sal_Int32 nPropId,
                            sal_uInt32 nOleColor ) const;

    static void         convertToMSColor(
                            PropertySet& rPropSet,
                            sal_Int32 nPropId,
                            sal_uInt32& nOleColor,
                            sal_uInt32 nDefault = 0 );


    /** Converts the passed StdPic picture stream to UNO properties. */
    void                convertPicture(
                            PropertyMap& rPropMap,
                            const StreamDataSequence& rPicData ) const;

    /** Converts the control orientation to UNO properties. */
    static void         convertOrientation(
                            PropertyMap& rPropMap,
                            bool bHorizontal );

    static void         convertToMSOrientation(
                            PropertySet& rPropMap,
                            bool& bHorizontal );

    /** Converts the vertical alignment to UNO properties. */
    static void         convertVerticalAlign(
                            PropertyMap& rPropMap,
                            sal_Int32 nVerticalAlign );

    /** Converts common scrollbar settings to UNO properties. */
    static void         convertScrollBar(
                            PropertyMap& rPropMap,
                            sal_Int32 nMin, sal_Int32 nMax, sal_Int32 nPosition,
                            sal_Int32 nSmallChange, sal_Int32 nLargeChange, bool bAwtModel );

    /** Converts scrollability settings to UNO properties. */
    void                convertScrollabilitySettings(
                            PropertyMap& rPropMap,
                            const AxPairData& rScrollPos, const AxPairData& rScrollArea,
                            sal_Int32 nScrollBars ) const;

    /** Binds the passed control model to the passed data sources. The
        implementation will check which source types are supported. */
    void                bindToSources(
                            const css::uno::Reference< css::awt::XControlModel >& rxCtrlModel,
                            const OUString& rCtrlSource,
                            const OUString& rRowSource,
                            sal_Int32 nRefSheet = 0 ) const;

    // ActiveX (Forms 2.0) specific conversion --------------------------------

    /** Converts the Forms 2.0 background formatting to UNO properties. */
    void                convertAxBackground(
                            PropertyMap& rPropMap,
                            sal_uInt32 nBackColor,
                            sal_uInt32 nFlags,
                            ApiTransparencyMode eTranspMode ) const;

    /** Converts the Forms 2.0 border formatting to UNO properties. */
    void                convertAxBorder(
                            PropertyMap& rPropMap,
                            sal_uInt32 nBorderColor,
                            sal_Int32 nBorderStyle,
                            sal_Int32 nSpecialEffect ) const;

    static void        convertToAxBorder(
                            PropertySet& rPropSet,
                            sal_uInt32& nBorderColor,
                            sal_Int32& nBorderStyle,
                            sal_Int32& nSpecialEffect );

    /** Converts the Forms 2.0 special effect to UNO properties. */
    static void         convertAxVisualEffect(
                            PropertyMap& rPropMap,
                            sal_Int32 nSpecialEffect );

    static void         convertToAxVisualEffect(
                            PropertySet& rPropSet,
                            sal_Int32& nSpecialEffect );

    /** Converts the passed picture stream and Forms 2.0 position to UNO
        properties. */
    void                convertAxPicture(
                            PropertyMap& rPropMap,
                            const StreamDataSequence& rPicData,
                            sal_uInt32 nPicPos ) const;

    /** Converts the passed picture stream and Forms 2.0 position to UNO
        properties. */
    void                convertAxPicture(
                            PropertyMap& rPropMap,
                            const StreamDataSequence& rPicData,
                            sal_Int32 nPicSizeMode,
                            sal_Int32 nPicAlign,
                            bool bPicTiling ) const;

    /** Converts the Forms 2.0 value for checked/unchecked/dontknow to UNO
        properties. */
    static void         convertAxState(
                            PropertyMap& rPropMap,
                            const OUString& rValue,
                            sal_Int32 nMultiSelect,
                            ApiDefaultStateMode eDefStateMode,
                            bool bAwtModel );

    static void        convertToAxState(
                            PropertySet& rPropSet,
                            OUString& rValue,
                            sal_Int32& nMultiSelect,
                            ApiDefaultStateMode eDefStateMode,
                            bool bAwtModel );

    /** Converts the Forms 2.0 control orientation to UNO properties. */
    static void        convertAxOrientation(
                            PropertyMap& rPropMap,
                            const AxPairData& rSize,
                            sal_Int32 nOrientation );

    static void        convertToAxOrientation(
                            PropertySet& rPropSet,
                            const AxPairData& rSize,
                            sal_Int32& nOrientation );

private:
    css::uno::Reference< css::frame::XModel > mxDocModel;
    const GraphicHelper& mrGraphicHelper;
    mutable PropertySet maAddressConverter;
    mutable PropertySet maRangeConverter;
    bool                mbDefaultColorBgr;
};


/** Base class for all models of form controls. */
class OOX_DLLPUBLIC ControlModelBase
{
public:
    explicit            ControlModelBase();
    virtual             ~ControlModelBase();

    /** Sets this control model to AWT model mode. */
    void         setAwtModelMode() { mbAwtModel = true; }
    /** Sets this control model to form component mode. */
    void         setFormComponentMode() { mbAwtModel = false; }

    /** Returns the UNO service name used to construct the AWT control model,
        or the control form component. */
    OUString     getServiceName() const;

    /** Derived classes set specific OOXML properties at the model structure. */
    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue );
    /** Derived classes set binary data (picture, mouse icon) at the model structure. */
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm );
    /** Derived classes import a form control model from the passed input stream. */
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) = 0;
    /** Derived classes export a form control model to the passed output stream. */
    virtual void        exportBinaryModel( BinaryOutputStream& /*rOutStrm*/ ) {}
    /** Derived classes export CompObjStream contents. */
    virtual void        exportCompObj( BinaryOutputStream& /*rOutStrm*/ ) {}
    /** Derived classes return the UNO control type enum value. */
    virtual ApiControlType getControlType() const = 0;
    /** Derived classes convert all control properties. */
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
    /** Derived classes convert from uno control properties to equiv. MS values. */
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv );

    /** Converts the control size to UNO properties. */
    void                convertSize( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

public: // direct access needed for legacy VML drawing controls
    AxPairData          maSize;         ///< Size of the control in 1/100 mm.

protected:
    bool                mbAwtModel;     ///< True = AWT control model, false = form component.
};

typedef std::shared_ptr< ControlModelBase > ControlModelRef;


/** Base class for all models of ComCtl form controls. */
class ComCtlModelBase : public ControlModelBase
{
public:
    explicit            ComCtlModelBase(
                            sal_uInt32 nDataPartId5, sal_uInt32 nDataPartId6, sal_uInt16 nVersion,
                            bool bCommonPart, bool bComplexPart );

    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;

protected:
    virtual void        importControlData( BinaryInputStream& rInStrm ) = 0;

private:
    /** Returns the data part identifier according to the model version. */
    sal_uInt32          getDataPartId() const;

    static bool         readPartHeader( BinaryInputStream& rInStrm,
                            sal_uInt32 nExpPartId,
                            sal_uInt16 nExpMajor = SAL_MAX_UINT16,
                            sal_uInt16 nExpMinor = SAL_MAX_UINT16 );

    bool                importSizePart( BinaryInputStream& rInStrm );
    bool                importCommonPart( BinaryInputStream& rInStrm, sal_uInt32 nPartSize );
    bool                importComplexPart( BinaryInputStream& rInStrm );

protected:
    StdFontInfo         maFontData;         ///< Font formatting.
    StreamDataSequence  maMouseIcon;        ///< Binary picture stream for mouse icon.
    sal_uInt32          mnFlags;            ///< Common flags for ComCtl controls.
    const sal_uInt16    mnVersion;          ///< Current version of the ComCtl control model.

private:
    sal_uInt32          mnDataPartId5;      ///< Identifier for version 5.0 control data.
    sal_uInt32          mnDataPartId6;      ///< Identifier for version 6.0 control data.
    bool                mbCommonPart;       ///< True = the COMCTL_COMMONDATA part exists.
    bool                mbComplexPart;      ///< True = the COMCTL_COMPLEXDATA part exists.
};


/** Model for a ComCtl scroll bar. */
class ComCtlScrollBarModel : public ComCtlModelBase
{
public:
    explicit            ComCtlScrollBarModel( sal_uInt16 nVersion );

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;

protected:
    virtual void        importControlData( BinaryInputStream& rInStrm ) override;

private:
    sal_uInt32          mnScrollBarFlags;   ///< Special flags for scroll bar model.
    sal_Int32           mnLargeChange;      ///< Increment step size (thumb).
    sal_Int32           mnSmallChange;      ///< Increment step size (buttons).
    sal_Int32           mnMin;              ///< Minimum of the value range.
    sal_Int32           mnMax;              ///< Maximum of the value range.
    sal_Int32           mnPosition;         ///< Value of the spin button.
};


/** Model for a ComCtl progress bar. */
class ComCtlProgressBarModel : public ComCtlModelBase
{
public:
    explicit            ComCtlProgressBarModel( sal_uInt16 nVersion );

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;

protected:
    virtual void        importControlData( BinaryInputStream& rInStrm ) override;

private:
    float               mfMin;              ///< Minimum of the value range.
    float               mfMax;              ///< Maximum of the value range.
    sal_uInt16          mnVertical;         ///< 0 = horizontal, 1 = vertical.
    sal_uInt16          mnSmooth;           ///< 0 = progress blocks, 1 = pixel resolution.
};


/** Base class for all models of Form 2.0 form controls. */
class OOX_DLLPUBLIC AxControlModelBase : public ControlModelBase
{
public:
    explicit            AxControlModelBase();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
};


/** Base class for Forms 2.0 controls supporting text formatting. */
class OOX_DLLPUBLIC AxFontDataModel : public AxControlModelBase
{
public:
    explicit            AxFontDataModel( bool bSupportsAlign = true );

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;

    /** Returns the font height in points. */
    sal_Int16           getFontHeight() const { return maFontData.getHeightPoints(); }

public: // direct access needed for legacy VML drawing controls
    AxFontData          maFontData;         ///< The font settings.

private:
    bool                mbSupportsAlign;    ///< True = UNO model supports Align property.
};


/** Model for a Forms 2.0 command button. */
class OOX_DLLPUBLIC AxCommandButtonModel : public AxFontDataModel
{
public:
    explicit            AxCommandButtonModel();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;

public: // direct access needed for legacy VML drawing controls
    StreamDataSequence  maPictureData;      ///< Binary picture stream.
    OUString     maCaption;          ///< Visible caption of the button.
    sal_uInt32          mnTextColor;        ///< Text color.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_uInt32          mnPicturePos;       ///< Position of the picture relative to text.
    sal_Int32           mnVerticalAlign;    ///< Vertical alignment (legacy VML drawing controls only).
    bool                mbFocusOnClick;     ///< True = take focus on click.
};


/** Model for a Forms 2.0 label. */
class OOX_DLLPUBLIC AxLabelModel : public AxFontDataModel
{
public:
    explicit            AxLabelModel();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;

public: // direct access needed for legacy VML drawing controls
    OUString     maCaption;          ///< Visible caption of the button.
    sal_uInt32          mnTextColor;        ///< Text color.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_uInt32          mnBorderColor;      ///< Flat border color.
    sal_Int32           mnBorderStyle;      ///< Flat border style.
    sal_Int32           mnSpecialEffect;    ///< 3D border effect.
    sal_Int32           mnVerticalAlign;    ///< Vertical alignment (legacy VML drawing controls only).
};


/** Model for a Forms 2.0 image. */
class OOX_DLLPUBLIC AxImageModel : public AxControlModelBase
{
public:
    explicit            AxImageModel();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;

private:
    StreamDataSequence  maPictureData;      ///< Binary picture stream.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_uInt32          mnBorderColor;      ///< Flat border color.
    sal_Int32           mnBorderStyle;      ///< Flat border style.
    sal_Int32           mnSpecialEffect;    ///< 3D border effect.
    sal_Int32           mnPicSizeMode;      ///< Clip, stretch, zoom.
    sal_Int32           mnPicAlign;         ///< Anchor position of the picture.
    bool                mbPicTiling;        ///< True = picture is repeated.
};

class OOX_DLLPUBLIC AxTabStripModel : public AxFontDataModel
{
public:
    explicit            AxTabStripModel();

    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;

    virtual ApiControlType getControlType() const override;

public:
    sal_uInt32   mnListIndex;
    sal_uInt32   mnTabStyle;
    sal_uInt32   mnTabData;
    sal_uInt32   mnVariousPropertyBits;
    std::vector< ::rtl::OUString > maItems; // captions for each tab
    std::vector< ::rtl::OUString > maTabNames; // names for each tab
};


/** Base class for a Forms 2.0 morph data control. */
class OOX_DLLPUBLIC AxMorphDataModelBase : public AxFontDataModel
{
public:
    explicit            AxMorphDataModelBase();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;

public: // direct access needed for legacy VML drawing controls
    StreamDataSequence  maPictureData;      ///< Binary picture stream.
    OUString     maCaption;          ///< Visible caption of the button.
    OUString     maValue;            ///< Current value of the control.
    OUString     maGroupName;        ///< Group name for option buttons.
    sal_uInt32          mnTextColor;        ///< Text color.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_uInt32          mnPicturePos;       ///< Position of the picture relative to text.
    sal_uInt32          mnBorderColor;      ///< Flat border color.
    sal_Int32           mnBorderStyle;      ///< Flat border style.
    sal_Int32           mnSpecialEffect;    ///< 3D border effect.
    sal_Int32           mnDisplayStyle;     ///< Type of the morph control.
    sal_Int32           mnMultiSelect;      ///< Selection mode.
    sal_Int32           mnScrollBars;       ///< Horizontal/vertical scroll bar.
    sal_Int32           mnMatchEntry;       ///< Auto completion mode.
    sal_Int32           mnShowDropButton;   ///< When to show the dropdown button.
    sal_Int32           mnMaxLength;        ///< Maximum character count.
    sal_Int32           mnPasswordChar;     ///< Password character in edit fields.
    sal_Int32           mnListRows;         ///< Number of rows in dropdown box.
    sal_Int32           mnVerticalAlign;    ///< Vertical alignment (legacy VML drawing controls only).
};


/** Model for a Forms 2.0 toggle button. */
class OOX_DLLPUBLIC AxToggleButtonModel : public AxMorphDataModelBase
{
public:
    explicit            AxToggleButtonModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a Forms 2.0 check box. */
class OOX_DLLPUBLIC AxCheckBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxCheckBoxModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a Forms 2.0 option button. */
class OOX_DLLPUBLIC AxOptionButtonModel : public AxMorphDataModelBase
{
public:
    explicit            AxOptionButtonModel();

    /** Returns the group name used to goup several option buttons together. */
    const OUString& getGroupName() const { return maGroupName; }

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a Forms 2.0 text box. */
class OOX_DLLPUBLIC AxTextBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxTextBoxModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a numeric field (legacy drawing controls only). */
class OOX_DLLPUBLIC AxNumericFieldModel : public AxMorphDataModelBase
{
public:
    explicit            AxNumericFieldModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a Forms 2.0 list box. */
class OOX_DLLPUBLIC AxListBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxListBoxModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a Forms 2.0 combo box. */
class OOX_DLLPUBLIC AxComboBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxComboBoxModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;
};


/** Model for a Forms 2.0 spin button. */
class OOX_DLLPUBLIC AxSpinButtonModel : public AxControlModelBase
{
public:
    explicit            AxSpinButtonModel();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;

public: // direct access needed for legacy VML drawing controls
    sal_uInt32          mnArrowColor;       ///< Button arrow color.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_Int32           mnOrientation;      ///< Orientation of the buttons.
    sal_Int32           mnMin;              ///< Minimum of the value range.
    sal_Int32           mnMax;              ///< Maximum of the value range.
    sal_Int32           mnPosition;         ///< Value of the spin button.
    sal_Int32           mnSmallChange;      ///< Increment step size.
    sal_Int32           mnDelay;            ///< Repeat delay in milliseconds.
};


/** Model for a Forms 2.0 scroll bar. */
class OOX_DLLPUBLIC AxScrollBarModel : public AxControlModelBase
{
public:
    explicit            AxScrollBarModel();

    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        exportBinaryModel( BinaryOutputStream& rOutStrm ) override;
    virtual void        exportCompObj( BinaryOutputStream& rOutStrm ) override;

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    virtual void        convertFromProperties( PropertySet& rPropSet, const ControlConverter& rConv ) override;

public: // direct access needed for legacy VML drawing controls
    sal_uInt32          mnArrowColor;       ///< Button arrow color.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_Int32           mnOrientation;      ///< Orientation of the buttons.
    sal_Int32           mnPropThumb;        ///< Proportional thumb size.
    sal_Int32           mnMin;              ///< Minimum of the value range.
    sal_Int32           mnMax;              ///< Maximum of the value range.
    sal_Int32           mnPosition;         ///< Value of the spin button.
    sal_Int32           mnSmallChange;      ///< Increment step size (buttons).
    sal_Int32           mnLargeChange;      ///< Increment step size (thumb).
    sal_Int32           mnDelay;            ///< Repeat delay in milliseconds.
};


typedef ::std::vector< OUString > AxClassTable;

/** Base class for ActiveX container controls. */
class OOX_DLLPUBLIC AxContainerModelBase : public AxFontDataModel
{
public:
    explicit            AxContainerModelBase( bool bFontSupport = false );

    /** Allows to set single properties specified by XML token identifier. */
    virtual void        importProperty( sal_Int32 nPropId, const OUString& rValue ) override;
    /** Reads the leading structure in the 'f' stream containing the model for
        this control. */
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    /** Converts font settings if supported. */
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;

    /** Reads the class table structure for embedded controls following the own
        model from the 'f' stream. */
    bool                importClassTable( BinaryInputStream& rInStrm, AxClassTable& orClassTable );

public: // direct access needed for legacy VML drawing controls
    StreamDataSequence  maPictureData;      ///< Binary picture stream.
    OUString     maCaption;          ///< Visible caption of the form.
    AxPairData          maLogicalSize;      ///< Logical form size (scroll area).
    AxPairData          maScrollPos;        ///< Scroll position.
    sal_uInt32          mnBackColor;        ///< Fill color.
    sal_uInt32          mnTextColor;        ///< Text color.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_uInt32          mnBorderColor;      ///< Flat border color.
    sal_Int32           mnBorderStyle;      ///< Flat border style.
    sal_Int32           mnScrollBars;       ///< Horizontal/vertical scroll bar.
    sal_Int32           mnCycleType;        ///< Cycle in all forms or in this form.
    sal_Int32           mnSpecialEffect;    ///< 3D border effect.
    sal_Int32           mnPicAlign;         ///< Anchor position of the picture.
    sal_Int32           mnPicSizeMode;      ///< Clip, stretch, zoom.
    bool                mbPicTiling;        ///< True = picture is repeated.
    bool                mbFontSupport;      ///< True = control supports the font property.
};


/** Model for a Forms 2.0 frame control. */
class OOX_DLLPUBLIC AxFrameModel : public AxContainerModelBase
{
public:
    explicit            AxFrameModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
};

class OOX_DLLPUBLIC AxPageModel : public AxContainerModelBase
{
public:
    explicit            AxPageModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
};

class OOX_DLLPUBLIC AxMultiPageModel : public AxContainerModelBase
{
public:
    explicit            AxMultiPageModel();

    virtual ApiControlType getControlType() const override;
    void                importPageAndMultiPageProperties( BinaryInputStream& rInStrm, sal_Int32 nPages );
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
    std::vector<sal_uInt32> mnIDs;
    sal_uInt32          mnActiveTab;
    sal_uInt32          mnTabStyle;
};


/** Model for a Forms 2.0 user form. */
class OOX_DLLPUBLIC AxUserFormModel : public AxContainerModelBase
{
public:
    explicit            AxUserFormModel();

    virtual ApiControlType getControlType() const override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
};

class HtmlSelectModel : public AxListBoxModel
{
    css::uno::Sequence< OUString > msListData;
    css::uno::Sequence< sal_Int16 > msIndices;
public:
    HtmlSelectModel();
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const override;
};

class HtmlTextBoxModel : public AxTextBoxModel
{
public:
    explicit            HtmlTextBoxModel();
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) override;
};


/** A form control embedded in a document draw page. Contains a specific model
    structure according to the type of the control. */
class OOX_DLLPUBLIC EmbeddedControl
{
public:
    explicit            EmbeddedControl( const OUString& rName );
    virtual             ~EmbeddedControl();

    /** Creates and returns the internal control model of the specified type. */
    template< typename ModelType >
    inline ModelType&   createModel();

    /** Creates and returns the internal control model of the specified type. */
    template< typename ModelType, typename ParamType >
    inline ModelType&   createModel( const ParamType& rParam );

    /** Creates and returns the internal control model according to the passed
        MS class identifier. */
    ControlModelBase*   createModelFromGuid( const OUString& rClassId );

    /** Returns true, if the internal control model exists. */
    bool         hasModel() const { return mxModel.get() != nullptr; }
    /** Returns read-only access to the internal control model. */
    const ControlModelBase* getModel() const { return mxModel.get(); }
    /** Returns read/write access to the internal control model. */
    ControlModelBase* getModel() { return mxModel.get(); }

    /** Returns the UNO service name needed to construct the control model. */
    OUString     getServiceName() const;

    /** Converts all control properties and inserts them into the passed model. */
    bool                convertProperties(
                            const css::uno::Reference< css::awt::XControlModel >& rxCtrlModel,
                            const ControlConverter& rConv ) const;

    void                convertFromProperties(
                            const css::uno::Reference< css::awt::XControlModel >& rxCtrlModel,
                            const ControlConverter& rConv );

private:
    ControlModelRef     mxModel;            ///< Control model containing the properties.
    OUString     maName;             ///< Name of the control.
};


template< typename ModelType >
inline ModelType& EmbeddedControl::createModel()
{
    std::shared_ptr< ModelType > xModel( new ModelType );
    mxModel = xModel;
    xModel->setFormComponentMode();
    return *xModel;
}

template< typename ModelType, typename ParamType >
inline ModelType& EmbeddedControl::createModel( const ParamType& rParam )
{
    std::shared_ptr< ModelType > xModel( new ModelType( rParam ) );
    mxModel = xModel;
    xModel->setFormComponentMode();
    return *xModel;
}


/** A wrapper for a control form embedded directly in a draw page. */
class EmbeddedForm
{
public:
    explicit            EmbeddedForm(
                            const css::uno::Reference< css::frame::XModel >& rxDocModel,
                            const css::uno::Reference< css::drawing::XDrawPage >& rxDrawPage,
                            const GraphicHelper& rGraphicHelper,
                            bool bDefaultColorBgr = true );

    /** Converts the passed control and inserts the control model into the form.
        @return  The API control model, if conversion was successful. */
    css::uno::Reference< css::awt::XControlModel >
                        convertAndInsert( const EmbeddedControl& rControl, sal_Int32& rnCtrlIndex );

    /** Returns the XIndexContainer interface of the UNO control form, if existing. */
    const css::uno::Reference< css::container::XIndexContainer >&
                        getXForm() const { return mxFormIC; }

private:
    /** Creates the form that will hold the form controls. */
    css::uno::Reference< css::container::XIndexContainer >
                        createXForm();

private:
    ControlConverter                                       maControlConv;
    css::uno::Reference< css::lang::XMultiServiceFactory > mxModelFactory;
    css::uno::Reference< css::form::XFormsSupplier >       mxFormsSupp;
    css::uno::Reference< css::container::XIndexContainer > mxFormIC;
};


} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
