/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_AVMEDIADLLAPI_H
#define INCLUDED_AVMEDIADLLAPI_H

#include "sal/types.h"

#if defined(AVMEDIA_DLLIMPLEMENTATION)
#define AVMEDIA_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define AVMEDIA_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define AVMEDIA_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_AVMEDIADLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
