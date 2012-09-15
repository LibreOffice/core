/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_SALDLLAPI_H
#define INCLUDED_SALDLLAPI_H

#include "sal/types.h"

#if defined(SAL_DLLIMPLEMENTATION)
#define SAL_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SAL_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#endif /* INCLUDED_SALDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
