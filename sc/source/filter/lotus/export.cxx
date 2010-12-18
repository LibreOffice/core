/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sc.hxx"




#include "scerrors.hxx"
#include "exp_op.hxx"

#if ENABLE_LOTUS123_EXPORT
FltError ScFormatFilterPluginImpl::ScExportLotus123( SvStream& aStream, ScDocument* pDoc, ExportFormatLotus eFormat, CharSet eDest )
{
    switch( eFormat )
    {
        case ExpWK1:
        {
            ExportWK1 aWKFile( aStream, pDoc, eDest );
            aWKFile.Write();
        }
            break;
        case ExpWK3:
            return eERR_NI;
        default:
            return eERR_NI;
    }

    return eERR_OK;
}
#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
