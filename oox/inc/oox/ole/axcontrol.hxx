/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef OOX_OLE_AXCONTROL_HXX
#define OOX_OLE_AXCONTROL_HXX

#include <boost/shared_ptr.hpp>
#include "oox/helper/binarystreambase.hxx"
#include "oox/ole/axbinaryreader.hxx"
#include "oox/ole/olehelper.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { class XControlModel; }
    namespace container { class XIndexContainer; }
    namespace drawing { class XDrawPage; }
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

// ============================================================================

const sal_Char* const COMCTL_GUID_SCROLLBAR_60      = "{FE38753A-44A3-11D1-B5B7-0000C09000C4}";
const sal_Char* const COMCTL_GUID_PROGRESSBAR_50    = "{0713E8D2-850A-101B-AFC0-4210102A8DA7}";
const sal_Char* const COMCTL_GUID_PROGRESSBAR_60    = "{35053A22-8589-11D1-B16A-00C0F0283628}";

// ----------------------------------------------------------------------------

const sal_Char* const AX_GUID_COMMANDBUTTON = "{D7053240-CE69-11CD-A777-00DD01143C57}";
const sal_Char* const AX_GUID_LABEL         = "{978C9E23-D4B0-11CE-BF2D-00AA003F40D0}";
const sal_Char* const AX_GUID_IMAGE         = "{4C599241-6926-101B-9992-00000B65C6F9}";
const sal_Char* const AX_GUID_TOGGLEBUTTON  = "{8BD21D60-EC42-11CE-9E0D-00AA006002F3}";
const sal_Char* const AX_GUID_CHECKBOX      = "{8BD21D40-EC42-11CE-9E0D-00AA006002F3}";
const sal_Char* const AX_GUID_OPTIONBUTTON  = "{8BD21D50-EC42-11CE-9E0D-00AA006002F3}";
const sal_Char* const AX_GUID_TEXTBOX       = "{8BD21D10-EC42-11CE-9E0D-00AA006002F3}";
const sal_Char* const AX_GUID_LISTBOX       = "{8BD21D20-EC42-11CE-9E0D-00AA006002F3}";
const sal_Char* const AX_GUID_COMBOBOX      = "{8BD21D30-EC42-11CE-9E0D-00AA006002F3}";
const sal_Char* const AX_GUID_SPINBUTTON    = "{79176FB0-B7F2-11CE-97EF-00AA006D2776}";
const sal_Char* const AX_GUID_SCROLLBAR     = "{DFD181E0-5E2F-11CE-A449-00AA004A803D}";
const sal_Char* const AX_GUID_FRAME         = "{6E182020-F460-11CE-9BCD-00AA00608E01}";

const sal_uInt32 AX_SYSCOLOR_WINDOWBACK     = 0x80000005;
const sal_uInt32 AX_SYSCOLOR_WINDOWFRAME    = 0x80000006;
const sal_uInt32 AX_SYSCOLOR_WINDOWTEXT     = 0x80000008;
const sal_uInt32 AX_SYSCOLOR_BUTTONFACE     = 0x8000000F;
const sal_uInt32 AX_SYSCOLOR_BUTTONTEXT     = 0x80000012;

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

// ----------------------------------------------------------------------------

/** Enumerates all UNO API control types supported by these filters. */
enum ApiControlType
{
    API_CONTROL_BUTTON,
    API_CONTROL_FIXEDTEXT,
    API_CONTROL_IMAGE,
    API_CONTROL_CHECKBOX,
    API_CONTROL_RADIOBUTTON,
    API_CONTROL_EDIT,
    API_CONTROL_LISTBOX,
    API_CONTROL_COMBOBOX,
    API_CONTROL_SPINBUTTON,
    API_CONTROL_SCROLLBAR,
    API_CONTROL_TABSTRIP,
    API_CONTROL_PROGRESSBAR,
    API_CONTROL_GROUPBOX,
    API_CONTROL_FRAME,
    API_CONTROL_PAGE,
    API_CONTROL_MULTIPAGE,
    API_CONTROL_DIALOG
};

// ============================================================================

/** Specifies how a form control supports transparent background. */
enum ApiTransparencyMode
{
    API_TRANSPARENCY_NOTSUPPORTED,      /// Control does not support transparency.
    API_TRANSPARENCY_VOID,              /// Transparency is enabled by missing fill color.
    API_TRANSPARENCY_PAINTTRANSPARENT   /// Transparency is enabled by the 'PaintTransparent' property.
};

