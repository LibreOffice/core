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
#ifndef INCLUDED_FILTER_MSFILTER_MSOLEEXP_HXX
#define INCLUDED_FILTER_MSFILTER_MSOLEEXP_HXX

#include <com/sun/star/uno/Reference.h>
#include <filter/msfilter/msfilterdllapi.h>

// for the CreateSdrOLEFromStorage we need the information, how we handle
// convert able OLE-Objects - this ist stored in
#define OLE_STARMATH_2_MATHTYPE             0x0001
#define OLE_STARWRITER_2_WINWORD            0x0002
#define OLE_STARCALC_2_EXCEL                0x0004
#define OLE_STARIMPRESS_2_POWERPOINT        0x0008

class SotStorage;

#include <svtools/embedhlp.hxx>

class MSFILTER_DLLPUBLIC SvxMSExportOLEObjects
{
    sal_uInt32 nConvertFlags;
public:
    SvxMSExportOLEObjects( sal_uInt32 nCnvrtFlgs ) : nConvertFlags(nCnvrtFlgs) {}
    sal_uInt32 GetFlags() const         { return nConvertFlags; }

    void ExportOLEObject( svt::EmbeddedObjectRef& rObj, SotStorage& rDestStg );
    void ExportOLEObject( const css::uno::Reference < css::embed::XEmbeddedObject>& rObj, SotStorage& rDestStg );
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
