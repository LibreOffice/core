/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <LibreOfficeKit/LibreOfficeKit.h>

void lok_office_destroy( LibreOfficeKit *pOffice )
{
    pOffice->pClass->destroy( pOffice );
}

int lok_office_initialize( LibreOfficeKit *pOffice, const char *pInstallPath )
{
    return pOffice->pClass->initialize( pOffice, pInstallPath );
}

LibreOfficeKitDocument* lok_office_document_load( LibreOfficeKit *pOffice,
                                                  const char *pURL )
{
    return pOffice->pClass->documentLoad( pOffice, pURL );
}

char* lok_office_get_error( LibreOfficeKit *pOffice )
{
    return pOffice->pClass->getError( pOffice );
}

void lok_document_destroy( LibreOfficeKitDocument* pDocument )
{
    pDocument->pClass->destroy(pDocument);
}

int lok_document_save_as( LibreOfficeKitDocument* pDocument, const char *pURL,
                          const char *pFormat)
{
    return pDocument->pClass->saveAs( pDocument, pURL, pFormat );
}

int lok_document_save_as_with_options(LibreOfficeKitDocument* pDocument,
                                      const char *pURL,
                                      const char *pFormat,
                                      const char *pFilterOptions)
{
    return pDocument->pClass->saveAsWithOptions( pDocument, pURL, pFormat, pFilterOptions );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
