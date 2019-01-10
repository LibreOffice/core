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

#include <common.h>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/container/XNameAccess.hpp>

#include "guid.hxx"

wchar_t const * getStorageTypeFromGUID_Impl( GUID const * guid )
{
    if ( *guid == OID_WriterTextServer )
        return L"soffice.StarWriterDocument.6";

    if ( *guid == OID_WriterOASISTextServer )
        return L"LibreOffice.WriterDocument.1";

    if ( *guid == OID_CalcServer )
        return L"soffice.StarCalcDocument.6";

    if ( *guid == OID_CalcOASISServer )
        return L"LibreOffice.CalcDocument.1";

    if ( *guid == OID_DrawingServer )
        return L"soffice.StarDrawDocument.6";

    if ( *guid == OID_DrawingOASISServer )
        return L"LibreOffice.DrawDocument.1";

    if ( *guid == OID_PresentationServer )
        return L"soffice.StarImpressDocument.6";

    if ( *guid == OID_PresentationOASISServer )
        return L"LibreOffice.ImpressDocument.1";

    if ( *guid == OID_MathServer )
        return L"soffice.StarMathDocument.6";

    if ( *guid == OID_MathOASISServer )
        return L"LibreOffice.MathDocument.1";

    return L"";
}

std::u16string_view getServiceNameFromGUID_Impl( GUID const * guid )
{
    if ( *guid == OID_WriterTextServer )
        return u"com.sun.star.comp.Writer.TextDocument";

    if ( *guid == OID_WriterOASISTextServer )
        return u"com.sun.star.comp.Writer.TextDocument";

    if ( *guid == OID_CalcServer )
        return u"com.sun.star.comp.Calc.SpreadsheetDocument";

    if ( *guid == OID_CalcOASISServer )
        return u"com.sun.star.comp.Calc.SpreadsheetDocument";

    if ( *guid == OID_DrawingServer )
        return u"com.sun.star.comp.Draw.DrawingDocument";

    if ( *guid == OID_DrawingOASISServer )
        return u"com.sun.star.comp.Draw.DrawingDocument";

    if ( *guid == OID_PresentationServer )
        return u"com.sun.star.comp.Draw.PresentationDocument";

    if ( *guid == OID_PresentationOASISServer )
        return u"com.sun.star.comp.Draw.PresentationDocument";

    if ( *guid == OID_MathServer )
        return u"com.sun.star.comp.Math.FormulaDocument";

    if ( *guid == OID_MathOASISServer )
        return u"com.sun.star.comp.Math.FormulaDocument";

    return u"";
}

OUString getFilterNameFromGUID_Impl( GUID const * guid )
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
