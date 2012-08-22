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

#ifndef OOX_OLE_OLEOBJECTHELPER_HXX
#define OOX_OLE_OLEOBJECTHELPER_HXX

#include "oox/helper/binarystreambase.hxx"

namespace com { namespace sun { namespace star {
    namespace awt { struct Size; }
    namespace document { class XEmbeddedObjectResolver; }
    namespace lang { class XMultiServiceFactory; }
} } }

namespace oox { class PropertyMap; }

namespace oox {
namespace ole {

// ============================================================================

/** Contains generic information about an OLE object. */
struct OleObjectInfo
{
    StreamDataSequence  maEmbeddedData;     ///< Data of an embedded OLE object.
    ::rtl::OUString     maTargetLink;       ///< Path to external data for linked OLE object.
    ::rtl::OUString     maProgId;
    bool                mbLinked;           ///< True = linked OLE object, false = embedded OLE object.
    bool                mbShowAsIcon;       ///< True = show as icon, false = show contents.
    bool                mbAutoUpdate;

    explicit            OleObjectInfo();
};

// ============================================================================

/** Helper for OLE object handling. */
class OleObjectHelper
{
public:
    explicit            OleObjectHelper(
                            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxModelFactory );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
