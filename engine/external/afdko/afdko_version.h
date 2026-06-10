/*
 * AFDKO Version Header
 *
 * Static replacement for the cmake-generated afdko_version.h. Keep the
 * version in afdko_version.cpp in sync with AFDKO_TARBALL in download.lst.
 */

#ifndef AFDKO_VERSION_H
#define AFDKO_VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

/* Mutable version pointer - must be initialized by init_fdk_version() */
extern char *FDK_VERSION;

/* Initialize FDK_VERSION with the build-time version */
void init_fdk_version(void);

#ifdef __cplusplus
}
#endif

#endif /* AFDKO_VERSION_H */
