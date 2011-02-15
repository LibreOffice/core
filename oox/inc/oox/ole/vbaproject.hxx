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

#ifndef OOX_OLE_VBAPROJECT_HXX
#define OOX_OLE_VBAPROJECT_HXX

#include <map>
#include <com/sun/star/uno/XInterface.hpp>
#include "oox/helper/refvector.hxx"
#include "oox/helper/storagebase.hxx"
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace document { class XEventsSupplier; }
    namespace frame { class XModel; }
    namespace script { class XLibraryContainer; }
    namespace script { namespace vba { class XVBAMacroResolver; } }
    namespace uno { class XComponentContext; }
} } }

namespace oox { class GraphicHelper; }

namespace oox {
namespace ole {

// ============================================================================

class OOX_DLLPUBLIC VbaFilterConfig
{
public:
    explicit            VbaFilterConfig(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::rtl::OUString& rConfigCompName );
                        ~VbaFilterConfig();

    /** Returns true, if the VBA source code and forms should be imported. */
    bool                isImportVba() const;
    /** Returns true, if the VBA source code should be imported executable. */
    bool                isImportVbaExecutable() const;
    /** Returns true, if the VBA source code and forms should be exported. */
    bool                isExportVba() const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        mxConfigAccess;
};

// ============================================================================

/** Base class for objects that attach a amcro to a specific action.

    Purpose is to collect objects that need to attach a VBA macro to an action.
    The VBA project will be loaded at a very late point of the document import
    process, because it depends on an initialized core document model (e.g.
    spreadsheet codenames). Some objects that want to attach a VBA macro to an
    action (e.g. mouse click action for drawing shapes) are loaded long before
    the VBA project. The drawback is that in most cases macros are specified
    without module name, or the VBA project name is part of the macro name.
    In the former case, all code modules have to be scanned for the macro to be
    able to create a valid script URL.

    The import code will register these requests to attach a VBA macro with an
    instance of a class derived from this base class. The derived class will
    store all information needed to finally attach the macro to the action,
    once the VBA project has been imported.
 */
class VbaMacroAttacherBase
{
public:
    explicit            VbaMacroAttacherBase( const ::rtl::OUString& rMacroName );
    virtual             ~VbaMacroAttacherBase();

    /** Resolves the internal macro name to the related macro URL, and attaches
        the macro to the object. */
    void                resolveAndAttachMacro(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::script::vba::XVBAMacroResolver >& rxResolver );

private:
    /** Called after the VBA project has been imported. Derived classes will
        attach the passed script to the object represented by this instance. */
    virtual void        attachMacro( const ::rtl::OUString& rScriptUrl ) = 0;

private:
    ::rtl::OUString     maMacroName;
};

typedef ::boost::shared_ptr< VbaMacroAttacherBase > VbaMacroAttacherRef;

// ============================================================================

class OOX_DLLPUBLIC VbaProject : public VbaFilterConfig
{
public:
    explicit            VbaProject(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxDocModel,
                            const ::rtl::OUString& rConfigCompName );
    virtual             ~VbaProject();

    /** Imports the entire VBA project from the passed storage.

        @param rVbaPrjStrg  The root storage of the entire VBA project.
     */
    void                importVbaProject(
                            StorageBase& rVbaPrjStrg,
                            const GraphicHelper& rGraphicHelper,
                            bool bDefaultColorBgr = true );

    /** Registers a macro atatcher object. For details, see description of the
        VbaMacroAttacherBase class. */
    void                registerMacroAttacher( const VbaMacroAttacherRef& rxAttacher );

    /** Returns true, if the document contains at least one code module. */
    bool                hasModules() const;
    /** Returns true, if the document contains the specified code module. */
    bool                hasModule( const ::rtl::OUString& rModuleName ) const;

    /** Returns true, if the document contains at least one dialog. */
    bool                hasDialogs() const;
    /** Returns true, if the document contains the specified dialog. */
    bool                hasDialog( const ::rtl::OUString& rDialogName ) const;

protected:
    /** Registers a dummy module that will be created when the VBA project is
        imported. */
    void                addDummyModule( const ::rtl::OUString& rName, sal_Int32 nType );

    /** Called when the import process of the VBA project has been started. */
    virtual void        prepareImport();
    /** Called when the import process of the VBA project is finished. */
    virtual void        finalizeImport();

private:
                        VbaProject( const VbaProject& );
    VbaProject&         operator=( const VbaProject& );

    /** Returns the Basic or dialog library container. */
    ::com::sun::star::uno::Reference< ::com::sun::star::script::XLibraryContainer >
                        getLibraryContainer( sal_Int32 nPropId );
    /** Opens a Basic or dialog library (creates missing if specified). */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        openLibrary( sal_Int32 nPropId, bool bCreateMissing );
    /** Creates and returns the Basic library of the document used for import. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        createBasicLibrary();
    /** Creates and returns the dialog library of the document used for import. */
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        createDialogLibrary();

    /** Imports the VBA code modules and forms. */
    void                importVba(
                            StorageBase& rVbaPrjStrg,
                            const GraphicHelper& rGraphicHelper,
                            bool bDefaultColorBgr );

    /** Attaches VBA macros to objects registered via registerMacroAttacher(). */
    void                attachMacros();

    /** Copies the entire VBA project storage to the passed document model. */
    void                copyStorage( StorageBase& rVbaPrjStrg );

private:
    typedef RefVector< VbaMacroAttacherBase >           MacroAttacherVector;
    typedef ::std::map< ::rtl::OUString, sal_Int32 >    DummyModuleMap;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        mxCompContext;      /// Component context with service manager.
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        mxDocModel;         /// Document model used to import/export the VBA project.
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        mxBasicLib;         /// The Basic library of the document used for import.
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        mxDialogLib;        /// The dialog library of the document used for import.
    MacroAttacherVector maMacroAttachers;   /// Objects that want to attach a VBA macro to an action.
    DummyModuleMap      maDummyModules;     /// Additional empty modules created on import.
    ::rtl::OUString     maPrjName;          /// Name of the VBA project.
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif
