/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_REGDLLAPI_H
#define INCLUDED_REGDLLAPI_H

#include "sal/types.h"

#if defined(REG_DLLIMPLEMENTATION)
#define REG_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define REG_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#endif /* INCLUDED_REGDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
