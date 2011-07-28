/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_SALHELPERDLLAPI_H
#define INCLUDED_SALHELPERDLLAPI_H

#include "sal/types.h"

#if defined(SALHELPER_DLLIMPLEMENTATION)
#define SALHELPER_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define SALHELPER_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define SALHELPER_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_SALHELPERDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