/** Specifies how a form control supports the DefaultState property. */
enum ApiDefaultStateMode
{
    API_DEFAULTSTATE_BOOLEAN,           /// Control does not support tri-state, state is given as boolean.
    API_DEFAULTSTATE_SHORT,             /// Control does not support tri-state, state is given as short.
    API_DEFAULTSTATE_TRISTATE           /// Control supports tri-state, state is given as short.
};

// ----------------------------------------------------------------------------

/** A base class with useful helper functions for something that is able to
    convert ActiveX and ComCtl form controls.
 */
class ControlConverter
{
public:
    explicit            ControlConverter(
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

    /** Converts the passed StdPic picture stream to UNO properties. */
    void                convertPicture(
                            PropertyMap& rPropMap,
                            const StreamDataSequence& rPicData ) const;

    /** Converts the control orientation to UNO properties. */
    void                convertOrientation(
                            PropertyMap& rPropMap,
                            bool bHorizontal ) const;

    /** Converts common scrollbar settings to UNO properties. */
    void                convertScrollBar(
                            PropertyMap& rPropMap,
                            sal_Int32 nMin, sal_Int32 nMax, sal_Int32 nPosition,
                            sal_Int32 nSmallChange, sal_Int32 nLargeChange, bool bAwtModel ) const;

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

    /** Converts the Forms 2.0 special effect to UNO properties. */
    void                convertAxVisualEffect(
                            PropertyMap& rPropMap,
                            sal_Int32 nSpecialEffect ) const;

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
    void                convertAxState(
                            PropertyMap& rPropMap,
                            const ::rtl::OUString& rValue,
                            sal_Int32 nMultiSelect,
                            ApiDefaultStateMode eDefStateMode,
                            bool bAwtModel ) const;

    /** Converts the Forms 2.0 control orientation to UNO properties. */
    void                convertAxOrientation(
                            PropertyMap& rPropMap,
                            const AxPairData& rSize,
                            sal_Int32 nOrientation ) const;

private:
    const GraphicHelper& mrGraphicHelper;
    bool                mbDefaultColorBgr;
};

// ============================================================================

/** Base class for all models of form controls. */
class ControlModelBase
{
public:
    explicit            ControlModelBase();
    virtual             ~ControlModelBase();

    /** Sets this control model to AWT model mode. */
    inline void         setAwtModelMode() { mbAwtModel = true; }
    /** Sets this control model to form component mode. */
    inline void         setFormComponentMode() { mbAwtModel = false; }

    /** Returns the UNO service name used to construct the AWT control model,
        or the control form component. */
    ::rtl::OUString     getServiceName() const;

    /** Derived classes set specific OOXML properties at the model structure. */
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    /** Derived classes set binary data (picture, mouse icon) at the model structure. */
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm );
    /** Derived classes import a form control model from the passed input stream. */
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm ) = 0;

    /** Derived classes return the UNO control type enum value. */
    virtual ApiControlType getControlType() const = 0;
    /** Derived classes convert all control properties. */
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

    /** Converts the control size to UNO properties. */
    void                convertSize( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

protected:
    AxPairData          maSize;         /// Size of the control in 1/100 mm.
    bool                mbAwtModel;     /// True = AWT control model, false = form component.
};

typedef ::boost::shared_ptr< ControlModelBase > ControlModelRef;

// ============================================================================

/** Base class for all models of ComCtl form controls. */
class ComCtlModelBase : public ControlModelBase
{
public:
    explicit            ComCtlModelBase(
                            sal_uInt32 nDataPartId5, sal_uInt32 nDataPartId6, sal_uInt16 nVersion,
                            bool bCommonPart, bool bComplexPart );

    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

protected:
    virtual void        importControlData( BinaryInputStream& rInStrm ) = 0;
    virtual void        importCommonExtraData( BinaryInputStream& rInStrm );
    virtual void        importCommonTrailingData( BinaryInputStream& rInStrm );

private:
    /** Returns the data part identifier according to the model version. */
    sal_uInt32          getDataPartId() const;
    
    bool                readPartHeader( BinaryInputStream& rInStrm,
                            sal_uInt32 nExpPartId,
                            sal_uInt16 nExpMajor = SAL_MAX_UINT16,
                            sal_uInt16 nExpMinor = SAL_MAX_UINT16 );

