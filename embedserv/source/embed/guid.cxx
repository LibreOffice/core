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

OUString getStorageTypeFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return OUString( "soffice.StarWriterDocument.6" );

    if ( *guid == OID_WriterOASISTextServer )
        return OUString( "LibreOffice.WriterDocument.1" );

    if ( *guid == OID_CalcServer )
        return OUString( "soffice.StarCalcDocument.6" );

    if ( *guid == OID_CalcOASISServer )
        return OUString( "LibreOffice.CalcDocument.1" );

    if ( *guid == OID_DrawingServer )
        return OUString( "soffice.StarDrawDocument.6" );

    if ( *guid == OID_DrawingOASISServer )
        return OUString( "LibreOffice.DrawDocument.1" );

    if ( *guid == OID_PresentationServer )
        return OUString( "soffice.StarImpressDocument.6" );

    if ( *guid == OID_PresentationOASISServer )
        return OUString( "LibreOffice.ImpressDocument.1" );

    if ( *guid == OID_MathServer )
        return OUString( "soffice.StarMathDocument.6" );

    if ( *guid == OID_MathOASISServer )
        return OUString( "LibreOffice.MathDocument.1" );

    return OUString();
}

OUString getServiceNameFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return OUString( "com.sun.star.comp.Writer.TextDocument" );

    if ( *guid == OID_WriterOASISTextServer )
        return OUString( "com.sun.star.comp.Writer.TextDocument" );

    if ( *guid == OID_CalcServer )
        return OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );

    if ( *guid == OID_CalcOASISServer )
        return OUString( "com.sun.star.comp.Calc.SpreadsheetDocument" );

    if ( *guid == OID_DrawingServer )
        return OUString( "com.sun.star.comp.Draw.DrawingDocument" );

    if ( *guid == OID_DrawingOASISServer )
        return OUString( "com.sun.star.comp.Draw.DrawingDocument" );

    if ( *guid == OID_PresentationServer )
        return OUString( "com.sun.star.comp.Draw.PresentationDocument" );

    if ( *guid == OID_PresentationOASISServer )
        return OUString( "com.sun.star.comp.Draw.PresentationDocument" );

    if ( *guid == OID_MathServer )
        return OUString( "com.sun.star.comp.Math.FormulaDocument" );

    if ( *guid == OID_MathOASISServer )
        return OUString( "com.sun.star.comp.Math.FormulaDocument" );

    return OUString();
}

OUString getFilterNameFromGUID_Impl( GUID* guid )
{
    if ( *guid == OID_WriterTextServer )
        return OUString( "StarOffice XML (Writer)" );

    if ( *guid == OID_WriterOASISTextServer )
        return OUString( "writer8" );

    if ( *guid == OID_CalcServer )
        return OUString( "StarOffice XML (Calc)" );

    if ( *guid == OID_CalcOASISServer )
        return OUString( "calc8" );

    if ( *guid == OID_DrawingServer )
        return OUString( "StarOffice XML (Draw)" );

    if ( *guid == OID_DrawingOASISServer )
        return OUString( "draw8" );

    if ( *guid == OID_PresentationServer )
        return OUString( "StarOffice XML (Impress)" );

    if ( *guid == OID_PresentationOASISServer )
        return OUString( "impress8" );

    if ( *guid == OID_MathServer )
        return OUString( "StarOffice XML (Math)" );

    if ( *guid == OID_MathOASISServer )
        return OUString( "math8" );

    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
