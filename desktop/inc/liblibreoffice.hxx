/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBLIBREOFFICE_HXX
#define INCLUDED_DESKTOP_INC_LIBLIBREOFFICE_HXX

#include <liblibreoffice.h>

/*
 * The reasons this C++ code is not as pretty as it could be are:
 *  a) provide a pure C API - that's useful for some people
 *  b) allow ABI stability - C++ vtables are not good for that.
 *  c) avoid C++ types as part of the API.
 */

class LODocument
{
private:
    LibreOfficeDocument* mpDoc;

public:
    inline LODocument(LibreOfficeDocument* pDoc) :
        mpDoc(pDoc)
    {}

    inline ~LODocument()
    {
        mpDoc->destroy(mpDoc);
    }

    // Save as the given format, if format is NULL sniff from ext'n
    inline bool saveAs(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        return mpDoc->saveAs(mpDoc, pUrl, pFormat, pFilterOptions);
    }
};

class LibLibreOffice
{
private:
    LibreOffice* mpThis;

public:
    inline LibLibreOffice(LibreOffice* pThis) :
        mpThis(pThis)
    {}

    inline ~LibLibreOffice()
    {
        mpThis->destroy(mpThis);
    }

    inline bool initialize(const char* pInstallPath)
    {
        return mpThis->initialize(mpThis, pInstallPath);
    }

    inline LODocument* documentLoad(const char* pUrl)
    {
        LibreOfficeDocument* pDoc = mpThis->documentLoad(mpThis, pUrl);
        if (pDoc == NULL)
            return NULL;
        return new LODocument(pDoc);
    }

    // return the last error as a string, free me.
    inline char* getError()
    {
        return mpThis->getError(mpThis);
    }
};

inline LibLibreOffice* lo_cpp_init(const char* pInstallPath)
{
    LibreOffice* pThis = lo_init(pInstallPath);
    if (pThis == NULL || pThis->nSize == 0)
        return NULL;
    return new LibLibreOffice(pThis);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