    bool                importSizePart( BinaryInputStream& rInStrm );
    bool                importCommonPart( BinaryInputStream& rInStrm, sal_uInt32 nPartSize );
    bool                importComplexPart( BinaryInputStream& rInStrm );

protected:
    StdFontInfo         maFontData;         /// Font formatting.
    StreamDataSequence  maMouseIcon;        /// Binary picture stream for mouse icon.
    sal_uInt32          mnFlags;            /// Common flags for ComCtl controls.
    const sal_uInt16    mnVersion;          /// Current version of the ComCtl control model.

private:
    sal_uInt32          mnDataPartId5;      /// Identifier for version 5.0 control data.
    sal_uInt32          mnDataPartId6;      /// Identifier for version 6.0 control data.
    bool                mbCommonPart;       /// True = the COMCTL_COMMONDATA part exists.
    bool                mbComplexPart;      /// True = the COMCTL_COMPLEXDATA part exists.
};

// ============================================================================

/** Model for a ComCtl scroll bar. */
class ComCtlScrollBarModel : public ComCtlModelBase
{
public:
    explicit            ComCtlScrollBarModel( sal_uInt16 nVersion );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

protected:
    virtual void        importControlData( BinaryInputStream& rInStrm );

private:
    sal_uInt32          mnScrollBarFlags;   /// Special flags for scroll bar model.
    sal_Int32           mnLargeChange;      /// Increment step size (thumb).
    sal_Int32           mnSmallChange;      /// Increment step size (buttons).
    sal_Int32           mnMin;              /// Minimum of the value range.
    sal_Int32           mnMax;              /// Maximum of the value range.
    sal_Int32           mnPosition;         /// Value of the spin button.
};

// ============================================================================

/** Model for a ComCtl progress bar. */
class ComCtlProgressBarModel : public ComCtlModelBase
{
public:
    explicit            ComCtlProgressBarModel( sal_uInt16 nVersion );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

protected:
    virtual void        importControlData( BinaryInputStream& rInStrm );

private:
    float               mfMin;              /// Minimum of the value range.
    float               mfMax;              /// Maximum of the value range.
    sal_uInt16          mnVertical;         /// 0 = horizontal, 1 = vertical.
    sal_uInt16          mnSmooth;           /// 0 = progress blocks, 1 = pixel resolution.
};

// ============================================================================

/** Base class for all models of Form 2.0 form controls. */
class AxControlModelBase : public ControlModelBase
{
public:
    explicit            AxControlModelBase();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
};

// ============================================================================

/** Base class for Forms 2.0 controls supporting text formatting. */
class AxFontDataModel : public AxControlModelBase
{
public:
    explicit            AxFontDataModel( bool bSupportsAlign = true );

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

    /** Returns the font height in points. */
    inline sal_Int16    getFontHeight() const { return maFontData.getHeightPoints(); }

protected:
    AxFontData          maFontData;         /// The font settings.
    bool                mbSupportsAlign;    /// True = UNO model supports Align property.
};

// ============================================================================

/** Model for a Forms 2.0 command button. */
class AxCommandButtonModel : public AxFontDataModel
{
public:
    explicit            AxCommandButtonModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

private:
    StreamDataSequence  maPictureData;      /// Binary picture stream.
    ::rtl::OUString     maCaption;          /// Visible caption of the button.
    sal_uInt32          mnTextColor;        /// Text color.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_uInt32          mnPicturePos;       /// Position of the picture relative to text.
    bool                mbFocusOnClick;     /// True = take focus on click.
};

// ============================================================================

/** Model for a Forms 2.0 label. */
class AxLabelModel : public AxFontDataModel
{
public:
    explicit            AxLabelModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

private:
    ::rtl::OUString     maCaption;          /// Visible caption of the button.
    sal_uInt32          mnTextColor;        /// Text color.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_uInt32          mnBorderColor;      /// Flat border color.
    sal_Int32           mnBorderStyle;      /// Flat border style.
    sal_Int32           mnSpecialEffect;    /// 3D border effect.
};

// ============================================================================

/** Model for a Forms 2.0 image. */
class AxImageModel : public AxControlModelBase
{
public:
    explicit            AxImageModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

private:
    StreamDataSequence  maPictureData;      /// Binary picture stream.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_uInt32          mnBorderColor;      /// Flat border color.
    sal_Int32           mnBorderStyle;      /// Flat border style.
    sal_Int32           mnSpecialEffect;    /// 3D border effect.
    sal_Int32           mnPicSizeMode;      /// Clip, stretch, zoom.
    sal_Int32           mnPicAlign;         /// Anchor position of the picture.
    bool                mbPicTiling;        /// True = picture is repeated.
};

// ============================================================================

