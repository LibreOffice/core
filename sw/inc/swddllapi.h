/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_SWDDLLAPI_H
#define INCLUDED_SWDDLLAPI_H

#include "sal/types.h"

#if defined(SWD_DLLIMPLEMENTATION)
#define SWD_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SWD_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define SWD_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_SWDDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
