/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_TUBESDLLAPI_H
#define INCLUDED_TUBESDLLAPI_H

#include "sal/types.h"

#if defined(TUBES_DLLIMPLEMENTATION)
#define TUBES_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define TUBES_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define TUBES_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_TUBESDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
