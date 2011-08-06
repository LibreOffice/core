/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#ifndef INCLUDED_I18NUTILDLLAPI_H
#define INCLUDED_I18NUTILDLLAPI_H

#include "sal/types.h"

#if defined(I18NUTIL_DLLIMPLEMENTATION)
#define I18NUTIL_DLLPUBLIC  SAL_DLLPUBLIC_EXPORT
#else
#define I18NUTIL_DLLPUBLIC  SAL_DLLPUBLIC_IMPORT
#endif
#define I18NUTIL_DLLPRIVATE SAL_DLLPRIVATE

#endif /* INCLUDED_I18NUTILDLLAPI_H */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
