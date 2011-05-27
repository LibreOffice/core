/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_FWEDLLAPI_H
#define INCLUDED_FWEDLLAPI_H

#include "sal/types.h"

#if defined(FWE_DLLIMPLEMENTATION)
#define FWE_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define FWE_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define FWE_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_FWEDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
