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
#ifdef _MSC_VER
#pragma warning(disable : 4917 4555)
#endif

#include "common.h"
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

::rtl::OUString getStorageTypeFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return ::rtl::OUString( "soffice.StarWriterDocument.6" );

    if ( *guid == OID_WriterOASISTextServer )
        return ::rtl::OUString( "LibreOffice.WriterDocument.1" );

    if ( *guid == OID_CalcServer )
        return ::rtl::OUString( "soffice.StarCalcDocument.6" );

    if ( *guid == OID_CalcOASISServer )
        return ::rtl::OUString( "LibreOffice.CalcDocument.1" );

    if ( *guid == OID_DrawingServer )
        return ::rtl::OUString( "soffice.StarDrawDocument.6" );

    if ( *guid == OID_DrawingOASISServer )
        return ::rtl::OUString( "LibreOffice.DrawDocument.1" );

    if ( *guid == OID_PresentationServer )
        return ::rtl::OUString( "soffice.StarImpressDocument.6" );

    if ( *guid == OID_PresentationOASISServer )
        return ::rtl::OUString( "LibreOffice.ImpressDocument.1" );

    if ( *guid == OID_MathServer )
        return ::rtl::OUString( "soffice.StarMathDocument.6" );

    if ( *guid == OID_MathOASISServer )
        return ::rtl::OUString( "LibreOffice.MathDocument.1" );

    return ::rtl::OUString();
}

::rtl::OUString getServiceNameFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return ::rtl::OUString( "com.sun.star.comp.Writer.TextDocument" );

    if ( *guid == OID_WriterOASISTextServer )
        return ::rtl::OUString( "com.sun.star.comp.Writer.TextDocument" );

    if ( *guid == OID_CalcServer )
        return ::rtl::OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );

    if ( *guid == OID_CalcOASISServer )
        return ::rtl::OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );

    if ( *guid == OID_DrawingServer )
        return ::rtl::OUString( "com.sun.star.comp.Draw.DrawingDocument" );

    if ( *guid == OID_DrawingOASISServer )
        return ::rtl::OUString( "com.sun.star.comp.Draw.DrawingDocument" );

    if ( *guid == OID_PresentationServer )
        return ::rtl::OUString( "com.sun.star.comp.Draw.PresentationDocument" );

    if ( *guid == OID_PresentationOASISServer )
        return ::rtl::OUString( "com.sun.star.comp.Draw.PresentationDocument" );

    if ( *guid == OID_MathServer )
        return ::rtl::OUString( "com.sun.star.comp.Math.FormulaDocument" );

    if ( *guid == OID_MathOASISServer )
        return ::rtl::OUString( "com.sun.star.comp.Math.FormulaDocument" );

    return ::rtl::OUString();
}

::rtl::OUString getFilterNameFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "StarOffice XML (Writer)" ) );

    if ( *guid == OID_WriterOASISTextServer )
        return ::rtl::OUString( "writer8" );

    if ( *guid == OID_CalcServer )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "StarOffice XML (Calc)" ) );

    if ( *guid == OID_CalcOASISServer )
        return ::rtl::OUString( "calc8" );

    if ( *guid == OID_DrawingServer )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "StarOffice XML (Draw)" ) );

    if ( *guid == OID_DrawingOASISServer )
        return ::rtl::OUString( "draw8" );

    if ( *guid == OID_PresentationServer )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "StarOffice XML (Impress)" ) );

    if ( *guid == OID_PresentationOASISServer )
        return ::rtl::OUString( "impress8" );

    if ( *guid == OID_MathServer )
        return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "StarOffice XML (Math)" ) );

    if ( *guid == OID_MathOASISServer )
        return ::rtl::OUString( "math8" );

    return ::rtl::OUString();
}

::rtl::OUString getTestFileURLFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.sxw" );

    if ( *guid == OID_WriterOASISTextServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.odt" );

    if ( *guid == OID_CalcServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.sxc" );

    if ( *guid == OID_CalcOASISServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.ods" );

    if ( *guid == OID_DrawingServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.sxd" );

    if ( *guid == OID_DrawingOASISServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.odg" );

    if ( *guid == OID_PresentationServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.sxi" );

    if ( *guid == OID_PresentationOASISServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.odp" );

    if ( *guid == OID_MathServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.sxm" );

    if ( *guid == OID_MathOASISServer )
        return ::rtl::OUString( "file:///d:/OLE_TEST/test.odf" );

    return ::rtl::OUString();
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
