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

#ifndef OOX_VML_VMLDRAWING_HXX
#define OOX_VML_VMLDRAWING_HXX

#include <map>
#include <memory>
#include <vector>
#include <oox/ole/axcontrol.hxx>
#include <oox/ole/oleobjecthelper.hxx>
#include <oox/vml/vmlshapecontainer.hxx>
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace awt { class XControlModel; }
    namespace drawing { class XDrawPage; }
    namespace drawing { class XShape; }
    namespace drawing { class XShapes; }
} } }

namespace oox {
    namespace core { class XmlFilterBase; }
    namespace ole { class EmbeddedControl; }
}

namespace oox {
namespace vml {

class ShapeBase;
struct ClientData;

// ============================================================================

/** Enumerates different types of VML drawings. */
enum DrawingType
{
    VMLDRAWING_WORD,            ///< Word: One shape per drawing.
    VMLDRAWING_EXCEL,           ///< Excel: OLE objects are part of VML.
    VMLDRAWING_POWERPOINT       ///< PowerPoint: OLE objects are part of DrawingML.
};

// ============================================================================

/** Contains information about an OLE object embedded in a draw page. */
struct OOX_DLLPUBLIC OleObjectInfo : public ::oox::ole::OleObjectInfo
{
    OUString     maShapeId;          ///< Shape identifier for shape lookup.
    OUString     maName;             ///< Programmatical name of the OLE object.
    bool                mbAutoLoad;
    const bool          mbDmlShape;         ///< True = DrawingML shape (PowerPoint), false = VML shape (Excel/Word).

    explicit            OleObjectInfo( bool bDmlShape = false );

    /** Sets the string representation of the passed numeric shape identifier. */
    void                setShapeId( sal_Int32 nShapeId );
};

// =========================================/===================================

/** Contains information about a form control embedded in a draw page. */
struct OOX_DLLPUBLIC ControlInfo
{
    OUString     maShapeId;          ///< Shape identifier for shape lookup.
    OUString     maFragmentPath;     ///< Path to the fragment describing the form control properties.
    OUString     maName;             ///< Programmatical name of the form control.

    explicit            ControlInfo();

    /** Sets the string representation of the passed numeric shape identifier. */
    void                setShapeId( sal_Int32 nShapeId );
};

// ============================================================================

/** Represents the collection of VML shapes for a complete draw page. */
class OOX_DLLPUBLIC Drawing
{
public:
    explicit            Drawing(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                            DrawingType eType );

    virtual             ~Drawing();

    /** Returns the filter object that imports/exports this VML drawing. */
    ::oox::core::XmlFilterBase& getFilter() const { return mrFilter; }
    /** Returns the application type containing the drawing. */
    DrawingType  getType() const { return meType; }
    /** Returns read/write access to the container of shapes and templates. */
    ShapeContainer& getShapes() { return *mxShapes; }
    /** Returns read access to the container of shapes and templates. */
    const ShapeContainer& getShapes() const { return *mxShapes; }
    /** Returns the form object used to process ActiveX form controls. */
    ::oox::ole::EmbeddedForm& getControlForm() const;

    /** Registers a block of shape identifiers reserved by this drawing. Block
        size is 1024, shape identifiers are one-based (block 1 => 1025-2048). */
    void                registerBlockId( sal_Int32 nBlockId );
    /** Registers the passed embedded OLE object. The related shape will then
        load the OLE object data from the specified fragment. */
    void                registerOleObject( const OleObjectInfo& rOleObject );
    /** Registers the passed embedded form control. The related shape will then
        load the control properties from the specified fragment. */
    void                registerControl( const ControlInfo& rControl );

    /** Final processing after import of the fragment. */
    void                finalizeFragmentImport();

    /** Creates and inserts all UNO shapes into the draw page. The virtual
        function notifyXShapeInserted() will be called for each new shape. */
    void                convertAndInsert() const;

    /** Returns the local shape index from the passed global shape identifier. */
    sal_Int32           getLocalShapeIndex( const OUString& rShapeId ) const;
    /** Returns the registered info structure for an OLE object, if extant. */
    const OleObjectInfo* getOleObjectInfo( const OUString& rShapeId ) const;
    /** Returns the registered info structure for a form control, if extant. */
    const ControlInfo*  getControlInfo( const OUString& rShapeId ) const;

    /** Creates a new UNO shape object, inserts it into the passed UNO shape
        container, and sets the shape position and size. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsertXShape(
                            const OUString& rService,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

    /** Creates a new UNO shape object for a form control, inserts the control
        model into the form, and the shape into the passed UNO shape container. */
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsertXControlShape(
                            const ::oox::ole::EmbeddedControl& rControl,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect,
                            sal_Int32& rnCtrlIndex ) const;

    /** Derived classes may disable conversion of specific shapes. */
    virtual bool        isShapeSupported( const ShapeBase& rShape ) const;

    /** Derived classes may return additional base names for automatic shape
        name creation. */
    virtual OUString getShapeBaseName( const ShapeBase& rShape ) const;

    /** Derived classes may calculate the shape rectangle from a non-standard
        anchor information string. */
    virtual bool        convertClientAnchor(
                            ::com::sun::star::awt::Rectangle& orShapeRect,
                            const OUString& rShapeAnchor ) const;

    /** Derived classes create a UNO shape according to the passed shape model.
        Called for shape models that specify being under host control. */
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >
                        createAndInsertClientXShape(
                            const ShapeBase& rShape,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShapes >& rxShapes,
                            const ::com::sun::star::awt::Rectangle& rShapeRect ) const;

    /** Derived classes may want to know that a UNO shape has been inserted.
        Will be called from the convertAndInsert() implementation.
        @param bGroupChild  True = inserted into a group shape,
            false = inserted directly into this drawing. */
    virtual void        notifyXShapeInserted(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XShape >& rxShape,
                            const ::com::sun::star::awt::Rectangle& rShapeRect,
                            const ShapeBase& rShape, bool bGroupChild );

private:
    typedef ::std::vector< sal_Int32 >                      BlockIdVector;
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    typedef ::std::auto_ptr< ::oox::ole::EmbeddedForm >     EmbeddedFormPtr;
    typedef ::std::auto_ptr< ShapeContainer >               ShapeContainerPtr;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    typedef ::std::map< OUString, OleObjectInfo >    OleObjectInfoMap;
    typedef ::std::map< OUString, ControlInfo >      ControlInfoMap;

    ::oox::core::XmlFilterBase& mrFilter;   ///< Filter object that imports/exports the VML drawing.
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                        mxDrawPage;         ///< UNO draw page used to insert the shapes.
    mutable EmbeddedFormPtr mxCtrlForm;     ///< The control form used to process embedded controls.
    mutable BlockIdVector maBlockIds;       ///< Block identifiers used by this drawing.
    ShapeContainerPtr   mxShapes;           ///< All shapes and shape templates.
    OleObjectInfoMap    maOleObjects;       ///< Info about all embedded OLE objects, mapped by shape id.
    ControlInfoMap      maControls;         ///< Info about all embedded form controls, mapped by control name.
    const DrawingType   meType;             ///< Application type containing the drawing.
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
