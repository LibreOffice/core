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

#include "oox/helper/storagebase.hxx"
#include <com/sun/star/uno/XInterface.hpp>
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
    namespace container { class XNameContainer; }
    namespace document { class XEventsSupplier; }
    namespace frame { class XModel; }
    namespace script { class XLibraryContainer; }
    namespace lang { class XMultiServiceFactory; }
} } }

namespace oox { class GraphicHelper; }

namespace oox {
namespace ole {

// ============================================================================

class OOX_DLLPUBLIC VbaFilterConfig
{
public:
    explicit            VbaFilterConfig(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory,
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

class OOX_DLLPUBLIC VbaProject : public VbaFilterConfig
{
public:
    explicit            VbaProject(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxGlobalFactory,
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

    /** Returns true, if the document contains at least one code module. */
    bool                hasModules() const;
    /** Returns true, if the document contains the specified code module. */
    bool                hasModule( const ::rtl::OUString& rModuleName ) const;

    /** Returns true, if the document contains at least one dialog. */
    bool                hasDialogs() const;
    /** Returns true, if the document contains the specified dialog. */
    bool                hasDialog( const ::rtl::OUString& rDialogName ) const;

    // Insert VBA code modules and VBA macros into modules --------------------

    /** Tries to insert a VBA macro into the specified code module.

        @descr  If the specified macro does not exist, it will be generated as
            following, using the passed parameters. If the parameter rMacroType
            is left empty, a sub procedure macro will be generated:

            Private Sub <rMacroName> ( <rMacroArgs> )
                <rMacroCode>
            End Sub

            If the parameter rMacroType is not empty, a function macro
            will be generated. Note that the parameter rMacroCode has to
            provide the code that returns the function value.

            Private Function <rMacroName> ( <rMacroArgs> ) As <rMacroType>
                <rMacroCode>
            End Function

            The source code in rMacroCode may contain a special placeholder
            $MACRO that will be replaced by the macro name passed in rMacroName
            before the macro will be inserted into the module.

        @param rModuleName  The name of the VBA module to be used.
        @param rMacroName  The name of the VBA macro to be inserted.
        @param rMacroArgs  The argument list of the VBA macro.
        @param rMacroType  Macro return type (empty for sub procedure).
        @param rMacroCode  The VBA source code for the macro.

        @return  True, if the specified VBA macro has been inserted. False, if
            there already exists a macro with the specified name, or if any
            error has occurred, for example, Office configuration forbids to
            generate executable VBA code or the specified module does not
            exist.
     */
    bool                insertMacro(
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName,
                            const ::rtl::OUString& rMacroArgs,
                            const ::rtl::OUString& rMacroType,
                            const ::rtl::OUString& rMacroCode );

    // Attach VBA macros to generic or document events ------------------------

    /** Tries to attach the specified VBA macro to an event directly.

        @descr  The function checks if the specified macro exists and attaches
            it to the event of the passed events supplier.

        @param rxEventsSupp  The events supplier for the event to be attached.
        @param rEventName  The event name used in the office API.
        @param rModuleName  The name of the VBA module containing the macro.
        @param rMacroName  The name of the VBA macro to attach to the event.

        @return  True, if the specified VBA macro exists and could be attached
            to the specified event.
     */
    bool                attachMacroToEvent(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventsSupplier >& rxEventsSupp,
                            const ::rtl::OUString& rEventName,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName );

    /** Tries to attach the specified VBA macro to a document event directly.

        @descr  The function checks if the specified macro exists and attaches
            it to the document event.

        @param rEventName  The document event name used in the office API.
        @param rModuleName  The name of the VBA module containing the macro.
        @param rMacroName  The name of the VBA macro to attach to the event.

        @return  True, if the specified VBA macro exists and could be attached
            to the specified document event.
     */
    bool                attachMacroToDocumentEvent(
                            const ::rtl::OUString& rEventName,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName );

    /** Tries to attach the specified VBA macro to an event via a generated
        proxy macro that implements necessary conversion between VBA and UNO.

        @descr  The function checks if the specified VBA macro exists, then it
            tries to generate a proxy macro using the parameters passed to this
            function, appends it to the code module, and attaches it to the
            event.

            The proxy macro will execute the code specified in the rProxyCode
            parameter. This code may contain special placeholders that will be
            replaced before the proxy macro will be inserted into the module:
            -   $MACRO will be replaced by the original VBA macro name passed
                in the rMacroName parameter.
            -   $PROXY will be replaced by the name of the proxy macro
                generated by this function.

        @param rxEventsSupp  The events supplier for the event to be attached.
        @param rEventName  The event name used in the office API.
        @param rModuleName  The name of the VBA module containing the macro.
        @param rMacroName  The name of the VBA macro to attach to the event.
        @param rProxyArgs  The argument list of the generated proxy macro.
        @param rProxyType  Proxy macro return type (empty for procedure).
        @param rProxyCode  Proxy macro source code.

        @return  True, if the specified VBA macro exists and could be attached
            to the specified event.
     */
    bool                attachMacroToEvent(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::document::XEventsSupplier >& rxEventsSupp,
                            const ::rtl::OUString& rEventName,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName,
                            const ::rtl::OUString& rProxyArgs,
                            const ::rtl::OUString& rProxyType,
                            const ::rtl::OUString& rProxyCode );

    /** Tries to attach the specified VBA macro to a document event via a
        proxy macro that implements necessary conversion between VBA and UNO.

        @descr  The function checks if the specified VBA macro exists, then it
            tries to generate a proxy macro using the parameters passed to this
            function, appends it to the code module, and attaches it to the
            document event.

            The proxy macro will execute the code specified in the rProxyCode
            parameter. This code may contain special placeholders that will be
            replaced before the proxy macro will be inserted into the module:
            -   $MACRO will be replaced by the original VBA macro name passed
                in the rMacroName parameter.
            -   $PROXY will be replaced by the name of the proxy macro
                generated by this function.

        @param rEventName  The document event name used in the office API.
        @param rModuleName  The name of the VBA module containing the macro.
        @param rMacroName  The name of the VBA macro to attach to the event.
        @param rProxyArgs  The argument list of the generated proxy macro.
        @param rProxyType  Proxy macro return type (empty for procedure).
        @param rProxyCode  Proxy macro source code.

        @return  True, if the specified VBA macro exists and could be attached
            to the specified event.
     */
    bool                attachMacroToDocumentEvent(
                            const ::rtl::OUString& rEventName,
                            const ::rtl::OUString& rModuleName,
                            const ::rtl::OUString& rMacroName,
                            const ::rtl::OUString& rProxyArgs,
                            const ::rtl::OUString& rProxyType,
                            const ::rtl::OUString& rProxyCode );

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
    void                importVba( StorageBase& rVbaPrjStrg, const GraphicHelper& rGraphicHelper, bool bDefaultColorBgr );
    /** Copies the entire VBA project storage to the passed document model. */
    void                copyStorage( StorageBase& rVbaPrjStrg );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        mxGlobalFactory;    /// Global service factory.
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        mxDocModel;         /// Document model used to import/export the VBA project.
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        mxBasicLib;         /// The Basic library of the document used for import.
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >
                        mxDialogLib;        /// The dialog library of the document used for import.
    const ::rtl::OUString maLibName;        /// Name for Basic and dialog library used for import.
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif
