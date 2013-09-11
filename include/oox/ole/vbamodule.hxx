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

#ifndef OOX_OLE_VBAMODULE_HXX
#define OOX_OLE_VBAMODULE_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <rtl/ustring.hxx>

namespace com { namespace sun { namespace star {
    namespace container { class XNameAccess; }
    namespace container { class XNameContainer; }
    namespace frame { class XModel; }
    namespace uno { class XComponentContext; }
} } }

namespace oox {
    class BinaryInputStream;
    class StorageBase;
}

namespace oox {
namespace ole {

// ============================================================================

class VbaModule
{
public:
    explicit            VbaModule(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& rxContext,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >& rxDocModel,
                            const OUString& rName,
                            rtl_TextEncoding eTextEnc,
                            bool bExecutable );

    /** Returns the module type (com.sun.star.script.ModuleType constant). */
    sal_Int32    getType() const { return mnType; }
    /** Sets the passed module type. */
    void         setType( sal_Int32 nType ) { mnType = nType; }

    /** Returns the name of the module. */
    const OUString& getName() const { return maName; }
    /** Returns the stream name of the module. */
    const OUString& getStreamName() const { return maStreamName; }

    /** Imports all records for this module until the MODULEEND record. */
    void                importDirRecords( BinaryInputStream& rDirStrm );

    /** Imports the VBA source code into the passed Basic library. */
    void                createAndImportModule(
                            StorageBase& rVbaStrg,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxBasicLib,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& rxDocObjectNA ) const;
    /** Creates an empty Basic module in the passed Basic library. */
    void                createEmptyModule(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxBasicLib,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& rxDocObjectNA ) const;

private:
    /** Reads and returns the VBA source code from the passed storage. */
    OUString     readSourceCode( StorageBase& rVbaStrg ) const;

    /** Creates a new Basic module and inserts it into the passed Basic library. */
    void                createModule(
                            const OUString& rVBASourceCode,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& rxBasicLib,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& rxDocObjectNA ) const;

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >
                        mxContext;          ///< Component context with service manager.
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
                        mxDocModel;         ///< Document model used to import/export the VBA project.
    OUString     maName;
    OUString     maStreamName;
    OUString     maDocString;
    rtl_TextEncoding    meTextEnc;
    sal_Int32           mnType;
    sal_uInt32          mnOffset;
    bool                mbReadOnly;
    bool                mbPrivate;
    bool                mbExecutable;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
