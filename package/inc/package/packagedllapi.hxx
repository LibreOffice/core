/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_PACKAGEDLLAPI_H
#define INCLUDED_PACKAGEDLLAPI_H

#include "sal/types.h"

#if defined(DLLIMPLEMENTATION_PACKAGE)
#define DLLPUBLIC_PACKAGE  SAL_DLLPUBLIC_EXPORT
#else
#define DLLPUBLIC_PACKAGE  SAL_DLLPUBLIC_IMPORT
#endif

#endif /* INCLUDED_PACKAGEDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
