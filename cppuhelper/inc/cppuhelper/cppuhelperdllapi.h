/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_CPPUHELPERDLLAPI_H
#define INCLUDED_CPPUHELPERDLLAPI_H

#include "sal/types.h"

#if defined(CPPUHELPER_DLLIMPLEMENTATION)
#define CPPUHELPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define CPPUHELPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define CPPUHELPER_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_CPPUHELPERDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
