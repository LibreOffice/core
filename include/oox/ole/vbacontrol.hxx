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

#ifndef INCLUDED_OOX_OLE_VBACONTROL_HXX
#define INCLUDED_OOX_OLE_VBACONTROL_HXX

#include <oox/ole/axcontrol.hxx>
#include <com/sun/star/frame/XModel.hpp>

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace uno { class XComponentContext; }
} } }

namespace oox { class StorageBase; }

namespace oox {
namespace ole {



/** Common properties for all controls that are part of a VBA user form or of
    another container control in a VBA user form. */
class VbaSiteModel
{
public:
    explicit            VbaSiteModel();
    virtual             ~VbaSiteModel();

    /** Allows to set single properties specified by XML token identifier. */
    void                importProperty( sal_Int32 nPropId, const OUString& rValue );
    /** Imports the site model data from the passed input stream. */
    bool                importBinaryModel( BinaryInputStream& rInStrm );
    /** Moves the control relative to its current position by the passed distance. */
    void                moveRelative( const AxPairData& rDistance );

    /** Returns the programmatical name of the control. */
    const OUString& getName() const { return maName; }
    /** Returns the position of the control in its parent. */
    const AxPairData& getPosition() const { return maPos; }
    /** Returns the unique identifier of this control. */
    sal_Int32    getId() const { return mnId; }
    /** Returns true, if this control is a container control. */
    bool                isContainer() const;
    /** Returns the length of the stream data for stream based controls. */
    sal_uInt32          getStreamLength() const;
    /** Returns the name of the substorage for the container control data. */
    OUString     getSubStorageName() const;
    /** Returns the tab index of the control. */
    sal_Int16    getTabIndex() const { return mnTabIndex; }

    /** Tries to create the control model according to the site model. */
    ControlModelRef     createControlModel( const AxClassTable& rClassTable ) const;
    /** Converts all form site properties. */
    void                convertProperties(
                            PropertyMap& rPropMap,
                            const ControlConverter& rConv,
                            ApiControlType eCtrlType,
                            sal_Int32 nCtrlIndex ) const;
    ::rtl::OUString getControlSource() { return  maControlSource; }
    ::rtl::OUString getRowSource() { return  maRowSource; }
protected:
    OUString     maName;             ///< Name of the control.
    OUString     maTag;              ///< User defined tag.
    OUString     maToolTip;          ///< Tool tip for the control.
    OUString     maControlSource;    ///< Linked cell for the control value in a spreadsheet.
    OUString     maRowSource;        ///< Source data for the control in a spreadsheet.

    AxPairData          maPos;              ///< Position in parent container.
    sal_Int32           mnId;               ///< Control identifier.
    sal_Int32           mnHelpContextId;    ///< Help context identifier.
    sal_uInt32          mnFlags;            ///< Various flags.
    sal_uInt32          mnStreamLen;        ///< Size of control stream data.
    sal_Int16           mnTabIndex;         ///< Tab order index.
    sal_uInt16          mnClassIdOrCache;   ///< Class name identifier or GUID cache index.
    sal_uInt16          mnGroupId;          ///< Group identifier for grouped controls.
};

typedef std::shared_ptr< VbaSiteModel > VbaSiteModelRef;



/** A control that is embedded in a VBA user form or in another container
    control in a VBA user form.

    The control may be a 'simple' control with its data stored in the 'o'
    stream, or it may be a container control with its data stored in an own
    substorage.
 */
class VbaFormControl
{
public:
    explicit            VbaFormControl();
    virtual             ~VbaFormControl();

    /** Imports the model from the passed stream or storage, depending on the
        control's type. Imports all embedded controls, if this is a container. */
    void                importModelOrStorage(
                            BinaryInputStream& rInStrm,
                            StorageBase& rStrg,
                            const AxClassTable& rClassTable );

    /** Returns the programmatical name of the control. */
    OUString     getControlName() const;

    /** Creates the UNO control model, inserts it into the passed container,
        and converts all control properties. */
    void                createAndConvert(
                            sal_Int32 nCtrlIndex,
                            const css::uno::Reference< css::container::XNameContainer >& rxParentNC,
                            const ControlConverter& rConv ) const;

protected:
    /** Creates and imports the control model containing properties of the control. */
    void                importControlModel( BinaryInputStream& rInStrm, const AxClassTable& rClassTable );
    /** Creates and imports the control model, and imports all embedded
        controls from the passed substorage. */
    void                importStorage( StorageBase& rStrg, const AxClassTable& rClassTable );

    /** Converts all control properties, and inserts and converts embedded controls. */
    bool                convertProperties(
                            const css::uno::Reference< css::awt::XControlModel >& rxCtrlModel,
                            const ControlConverter& rConv,
                            sal_Int32 nCtrlIndex ) const;

private:
    typedef RefVector< VbaFormControl >         VbaFormControlVector;
    typedef VbaFormControlVector::value_type    VbaFormControlRef;

    /** Creates the control model according to the current site model. */
    void                createControlModel( const AxClassTable& rClassTable );
    /** Imports the site model data containing common properties of the control. */
    bool                importSiteModel( BinaryInputStream& rInStrm );

    /** Imports the site models of all embedded controls from the 'f' stream. */
    bool                importEmbeddedSiteModels( BinaryInputStream& rInStrm );
    /*  Final processing of all embedded controls after import. */
    void                finalizeEmbeddedControls();

    /** Moves the control relative to its current position by the passed distance. */
    void                moveRelative( const AxPairData& rDistance );
    /** Moves all embedded controls from their relative position in this
        control to an absolute position in the parent of this control. */
    void                moveEmbeddedToAbsoluteParent();

    /** Functor for comparing controls by their tab index. */
    static bool         compareByTabIndex( const VbaFormControlRef& rxLeft, const VbaFormControlRef& rxRight );

protected:
    VbaSiteModelRef     mxSiteModel;        ///< Common control properties.
    ControlModelRef     mxCtrlModel;        ///< Specific control properties.

private:
    VbaFormControlVector maControls;        ///< All embedded form controls.
    AxClassTable         maClassTable;      ///< Class identifiers for exotic embedded controls.
};



class VbaUserForm : public VbaFormControl
{
public:
    explicit            VbaUserForm(
                            const css::uno::Reference< css::uno::XComponentContext >& rxContext,
                            const css::uno::Reference< css::frame::XModel >& rxDocModel,
                            const GraphicHelper& rGraphicHelper,
                            bool bDefaultColorBgr = true );

    /** Imports the form and its embedded controls, and inserts the form with
        all its controls into the passed dialog library. */
    void                importForm(
                           const css::uno::Reference< css::container::XNameContainer >& rxDialogLib,
                           StorageBase& rVbaFormStrg,
                           const OUString& rModuleName,
                           rtl_TextEncoding eTextEnc );

private:
    css::uno::Reference< css::uno::XComponentContext > mxContext;
    css::uno::Reference< css::frame::XModel >          mxDocModel;
    ControlConverter                                   maConverter;
};



} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
