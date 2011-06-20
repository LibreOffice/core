/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_SWDLLAPI_H
#define INCLUDED_SWDLLAPI_H

#include "sal/types.h"

#if defined(SW_DLLIMPLEMENTATION)
#define SW_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SW_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define SW_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_SWDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
