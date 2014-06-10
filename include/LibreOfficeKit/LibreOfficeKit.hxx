/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_HXX
#define INCLUDED_DESKTOP_INC_LIBREOFFICEKIT_HXX

#include "LibreOfficeKit.h"

/*
 * The reasons this C++ code is not as pretty as it could be are:
 *  a) provide a pure C API - that's useful for some people
 *  b) allow ABI stability - C++ vtables are not good for that.
 *  c) avoid C++ types as part of the API.
 */
namespace lok
{

class Document
{
private:
    LibreOfficeKitDocument* mpDoc;

public:
    inline Document(LibreOfficeKitDocument* pDoc) :
        mpDoc(pDoc)
    {}

    inline ~Document()
    {
        mpDoc->destroy(mpDoc);
    }

    // Save as the given format, if format is NULL sniff from ext'n
    inline bool saveAs(const char* pUrl, const char* pFormat = NULL)
    {
        return mpDoc->saveAs(mpDoc, pUrl, pFormat);
    }

    inline bool saveAsWithOptions(const char* pUrl, const char* pFormat = NULL, const char* pFilterOptions = NULL)
    {
        // available since LibreOffice 4.3
        if (!mpDoc->saveAsWithOptions)
            return false;

        return mpDoc->saveAsWithOptions(mpDoc, pUrl, pFormat, pFilterOptions);
    }
};

class Office
{
private:
    LibreOfficeKit* mpThis;

public:
    inline Office(LibreOfficeKit* pThis) :
        mpThis(pThis)
    {}

    inline ~Office()
    {
        mpThis->destroy(mpThis);
    }

    inline bool initialize(const char* pInstallPath)
    {
        return mpThis->initialize(mpThis, pInstallPath);
    }

    inline Document* documentLoad(const char* pUrl)
    {
        LibreOfficeKitDocument* pDoc = mpThis->documentLoad(mpThis, pUrl);
        if (pDoc == NULL)
            return NULL;
        return new Document(pDoc);
    }

    // return the last error as a string, free me.
    inline char* getError()
    {
        return mpThis->getError(mpThis);
    }
};

}

inline ::lok::Office* lo_cpp_init(const char* pInstallPath)
{
    LibreOfficeKit* pThis = lok_init(pInstallPath);
    if (pThis == NULL || pThis->nSize == 0)
        return NULL;
    return new ::lok::Office(pThis);
}

#endif
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
