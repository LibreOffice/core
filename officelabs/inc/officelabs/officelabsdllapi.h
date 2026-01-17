/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * OfficeLabs DLL API definitions
 */

#ifndef INCLUDED_OFFICELABS_DLLAPI_H
#define INCLUDED_OFFICELABS_DLLAPI_H

#include <sal/config.h>
#include <sal/types.h>

#if defined(OFFICELABS_DLLIMPLEMENTATION)
#define OFFICELABS_DLLPUBLIC SAL_DLLPUBLIC_EXPORT
#else
#define OFFICELABS_DLLPUBLIC SAL_DLLPUBLIC_IMPORT
#endif

#endif // INCLUDED_OFFICELABS_DLLAPI_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
