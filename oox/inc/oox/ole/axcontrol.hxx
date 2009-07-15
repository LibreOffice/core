/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: axcontrol.hxx,v $
 * $Revision: 1.1 $
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

#include <memory>
#include <rtl/ustring.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { class XControlModel; }
} } }

namespace oox { class PropertyMap; }

namespace oox {
namespace ole {

class AxControlHelper;

// ============================================================================

/** Base class for all models of ActiveX form controls. */
class AxControlModelBase
{
public:
    virtual             ~AxControlModelBase();

    /** Derived classes set specific OOXML properties at the model structure. */
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    /** Derived classes set binary data (picture, mouse icon) at the model structure. */
    virtual void        importPictureData( sal_Int32 nPropId, const StreamDataSequence& rDataSeq );

    /** Derived classes return the UNO service name used to construct the control component. */
    virtual ::rtl::OUString getServiceName() const = 0;
    /** Derived classes convert all control properties. */
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

protected:
    explicit            AxControlModelBase();

protected:
    sal_Int32           mnWidth;
    sal_Int32           mnHeight;
};

// ============================================================================

class AxFontDataModel : public AxControlModelBase
{
public:
    explicit            AxFontDataModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

private:
    ::rtl::OUString     maFontName;         /// Name of the used font.
    sal_uInt32          mnFontEffects;      /// Font effect flags.
    sal_Int32           mnFontHeight;       /// Height of the font (not really twips, see code).
    sal_Int32           mnFontCharSet;      /// Windows character set of the font.
    sal_Int32           mnHorAlign;         /// Horizontal text alignment.
};

// ============================================================================

class AxCommandButtonModel : public AxFontDataModel
{
public:
    explicit            AxCommandButtonModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        importPictureData( sal_Int32 nPropId, const StreamDataSequence& rDataSeq );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

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

class AxLabelModel : public AxFontDataModel
{
public:
    explicit            AxLabelModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

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

class AxImageModel : public AxControlModelBase
{
public:
    explicit            AxImageModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        importPictureData( sal_Int32 nPropId, const StreamDataSequence& rDataSeq );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

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

class AxMorphDataModel : public AxFontDataModel
{
public:
    explicit            AxMorphDataModel();

    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        importPictureData( sal_Int32 nPropId, const StreamDataSequence& rDataSeq );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

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

class AxToggleButtonModel : public AxMorphDataModel
{
public:
    explicit            AxToggleButtonModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;
};

// ============================================================================

class AxCheckBoxModel : public AxMorphDataModel
{
public:
    explicit            AxCheckBoxModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;
};

// ============================================================================

class AxOptionButtonModel : public AxMorphDataModel
{
public:
    explicit            AxOptionButtonModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;
};

// ============================================================================

class AxTextBoxModel : public AxMorphDataModel
{
public:
    explicit            AxTextBoxModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;
};

// ============================================================================

class AxListBoxModel : public AxMorphDataModel
{
public:
    explicit            AxListBoxModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;
};

// ============================================================================

class AxComboBoxModel : public AxMorphDataModel
{
public:
    explicit            AxComboBoxModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;
};

// ============================================================================

class AxSpinButtonModel : public AxControlModelBase
{
public:
    explicit            AxSpinButtonModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

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

class AxScrollBarModel : public AxControlModelBase
{
public:
    explicit            AxScrollBarModel();

    virtual ::rtl::OUString getServiceName() const;
    virtual void        importProperty( sal_Int32 nPropId, const ::rtl::OUString& rValue );
    virtual void        convertProperties( AxControlHelper& rHelper, PropertyMap& rPropMap ) const;

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

/** Container for all ActiveX form control model implementations. */
class AxControl
{
public:
    explicit            AxControl( const ::rtl::OUString& rName );
                        ~AxControl();

    /** Creates and returns the internal control model according to the passed
        MS class identifier. */
    AxControlModelBase* createModel( const ::rtl::OUString& rClassId );

    /** Creates and returns the UNO form component object for this control and
        inserts it into the form wrapped by the passed helper. */
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >
                        convertAndInsert( AxControlHelper& rHelper ) const;

private:
    ::std::auto_ptr< AxControlModelBase > mxModel;
    ::rtl::OUString     maName;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

