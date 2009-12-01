/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vmldrawing.hxx,v $
 * $Revision: 1.3 $
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

#ifndef OOX_VML_VMLDRAWING_HXX
#define OOX_VML_VMLDRAWING_HXX

#include <map>
#include <memory>
#include "oox/ole/oleobjecthelper.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Rectangle; }
    namespace awt { class XControlModel; }
    namespace drawing { class XDrawPage; }
} } }

namespace oox { namespace core { class XmlFilterBase; } }
namespace oox { namespace ole { class AxControlHelper; } }

namespace oox {
namespace vml {

class ShapeContainer;
struct ShapeClientData;

// ============================================================================

/** Enumerates different types of VML drawings. */
enum DrawingType
{
    VMLDRAWING_WORD,            /// Word: One shape per drawing.
    VMLDRAWING_EXCEL,           /// Excel: OLE objects are part of VML.
    VMLDRAWING_POWERPOINT       /// PowerPoint: OLE objects are part of DrawingML.
};

// ============================================================================

/** Contains information about an OLE object embedded in a draw page. */
struct OleObjectInfo : public ::oox::ole::OleObjectInfo
{
    ::rtl::OUString     maShapeId;          /// Shape identifier for shape lookup.
    ::rtl::OUString     maName;             /// Programmatical name of the OLE object.
    bool                mbAutoLoad;
    const bool          mbDmlShape;         /// True = DrawingML shape (PowerPoint), false = VML shape (Excel/Word).

    explicit            OleObjectInfo( bool bDmlShape = false );

    /** Sets the string representation of the passed numeric shape identifier. */
    void                setShapeId( sal_Int32 nShapeId );
};

// ============================================================================

/** Contains information about a form control embedded in a draw page. */
struct ControlInfo
{
    ::rtl::OUString     maShapeId;          /// Shape identifier for shape lookup.
    ::rtl::OUString     maFragmentPath;     /// Path to the fragment describing the form control properties.
    ::rtl::OUString     maName;             /// Programmatical name of the form control.

    explicit            ControlInfo();

    /** Sets the string representation of the passed numeric shape identifier. */
    void                setShapeId( sal_Int32 nShapeId );
};

// ============================================================================

/** Represents the collection of VML shapes for a complete draw page. */
class Drawing
{
public:
    explicit            Drawing(
                            ::oox::core::XmlFilterBase& rFilter,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >& rxDrawPage,
                            DrawingType eType );

    virtual             ~Drawing();

    /** Returns the filter object that imports/exports this VML drawing. */
    inline ::oox::core::XmlFilterBase& getFilter() const { return mrFilter; }
    /** Returns the application type containing the drawing. */
    inline DrawingType  getType() const { return meType; }
    /** Returns read/write access to the container of shapes and templates. */
    inline ShapeContainer& getShapes() { return *mxShapes; }
    /** Returns read access to the container of shapes and templates. */
    inline const ShapeContainer& getShapes() const { return *mxShapes; }
    /** Returns the helper object used to process ActiveX form controls. */
    ::oox::ole::AxControlHelper& getControlHelper() const;

    /** Registers the passed embedded OLE object. The related shape will then
        load the OLE object data from the specified fragment. */
    void                registerOleObject( const OleObjectInfo& rOleObject );
    /** Registers the passed embedded form control. The related shape will then
        load the control properties from the specified fragment. */
    void                registerControl( const ControlInfo& rControl );

    /** Final processing after import of the fragment. */
    void                finalizeFragmentImport();

    /** Creates and inserts all UNO shapes into the passed container. */
    void                convertAndInsert() const;

    /** Returns the registered info structure for an OLE object, if extant. */
    const OleObjectInfo* getOleObjectInfo( const ::rtl::OUString& rShapeId ) const;
    /** Returns the registered info structure for a form control, if extant. */
    const ControlInfo*  getControlInfo( const ::rtl::OUString& rShapeId ) const;

    /** Derived classes may calculate the shape rectangle from a non-standard
        anchor information string. */
    virtual bool        convertShapeClientAnchor(
                            ::com::sun::star::awt::Rectangle& orShapeRect,
                            const ::rtl::OUString& rShapeAnchor ) const;

    /** Derived classes may convert additional form control properties from the
        passed VML shape client data. */
    virtual void        convertControlClientData(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& rxCtrlModel,
                            const ShapeClientData& rClientData ) const;

protected:
    /** Derived classes may create a specialized form control helper object. */
    virtual ::oox::ole::AxControlHelper* createControlHelper() const;

private:
    typedef ::std::auto_ptr< ::oox::ole::AxControlHelper >  AxControlHelperPtr;
    typedef ::std::auto_ptr< ShapeContainer >               ShapeContainerPtr;
    typedef ::std::map< ::rtl::OUString, OleObjectInfo >    OleObjectInfoMap;
    typedef ::std::map< ::rtl::OUString, ControlInfo >      ControlInfoMap;

    ::oox::core::XmlFilterBase& mrFilter;   /// Filter object that imports/exports the VML drawing.
    ::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawPage >
                        mxDrawPage;         /// UNO draw page used to insert the shapes.
    mutable AxControlHelperPtr mxCtrlHelper;/// Form control helper.
    ShapeContainerPtr   mxShapes;           /// All shapes and shape templates.
    OleObjectInfoMap    maOleObjects;       /// Info about all embedded OLE objects, mapped by shape id.
    ControlInfoMap      maControls;         /// Info about all embedded form controls, mapped by control name.
    const DrawingType   meType;             /// Application type containing the drawing.
};

// ============================================================================

} // namespace vml
} // namespace oox

#endif

