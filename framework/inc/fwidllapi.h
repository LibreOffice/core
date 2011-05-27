/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_FWIDLLAPI_H
#define INCLUDED_FWIDLLAPI_H

#include "sal/types.h"

#if defined(FWI_DLLIMPLEMENTATION)
#define FWI_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define FWI_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define FWI_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_FWIDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