/** Base class for a Forms 2.0 morph data control. */
class AxMorphDataModelBase : public AxFontDataModel
{
public:
    explicit            AxMorphDataModelBase();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        importPictureData( sal_Int32 nPropId, BinaryInputStream& rInStrm );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

protected:
    StreamDataSequence  maPictureData;      /// Binary picture stream.
    ::rtl::OUString     maCaption;          /// Visible caption of the button.
    ::rtl::OUString     maValue;            /// Current value of the control.
    ::rtl::OUString     maGroupName;        /// Group name for option buttons.
    sal_uInt32          mnTextColor;        /// Text color.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_uInt32          mnPicturePos;       /// Position of the picture relative to text.
    sal_uInt32          mnBorderColor;      /// Flat border color.
    sal_Int32           mnBorderStyle;      /// Flat border style.
    sal_Int32           mnSpecialEffect;    /// 3D border effect.
    sal_Int32           mnDisplayStyle;     /// Type of the morph control.
    sal_Int32           mnMultiSelect;      /// Selection mode.
    sal_Int32           mnScrollBars;       /// Horizontal/vertical scroll bar.
    sal_Int32           mnMatchEntry;       /// Auto completion mode.
    sal_Int32           mnShowDropButton;   /// When to show the dropdown button.
    sal_Int32           mnMaxLength;        /// Maximum character count.
    sal_Int32           mnPasswordChar;     /// Password character in edit fields.
    sal_Int32           mnListRows;         /// Number of rows in dropdown box.
};

// ============================================================================

/** Model for a Forms 2.0 toggle button. */
class AxToggleButtonModel : public AxMorphDataModelBase
{
public:
    explicit            AxToggleButtonModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 check box. */
class AxCheckBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxCheckBoxModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 option button. */
class AxOptionButtonModel : public AxMorphDataModelBase
{
public:
    explicit            AxOptionButtonModel();

    /** Returns the group name used to goup several option buttons gogether. */
    inline const ::rtl::OUString& getGroupName() const { return maGroupName; }

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 text box. */
class AxTextBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxTextBoxModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 list box. */
class AxListBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxListBoxModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 combo box. */
class AxComboBoxModel : public AxMorphDataModelBase
{
public:
    explicit            AxComboBoxModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 spin button. */
class AxSpinButtonModel : public AxControlModelBase
{
public:
    explicit            AxSpinButtonModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

private:
    sal_uInt32          mnArrowColor;       /// Button arrow color.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_Int32           mnOrientation;      /// Orientation of the buttons.
    sal_Int32           mnMin;              /// Minimum of the value range.
    sal_Int32           mnMax;              /// Maximum of the value range.
    sal_Int32           mnPosition;         /// Value of the spin button.
    sal_Int32           mnSmallChange;      /// Increment step size.
    sal_Int32           mnDelay;            /// Repeat delay in milliseconds.
};

// ============================================================================

/** Model for a Forms 2.0 scroll bar. */
class AxScrollBarModel : public AxControlModelBase
{
public:
    explicit            AxScrollBarModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

private:
    sal_uInt32          mnArrowColor;       /// Button arrow color.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_Int32           mnOrientation;      /// Orientation of the buttons.
    sal_Int32           mnPropThumb;        /// Proportional thumb size.
    sal_Int32           mnMin;              /// Minimum of the value range.
    sal_Int32           mnMax;              /// Maximum of the value range.
    sal_Int32           mnPosition;         /// Value of the spin button.
    sal_Int32           mnSmallChange;      /// Increment step size (buttons).
    sal_Int32           mnLargeChange;      /// Increment step size (thumb).
    sal_Int32           mnDelay;            /// Repeat delay in milliseconds.
};

// ============================================================================

/** Model for a Forms 2.0 tabstrip control. */
class AxTabStripModel : public AxFontDataModel
{
public:
    explicit            AxTabStripModel();

    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

    /** Returns the caption with the specified zero-based index. */
    ::rtl::OUString     getCaption( sal_Int32 nIndex ) const;

private:
    AxStringArray       maCaptions;         /// Captions of all tabs.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnTextColor;        /// Text color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_Int32           mnSelectedTab;      /// The index of the selected tab.
    sal_uInt32          mnTabStyle;         /// Visual style of the tabs.
    sal_Int32           mnTabFlagCount;     /// Number of entries in tab flag array.
};

typedef ::boost::shared_ptr< AxTabStripModel > AxTabStripModelRef;

// ============================================================================

typedef ::std::vector< ::rtl::OUString > AxClassTable;

/** Base class for ActiveX container controls. */
class AxContainerModelBase : public AxFontDataModel
{
public:
    explicit            AxContainerModelBase( bool bFontSupport = false );

