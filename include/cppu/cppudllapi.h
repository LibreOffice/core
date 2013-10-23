/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_CPPU_CPPUDLLAPI_H
#define INCLUDED_CPPU_CPPUDLLAPI_H

#include "sal/types.h"

#if defined(CPPU_DLLIMPLEMENTATION)
#define CPPU_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define CPPU_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#if defined(PURPENV_DLLIMPLEMENTATION)
#define PURPENV_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define PURPENV_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif

#endif // INCLUDED_CPPU_CPPUDLLAPI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
