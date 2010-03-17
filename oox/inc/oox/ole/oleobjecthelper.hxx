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

#ifndef OOX_OLE_OLEOBJECTHELPER_HXX
#define OOX_OLE_OLEOBJECTHELPER_HXX

#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Size; }
    namespace lang { class XMultiServiceFactory; }
    namespace document { class XEmbeddedObjectResolver; }
} } }

namespace oox { class PropertyMap; }

namespace oox {
namespace ole {

// ============================================================================

/** Contains generic information about an OLE object. */
struct OleObjectInfo
{
    StreamDataSequence  maEmbeddedData;     /// Data of an embedded OLE object.
    ::rtl::OUString     maTargetLink;       /// Path to external data for linked OLE object.
    ::rtl::OUString     maProgId;
    bool                mbLinked;           /// True = linked OLE object, false = embedded OLE object.
    bool                mbShowAsIcon;       /// True = show as icon, false = show contents.
    bool                mbAutoUpdate;

    explicit            OleObjectInfo();
};

// ============================================================================

/** Helper for OLE object handling. */
class OleObjectHelper
{
public:
    explicit            OleObjectHelper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );
                        ~OleObjectHelper();

    bool                importOleObject(
                            PropertyMap& rPropMap,
                            const OleObjectInfo& rOleObject,
                            const ::com::sun::star::awt::Size& rObjSize );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::document::XEmbeddedObjectResolver > mxResolver;
    const ::rtl::OUString maEmbeddedObjScheme;
    sal_Int32           mnObjectId;
};

// ============================================================================

} // namespace ole
} // namespace oox

#endif