    /** Allows to set single properties specified by XML token identifier. */
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    /** Reads the leading structure in the 'f' stream containing the model for
        this control. */
    virtual bool        importBinaryModel( BinaryInputStream& rInStrm );
    /** Converts font settings if supported. */
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

    /** Reads the class table structure for embedded controls following the own
        model from the 'f' stream. */
    bool                importClassTable( BinaryInputStream& rInStrm, AxClassTable& orClassTable );

protected:
    StreamDataSequence  maPictureData;      /// Binary picture stream.
    ::rtl::OUString     maCaption;          /// Visible caption of the form.
    AxPairData          maLogicalSize;      /// Logical form size (scroll area).
    AxPairData          maScrollPos;        /// Scroll position.
    sal_uInt32          mnBackColor;        /// Fill color.
    sal_uInt32          mnTextColor;        /// Text color.
    sal_uInt32          mnFlags;            /// Various flags.
    sal_uInt32          mnBorderColor;      /// Flat border color.
    sal_Int32           mnBorderStyle;      /// Flat border style.
    sal_Int32           mnScrollBars;       /// Horizontal/vertical scroll bar.
    sal_Int32           mnCycleType;        /// Cycle in all forms or in this form.
    sal_Int32           mnSpecialEffect;    /// 3D border effect.
    sal_Int32           mnPicAlign;         /// Anchor position of the picture.
    sal_Int32           mnPicSizeMode;      /// Clip, stretch, zoom.
    bool                mbPicTiling;        /// True = picture is repeated.
    bool                mbFontSupport;      /// True = control supports the font property.
};

typedef ::boost::shared_ptr< AxContainerModelBase > AxContainerModelRef;

// ============================================================================

/** Model for a Forms 2.0 frame control. */
class AxFrameModel : public AxContainerModelBase
{
public:
    explicit            AxFrameModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 formpage control (a single page in a multipage control). */
class AxFormPageModel : public AxContainerModelBase
{
public:
    explicit            AxFormPageModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** Model for a Forms 2.0 multipage control. Contains the tabstrip control
    (class AxTabStripModel) and the single pages (class AxFormPageModel). */
class AxMultiPageModel : public AxContainerModelBase
{
public:
    explicit            AxMultiPageModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;

    /** Sets the tabstrip control model related to this multipage control.
        Contains all formatting attributes of the page tabs. */
    void                setTabStripModel( const AxTabStripModelRef& rxTabStrip );

private:
    AxTabStripModelRef  mxTabStrip;
};

// ============================================================================

/** Model for a Forms 2.0 user form. */
class AxUserFormModel : public AxContainerModelBase
{
public:
    explicit            AxUserFormModel();

    virtual ApiControlType getControlType() const;
    virtual void        convertProperties( PropertyMap& rPropMap, const ControlConverter& rConv ) const;
};

// ============================================================================

/** A form control embedded in a document draw page. Contains a specific model
    structure according to the type of the control. */
class EmbeddedControl
{
public:
    explicit            EmbeddedControl( const ::rtl::OUString& rName );
                        ~EmbeddedControl();

    /** Creates and returns the internal control model according to the passed
        MS class identifier. */
    ControlModelRef     createModel( const ::rtl::OUString& rClassId );

    /** Returns true, if the internal control model exists. */
    inline bool         hasModel() const { return mxModel.get() != 0; }
    /** Returns the UNO service name needed to construct the control model. */
    ::rtl::OUString     getServiceName() const;

    /** Converts all control properties and inserts them into the passed model. */
    bool                convertProperties(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxCtrlModel,
                            const ControlConverter& rConv ) const;

private:
    ControlModelRef     mxModel;            /// Control model containing the properties.
    ::rtl::OUString     maName;             /// Name of the control.
};

// ============================================================================

/** A wrapper for a control form embedded directly in a draw page. */
class EmbeddedForm : public ControlConverter
{
public:
    explicit            EmbeddedForm(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxModelFactory,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                            const GraphicHelper& rGraphicHelper,
                            bool bDefaultColorBgr = true );

    /** Converts the passed ActiveX control and inserts it into the form.
        @return  The API control model, if conversion was successful. */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                        convertAndInsert( const EmbeddedControl& rControl );

private:
    /** Tries to insert the passed control model into the form. */
    bool                insertControl( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxCtrlModel );

    /** Creates the form that will hold the form controls. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer >
                        createForm();

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxModelFactory;
    ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormsSupplier > mxFormsSupp;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexContainer > mxFormIC;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
