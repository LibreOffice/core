/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_STOREDLLAPI_H
#define INCLUDED_STOREDLLAPI_H

#include "sal/types.h"

#if defined(STORE_DLLIMPLEMENTATION)
#define STORE_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define STORE_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#endif /* INCLUDED_STOREDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
