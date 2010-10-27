/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_SCDLLAPI_H
#define INCLUDED_SCDLLAPI_H

#include "sal/types.h"

#if defined(SC_DLLIMPLEMENTATION)
#define SC_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SC_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define SC_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_SCDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
